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

const int width = 800;
const int height = 800;
const int depth = 255;

Model *model = NULL;
int *zbuffer = NULL;
Vec3f light_dir(0, 0, -1);
Vec3f camera(0, 0, 3);

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

void triangle(
  Vec3i t0, Vec3i t1, Vec3i t2,
  Vec2i uv0, Vec2i uv1, Vec2i uv2,
  TGAImage &image, float intensity, int *zbuffer) {

  if (t0.y == t1.y && t0.y == t2.y) return;

  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(uv0, uv1);
  }

  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(uv0, uv2);
  }

  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(uv1, uv2);
  }

  int total_height = t2.y - t0.y;
  for (int i = 0; i < total_height; i++) {
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
    float alpha = (float) i / total_height;
    float beta = (float) (i - (second_half ? t1.y - t0.y : 0)) / segment_height;
    Vec3i A = t0 + Vec3f(t2-t0) * alpha;
    Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta
                          : t0 + Vec3f(t1 - t0) * beta;
    Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
    Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta
                            : uv0 + (uv1 - uv0) * beta;

    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(uvA, uvB);
    }
    for (int j = A.x; j <= B.x; j++) {
      float phi = B.x == A.x ? 1.0f : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = Vec3f(A) + Vec3f(B-A) * phi;
      Vec2i uvP = uvA + (uvB - uvA) * phi;
      int idx = P.x + P.y * width;
      if (zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        TGAColor color = model->diffuse(uvP);
        image.set(P.x, P.y, TGAColor(
          color.r * intensity,
          color.g * intensity,
          color.b * intensity,
          255
        ));
      }
    }
  }
}

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
    model = new Model("obj/african_head.obj");
  }

  zbuffer = new int[width * height];
  for (int i = 0; i < width*height; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }

  {
    Matrix Projection = Matrix::identity(4);
    Matrix ViewPort = viewport(width/8, width/8, width*3/4, height*3/4);
    Projection[3][2] = -1.0f / camera.z;

    TGAImage image(width, height, TGAImage::RGB);
    // Matrix r = rotation_y(cos(90 * M_PI / 180.0), sin(90 * M_PI / 180.0));
    for (int i=0; i<model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec3i screen_coords[3];
      Vec3f world_coords[3];
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
        world_coords[j] = v;
      }
      Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
      n.normalize();
      float intensity = n * light_dir;
      if (intensity > 0) {
        Vec2i uv[3];
        for (int k = 0; k < 3; k++) {
          uv[k] = model->uv(i, k);
        }
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], image, intensity, zbuffer);
      }
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
  }

  delete model;
  delete [] zbuffer;
  return 0;
}

/*
  TGAImage texture(1, 1, TGAImage::RGB);
  texture.read_tga_file("obj/african_head_diffuse.tga");

  TGAImage render(width, height, TGAImage::RGB);


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

/*

  TGAImage image(width, height, TGAImage::RGB);
  Matrix VP = viewport(width/4, width/4, width/2, height/2);
  Matrix Projection = Matrix::identity(4);
  Projection[3][2] = -1.0f / camera.z;

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

  // Matrix r = rotation_y(cos(10 * M_PI / 180.0), sin(10 * M_PI / 180.0));

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    for (int j = 0; j < (int) face.size(); j++) {
      Vec3f wp0 = model->vert(face[j]);
      Vec3f wp1 = model->vert(face[(j+1)%face.size()]);
      {
        Vec3f sp0 = m2v(VP * Projection * v2m(wp0));
        Vec3f sp1 = m2v(VP * Projection * v2m(wp1));
        line(Vec3i(sp0.x, sp0.y, sp0.z), Vec3i(sp1.x, sp1.y, sp1.z), image, white);
      }
      // {
      //   Matrix T = zoom(1.5);
      //   //
      //   Vec3f sp0 = m2v(VP * T * Projection * v2m(wp0));
      //   Vec3f sp1 = m2v(VP * T * Projection * v2m(wp1));
      //   line(Vec3i(sp0.x, sp0.y, sp0.z), Vec3i(sp1.x, sp1.y, sp1.z), image, yellow);
      // }
    }
    // break;
  }

  image.flip_vertically();
  image.write_tga_file("image.tga");
  delete model;
*/
