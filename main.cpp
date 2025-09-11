#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "tgaimage.h"
#include "model.h"
#include "our_gl.h"

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
Vec3f light_dir = Vec3f(1, -1, 1).normalize();
Vec3f eye(0, 0, 1);
Vec3f center(0, 0, 0);

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
    Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix Projection = projection(-1.0f / (eye - center).norm());
    Matrix ViewPort = viewport(width/8, width/8, width*3/4, height*3/4, depth);

    TGAImage image(width, height, TGAImage::RGB);
    // Matrix r = rotation_y(cos(90 * M_PI / 180.0), sin(90 * M_PI / 180.0));
    for (int i=0; i<model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec3i screen_coords[3];
      Vec3f world_coords[3];
      float intensity[3];
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = m2v(ViewPort * Projection * ModelView * v2m(v));
        world_coords[j] = v;
        intensity[j] = model->norm(i, j) * light_dir;
      }
      Vec2i uv[3];
      for (int k = 0; k < 3; k++) {
        uv[k] = model->uv(i, k);
      }
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], intensity[0], intensity[1], intensity[2], image, zbuffer, model);
    }
    image.flip_vertically();
    image.write_tga_file("output3.tga");
  }

  delete model;
  delete [] zbuffer;
  return 0;
}
