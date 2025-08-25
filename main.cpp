#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

const TGAColor white  = TGAColor(255, 255, 255, 255);
const TGAColor red    = TGAColor(255, 0,   0,   255);
const TGAColor green  = TGAColor(0,   255, 0,   255);
const TGAColor blue   = TGAColor(0,   0,   255, 255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

Model *model = NULL;
const int width = 100;
const int height = 100;
const int depth = 255;
/*
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
  Vec3f s[2];
  for (int i = 2; i--;) {
    s[i].raw[0] = C.raw[i] - A.raw[i];
    s[i].raw[1] = B.raw[i] - A.raw[i];
    s[i].raw[2] = A.raw[i] - P.raw[i];
  }
  Vec3f u = s[0] ^ s[1];
  if (std::abs(u.z) > 1e-2) {
    return Vec3f(
      1.0f - (u.x + u.y) / u.z,
      u.y / u.z,
      u.x / u.z
    );
  }
  return Vec3f(-1, 1, 1);
}
*/
void line(Vec3i p0, Vec3i p1, TGAImage &image, TGAColor color) {
  bool steep = false;
  if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
    std::swap(p0.x, p0.y);
    std::swap(p1.x, p1.y);
    steep = 1;
  }
  if (p0.x > p1.x) {
    std::swap(p0, p1);
  }

  for (int x=p0.x; x <= p1.x; x++) {
    float t = (x-p0.x) / (float) (p1.x - p0.x);
    int y = p0.y * (1.0 - t) + p1.y * t + 0.5;
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
  }
}
/*
void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, Vec2f *texture_pts, TGAImage &texture) {
  Vec2f bboxmin(
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::max()
  );
  Vec2f bboxmax(
    -std::numeric_limits<float>::max(),
    -std::numeric_limits<float>::max()
  );
  Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin.raw[j] = std::max(0.0f, std::min(bboxmin.raw[j], pts[i].raw[j]));
      bboxmax.raw[j] = std::max(clamp.raw[j], std::min(bboxmax.raw[j], pts[i].raw[j]));
    }
  }
  Vec3f P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) {
        continue;
      }
      P.z = 0;
      for (int i = 0; i < 3; i++) {
        P.z += pts[i].z * bc_screen.raw[i];
      }
      // FIXME 重心からテクスチャのx,yを求める
      int tx = 0;
      int ty = 0;
      if (zbuffer[int(P.x + P.y * width)] < P.z) {
        zbuffer[int(P.x + P.y * width)] = P.z;
        int tx =
        image.set(P.x, P.y, texture.get(tx, ty));
      }
    }
  }
}
*/
/*
void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[]) {

  if (p0.x > p1.x) {
    std::swap(p0, p1);
  }

  for (int x = p0.x; x <= p1.x; x++) {
    float t = (x - p0.x) / (float)(p1.x - p0.x);
    int y = p0.y * (1.0 - t) + p1.y * t;
    if (ybuffer[x] < y) {
      ybuffer[x] = y;
      image.set(x, 0, color);
    }
  }
}

Vec3f world2screen(Vec3f v) {
  return Vec3f(
    int((v.x + 1.0) * width / 2.0 + 0.5),
    int((v.y + 1.0) * height / 2.0 + 0.5),
    v.z
  );
}

Vec2f world2texture(Vec2f v, TGAImage &texture) {
  return Vec2f(
    int((v.x + 1.0) * texture.get_width() / 2.0 + 0.5),
    int((v.y + 1.0) * texture.get_height() / 2.0 + 0.5)
  );
}
*/

Vec3f m2v(Matrix m) {
  return Vec3f(
    m[0][0] / m[3][0],
    m[1][0] / m[3][0],
    m[2][0] / m[3][0]
  );
}

Matrix v2m(Vec3f v) {
  Matrix m(4, 1);
  m[0][0] = v.x;
  m[1][0] = v.y;
  m[2][0] = v.z;
  m[3][0] = 1.0f;
  return m;
}

Matrix viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::identity(4);
  m[0][3] = x + w / 2.0f;
  m[1][3] = y + h / 2.0f;
  m[2][3] = depth / 2.0f;

  m[0][0] = w/2.0f;
  m[1][1] = h/2.0f;
  m[2][2] = depth/2.0f;

  return m;
}

Matrix translation(Vec3f v) {
  Matrix Tr = Matrix::identity(4);
  Tr[0][3] = v.x;
  Tr[1][3] = v.y;
  Tr[2][3] = v.z;
  return Tr;
}

