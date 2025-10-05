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

Model *model = NULL;
const int width = 800;
const int height = 800;

Vec3f light_dir = Vec3f(1, 1, 1);
Vec3f eye(0, -1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct GouraundShader : public IShader {
  Vec3f varying_intensity;

  virtual Vec4f vertex(int iface, int nthvert) {
    varying_intensity[nthvert] = std::max(0.0f, model->normal(iface, nthvert) * light_dir);
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    return Viewport * Projection * ModelView * gl_Vertex;
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    float intensity = varying_intensity * bar;
    color = TGAColor(
      255 * intensity,
      255 * intensity,
      255 * intensity,
      255
    );
    return false;
  }
};

int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  lookat(eye, center, up);
  viewport(width/8, width/8, width*3/4, height*3/4);
  projection(-1.0f / (eye - center).norm());
  light_dir.normalize();

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  GouraundShader shader;
  for (int i = 0; i < model->nfaces(); i++) {
    Vec4f screen_coords[3];
    for (int j = 0; j < 3; j++) {
      screen_coords[j] = shader.vertex(i, j);
    }
    triangle(screen_coords, shader, image, zbuffer);
  }

  image.flip_vertically();
  zbuffer.flip_vertically();

  image.write_tga_file("output.tga");
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}
