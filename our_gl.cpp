#include "our_gl.h"

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

void viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::identity();
  float depth = 255.0f;

  m[0][3] = x + w / 2.0f;
  m[1][3] = y + h / 2.0f;
  m[2][3] = depth / 2.0f;

  m[0][0] = w / 2.0f;
  m[1][1] = h / 2.0f;
  m[2][2] = depth / 2.0f;

  Viewport = m;
}

void projection(float coeff) {
  Matrix p = Matrix::identity();
  p[3][2] = coeff;
  Projection = p;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye - center).normalize();
  Vec3f x = cross(up, z).normalize();
  Vec3f y = cross(z, x).normalize();
  ModelView = Matrix::identity();

  for (int i = 0; i < 3; i++) {
    ModelView[0][i] = x[i];
    ModelView[1][i] = y[i];
    ModelView[2][i] = z[i];
    ModelView[i][3] = -center[i];
  }
}

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
  Vec3f s[2];
  for (int i = 2; i--;) {
    s[i][0] = C[i] - A[i];
    s[i][1] = B[i] - A[i];
    s[i][2] = A[i] - P[i];
  }
  Vec3f u = cross(s[0], s[1]);
  if (std::abs(u.z) > 1e-2) {
    return Vec3f(
      1.0f - (u.x + u.y) / u.z,
      u.y / u.z,
      u.x / u.z
    );
  }
  return Vec3f(-1, 1, 1);
}

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j]/pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j]/pts[i][3]);
        }
    }
    Vec2i P;
    TGAColor color;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f c = barycentric(proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1]/pts[1][3]), proj<2>(pts[2]/pts[2][3]), proj<2>(P));
            float z = pts[0][2]*c.x + pts[1][2]*c.y + pts[2][2]*c.z;
            float w = pts[0][3]*c.x + pts[1][3]*c.y + pts[2][3]*c.z;
            int frag_depth = z/w;
            if (c.x<0 || c.y<0 || c.z<0 || zbuffer.get(P.x, P.y)[0]>frag_depth) continue;
            bool discard = shader.fragment(c, color);
            if (!discard) {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}


// ------------------------------------

Matrix translation(Vec3f v) {
  Matrix Tr = Matrix::identity();
  Tr[0][3] = v.x;
  Tr[1][3] = v.y;
  Tr[2][3] = v.z;
  return Tr;
}

Matrix zoom(float factor) {
  Matrix Z = Matrix::identity();
  Z[0][0] = Z[1][1] = Z[2][2] = factor;
  return Z;
}

Matrix rotation_x(float cosangle, float sinangle) {
  Matrix R = Matrix::identity();
  R[1][1] = R[2][2] = cosangle;
  R[1][2] = -sinangle;
  R[2][1] = sinangle;
  return R;
}

Matrix rotation_y(float cosangle, float sinangle) {
  Matrix R = Matrix::identity();
  R[0][0] = R[2][2] = cosangle;
  R[0][2] = sinangle;
  R[2][0] = -sinangle;
  return R;
}

Matrix rotation_z(float cosangle, float sinangle) {
  Matrix R = Matrix::identity();
  R[0][0] = R[1][1] = cosangle;
  R[0][1] = -sinangle;
  R[1][0] = sinangle;
  return R;
}
