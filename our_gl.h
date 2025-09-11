#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "geometry.h"
#include "tgaimage.h"
#include "model.h"

Matrix viewport(int x, int y, int w, int h, int depth);
Matrix projection(float coeff=0.0f); // coeff = -1/c
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

void triangle(
  Vec3i t0, Vec3i t1, Vec3i t2,
  Vec2i uv0, Vec2i uv1, Vec2i uv2,
  float ity0, float ity1, float ity2,
  TGAImage &image, int *zbuffer, Model *model);

#endif
