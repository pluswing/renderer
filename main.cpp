#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

const int width = 800;
const int height = 800;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
const TGAColor blue  = TGAColor(0,   0,   255, 255);


Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec2i P) {
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

void line(Vec2i t0, Vec2i t1, TGAImage &image, TGAColor color) {
  bool steep = false;
  int x0 = t0.x;
  int y0 = t0.y;
  int x1 = t1.x;
  int y1 = t1.y;
  if (std::abs(x0-x1) < std::abs(y0-y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int dx = x1-x0;
  int dy = y1-y0;
  int derror2 = std::abs(dy) * 2;
  int error2 = 0;
  int y = y0;
  if (steep) {
    for (int x = x0; x <= x1; x++) {
      image.set(y, x, color);
      error2 += derror2;
      if (error2 > dx) {
        y += (y1 > y0 ? 1 : -1);
        error2 -= dx*2;
      }
    }
  } else {
    for (int x = x0; x <= x1; x++) {
      image.set(x, y, color);
      error2 += derror2;
      if (error2 > dx) {
        y += (y1 > y0 ? 1 : -1);
        error2 -= dx*2;
      }
    }
  }
}

void triangle(Vec3i *pts, float *zbuffer, TGAImage &image, TGAColor color) {
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
      Vec3f bc_screen = barycentric(pts[0], pts[1], tps[2], P);
      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) {
        continue;
      }
      P.z = 0;
      for (int i = 0; i < 3; i++) {
        P.z += pts[i].z * bc_screen.raw[i];
      }
      if (zbuffer[int(P.x + P.y * width)] < P.z) {
        zbuffer[int(P.x + P.y * width)] = P.z;
        image.set(P.x, P.y, color);
      }
    }
  }
}

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

int main(int argc, char** argv) {

  TGAImage render(width, height, TGAImage::RGB);
  int *zbuffer = new int[width * height];

  for (int i = 0; i < width; i++) {
    ybuffer[i] = std::numeric_limits<int>::min();
  }
  rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
  rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
  rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);

  render.flip_vertically();
  render.write_tga_file("render.tga");
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
