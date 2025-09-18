#include "our_gl.h"

static Matrix ModelView;
static Matrix Viewport;
static Matrix Projection;

void viewport(int x, int y, int w, int h, int depth) {
  Matrix m = Matrix::identity(4);
  m[0][3] = x + w / 2.0f;
  m[1][3] = y + h / 2.0f;
  m[2][3] = depth / 2.0f;

  m[0][0] = w/2.0f;
  m[1][1] = h/2.0f;
  m[2][2] = depth/2.0f;

  Viewport = m;
}

void projection(float coeff) {
  Matrix p = Matrix::identity(4);
  p[3][2] = coeff;
  Projection = p;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up) {
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
  ModelView = Minv * Tr;
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
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

void triangle(
  Vec4f *pts,
  IShader &ishader,
  TGAImage &image,
  TGAImage &zbuffer) {

  Vec2f bboxmin(std::numetic_limits<float>::max(), std::numeric_limit<float>::max());
  Vec2f bboxmax(-std::numetic_limits<float>::max(), -std::numeric_limit<float>::max());

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin[j] = std::min(bboxmin.raw[j], pts[i].raw[j] / pts[i].w);
      bboxmax[j] = std::max(bboxmax.raw[j], pts[i].raw[j] / pts[i].w);
    }
  }

  Vec2i P;
  TGAColor color;
  for (P.x = bboxmin.x; P.x < bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y < bboxmax.y; P.y++) {
      Vec3f c = barycentric(
        proj<2>(pts[0] / pts[0].w),
        proj<2>(pts[1] / pts[1].w),
        proj<2>(pts[2] / pts[2].w),
        proj<2>(P)
      );
      float z = pts[0].z * c.x + pts[1].z * c.y + pts[2].z * c.z;
      float w = pts[0].w * c.x + pts[1].w * c.y + pts[2].w * c.z;
      int frag_depth = std::max(0, std::min(255, int(z / w + 0.5)));
      if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y).r > frag_depth) {
        continue;
      }
      bool discard = shader.fragment(c, color);
      if (!dicard) {
        zbuffer.set(P.x, P.y, TGAColor(frag_depth));
        image.set(P.x, P.y, color);
      }
    }
  }
}


// ------------------------------------

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