Matrix zoom(float factor) {
  Matrix Z = Matrix::identity(4);
  Z[0][0] = Z[1][1] = Z[2][2] = factor;
  return Z;
}

Matrix rotation_x(float cosangle, float sinangle) {
  Matrix R = Matrix::identity(4);
  R[1][1] = R[2][2] = cosangle;
  R[1][2] = -sinangle;
  R[2][1] = sinangle;
  return R;
}

Matrix rotation_y(float cosangle, float sinangle) {
  Matrix R = Matrix::identity(4);
  R[0][0] = R[2][2] = cosangle;
  R[0][2] = sinangle;
  R[2][0] = -sinangle;
  return R;
}

Matrix rotation_z(float cosangle, float sinangle) {
  Matrix R = Matrix::identity(4);
  R[0][0] = R[1][1] = cosangle;
  R[0][1] = -sinangle;
  R[1][0] = sinangle;
  return R;
}

int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/cube.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);
  Matrix VP = viewport(width/4, width/4, width/2, height/2);

  {
    Vec3f x(1.0f, 0.0f, 0.0f);
    Vec3f y(0.0f, 1.0f, 0.0f);
    Vec3f o(0.0f, 0.0f, 0.0f);

    o = m2v(VP*v2m(o));
    x = m2v(VP*v2m(x));
    y = m2v(VP*v2m(y));

    line(Vec3i(o.x, o.y, o.z), Vec3i(x.x, x.y, x.z), image, red);
    line(Vec3i(o.x, o.y, o.z), Vec3i(y.x, y.y, y.z), image, green);
  }

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    for (int j = 0; j < (int) face.size(); j++) {
      Vec3f wp0 = model->vert(face[j]);
      Vec3f wp1 = model->vert(face[(j+1)%face.size()]);
      {
        Vec3f sp0 = m2v(VP * v2m(wp0));
        Vec3f sp1 = m2v(VP * v2m(wp1));
        line(Vec3i(sp0.x, sp0.y, sp0.z), Vec3i(sp1.x, sp1.y, sp1.z), image, white);
      }
      {
        Matrix T = zoom(1.5);
        //
        Vec3f sp0 = m2v(VP * T * v2m(wp0));
        Vec3f sp1 = m2v(VP * T * v2m(wp1));
        line(Vec3i(sp0.x, sp0.y, sp0.z), Vec3i(sp1.x, sp1.y, sp1.z), image, yellow);
      }
    }
    break;
  }

  image.flip_vertically();
  image.write_tga_file("image.tga");
  delete model;
/*
  TGAImage texture(1, 1, TGAImage::RGB);
  texture.read_tga_file("obj/african_head_diffuse.tga");

  TGAImage render(width, height, TGAImage::RGB);
  float *zbuffer = new float[width * height];

  for (int i = 0; i < width*height; i++) {
    zbuffer[i] = -std::numeric_limits<float>::max();
  }

  Vec3f light_dir(0, 0, -1);
  for (int i=0; i<model->nfaces(); i++) {
    std::cout << "draw" << i << "/" << model->nfaces() << "\n";
    std::vector<int> face = model->face(i);
    std::vector<int> texture_face = model->texture_face(i);
    Vec3f pts[3];
    Vec2f texture_pts[3];

    Vec3f world_coods[3];
    for (int j=0; j < 3; j++) {
      pts[j] = world2screen(model->vert(face[j]));
      texture_pts[j] = world2texture(model->texture_vert(texture_face[j]), texture);
      world_coods[j] = model->vert(face[j]);
    }
    Vec3f n = (world_coods[2] - world_coods[0]) ^ (world_coods[1] - world_coods[0]);
    n.normalize();
    float intensity = n * light_dir;
    if (intensity > 0) {
      triangle(pts, zbuffer, render, texture_pts, texture);
    }
  }

  render.flip_vertically();
  render.write_tga_file("render.tga");
  delete model;
*/
/*
  TGAImage scene(width, height, TGAImage::RGB);
  line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
  line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
  line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

  line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

  scene.flip_vertically();
  scene.write_tga_file("scene.tga");
*/

  /*
  Model* model;
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  Vec3f light_dir(0, 0, -1);

  for (int i=0; i<model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec2i screen_coords[3];
    Vec3f world_coords[3];
    for (int j=0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = Vec2i(
        (v.x + 1.0) * width / 2.0,
        (v.y + 1.0) * height / 2.0
      );
      world_coords[j] = v;
    }
    Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;
    if (intensity > 0) {
      triangle(screen_coords, image, TGAColor(
        intensity * 255,
        intensity * 255,
        intensity * 255,
        255
      ));
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");
  delete model;
*/
}
