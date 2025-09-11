#include "our_gl.h"


Matrix viewport(int x, int y, int w, int h, int depth) {
  Matrix m = Matrix::identity(4);
  m[0][3] = x + w / 2.0f;
  m[1][3] = y + h / 2.0f;
  m[2][3] = depth / 2.0f;

  m[0][0] = w/2.0f;
  m[1][1] = h/2.0f;
  m[2][2] = depth/2.0f;

  return m;
}

Matrix projection(float coeff) {
  Matrix Projection = Matrix::identity(4);
  Projection[3][2] = coeff;
  return Projection;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye - center).normalize();
  Vec3f x = (up ^ z).normalize();
  Vec3f y = (z ^ x).normalize();
  Matrix Minv = Matrix::identity(4);
  Matrix Tr = Matrix::identity(4);
  for (int i = 0; i < 3; i++) {
    Minv[0][i] = x.raw[i];
    Minv[1][i] = y.raw[i];
    Minv[2][i] = z.raw[i];
    Tr[i][3] = -eye.raw[i];
  }
  return Minv * Tr;
}

void triangle(
  Vec3i t0, Vec3i t1, Vec3i t2,
  Vec2i uv0, Vec2i uv1, Vec2i uv2,
  float ity0, float ity1, float ity2,
  TGAImage &image, int *zbuffer, Model *model) {

  int width = image.get_width();
  int height = image.get_height();

  if (t0.y == t1.y && t0.y == t2.y) return;

  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(uv0, uv1);
    std::swap(ity0, ity1);
  }

  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(uv0, uv2);
    std::swap(ity0, ity2);
  }

  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(uv1, uv2);
    std::swap(ity1, ity2);
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
    float ityA = ity0 + (ity2 - ity0) * alpha;
    float ityB = second_half ? ity1 + (ity2 - ity1) * beta
                            : ity0 + (ity1 - ity0) * beta;

    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(uvA, uvB);
      std::swap(ityA, ityB);
    }
    for (int j = A.x; j <= B.x; j++) {
      float phi = B.x == A.x ? 1.0f : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = Vec3f(A) + Vec3f(B-A) * phi;
      Vec2i uvP = uvA + (uvB - uvA) * phi;
      float ityP = ityA + (ityB - ityA) * phi;
      int idx = P.x + P.y * width;
      if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) {
        continue;
      }
      if (zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        TGAColor color = model->diffuse(uvP);
        image.set(P.x, P.y, TGAColor(
          color.r * ityP,
          color.g * ityP,
          color.b * ityP,
          255
        ));
      }
    }
  }
}
