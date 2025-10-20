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
  mat<2, 3, float> varying_uv;

  virtual Vec4f vertex(int iface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    varying_intensity[nthvert] = std::max(0.0f, model->normal(iface, nthvert) * light_dir);
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    return Viewport * Projection * ModelView * gl_Vertex;
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    float intensity = varying_intensity * bar;
    // if (intensity > 0.85) intensity = 1.0;
    // else if (intensity > 0.60) intensity = 0.8;
    // else if (intensity > 0.45) intensity = 0.60;
    // else if (intensity > 0.30) intensity = 0.45;
    // else if (intensity > 0.15) intensity = 0.3;
    // else intensity = 0;

    Vec2f uv = varying_uv * bar;
    color = model->diffuse(uv) * intensity;

    // color = TGAColor(
    //   255 * intensity,
    //   155 * intensity,
    //   0 * intensity,
    //   255
    // );
    return false;
  }
};

struct ShaderPhong : public IShader {
  mat<2, 3, float> varying_uv;
  mat<4, 4, float> uniform_M;
  mat<4, 4, float> uniform_MIT;

  virtual Vec4f vertex(int iface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    return Viewport * Projection * ModelView * gl_Vertex;
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    Vec2f uv = varying_uv * bar;
    Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
    Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
    Vec3f r = (n * (n * l * 2.0f) - l).normalize();
    float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
    float diff = std::max(0.0f, n * l);
    TGAColor c = model->diffuse(uv);
    color = c;
    for (int i = 0; i < 3; i++) {
      color[i] = std::min<float>(5 + c[i] * (diff + 0.6 * spec), 255);
    }
    return false;
  }
};


struct Shader : public IShader {
  mat<2, 3, float> varying_uv;
  mat<4, 3, float> varying_tri;
  mat<3, 3, float> varying_nrm;
  mat<3, 3, float> ndc_tri;

  virtual Vec4f vertex(int iface, int nthvert) {
    varying_uv.set_col(nthvert, model->uv(iface, nthvert));
    varying_nrm.set_col(nthvert, proj<3>(
      (Projection * ModelView).invert_transpose()
      * embed<4>(model->normal(iface, nthvert), 0.0f)
    ));
    Vec4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
    varying_tri.set_col(nthvert, gl_Vertex);
    ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
    return gl_Vertex;
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    Vec3f bn = (varying_nrm * bar).normalize();
    Vec2f uv = varying_uv * bar;

    mat<3, 3, float> A;
    A[0] = ndc_tri.col(1) - ndc_tri.col(0);
    A[1] = ndc_tri.col(2) - ndc_tri.col(0);
    A[2] = bn;

    mat<3, 3, float> AI = A.invert();
    Vec3f i = AI * Vec3f(
      varying_uv[0][1] - varying_uv[0][0],
      varying_uv[0][2] - varying_uv[0][0],
      0
    );
    Vec3f j = AI * Vec3f(
      varying_uv[1][1] - varying_uv[1][0],
      varying_uv[1][2] - varying_uv[1][0],
      0
    );

    mat<3, 3, float> B;
    B.set_col(0, i.normalize());
    B.set_col(1, j.normalize());
    B.set_col(2, bn);

    Vec3f n = (B * model->normal(uv)).normalize();

    float diff = std::max(0.0f, n * light_dir);
    color = model->diffuse(uv) * diff;
    return false;
  }
};

struct DepthShader : public IShader {
  mat<3, 3, float> varying_tri;

  DepthShader(): varying_tri() {}

  virtual Vec4f vertex(int iface, int nthvert) {
    Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
    gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
    varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
    return gl_Vertex;
  }

  virtual bool fragment(Vec3f bar, TGAColor &color) {
    Vec3f p = varying_tri * bar;
    float depth = 255.0f;
    color = TGAColor(255, 255, 255) * ((p.z) / depth);
    return false;
  }
};

int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage depth(width, height, TGAImage::RGB);
  TGAImage shadowbuffer(width, height, TGAImage::GRAYSCALE);
  lookat(light_dir, center, up);
  viewport(width/8, height/8, width*3/4, height*3/4);
  projection(0);
  DepthShader depthshader;
  Vec4f screen_coords[3];
  for (int i = 0; i < model->nfaces(); i++) {
    for (int j = 0; j < 3; j++) {
      screen_coords[j] = depthshader.vertex(i, j);
    }
    triangle(screen_coords, depthshader, depth, shadowbuffer);
  }
  depth.flip_vertically();
  depth.write_tga_file("depth.tga");

  // -----------

  lookat(eye, center, up);
  viewport(width/8, width/8, width*3/4, height*3/4);
  projection(-1.0f / (eye - center).norm());
  light_dir.normalize();

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  // GouraundShader shader;
  Shader shader;
  // shader.uniform_M = Projection * ModelView;
  // shader.uniform_MIT = (Projection * ModelView).invert_transpose();
  for (int i = 0; i < model->nfaces(); i++) {
    for (int j = 0; j < 3; j++) {
      shader.vertex(i, j);
    }
    triangle(shader.varying_tri, shader, image, zbuffer);
  }

  image.flip_vertically();
  zbuffer.flip_vertically();

  image.write_tga_file("output.tga");
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}
