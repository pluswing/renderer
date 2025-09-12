#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h, int depth);
void projection(float coeff=0.0f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
  virtual ~IShader();
  virtual Vec3i vertex(int iface, int nthvert) = 0;
  virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(
  Vec3i t0, Vec3i t1, Vec3i t2,
  Vec2i uv0, Vec2i uv1, Vec2i uv2,
  float ity0, float ity1, float ity2,
  TGAImage &image, int *zbuffer, Model *model);

#endif
