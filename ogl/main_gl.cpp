#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#define USE_SHADERS 1
GLuint prog_hdlr;

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 1024;
const float camera[] = {0.6, 0, 1};
const float light0_position[4] = { 1, 1, 1, 0 };

void render_scene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  gluLookAt(
    camera[0], camera[1], camera[2],
    0, 0, 0,
    0, 1, 0
  );
  glColor3f(0.8, 0.5, 0.0);
  glutSolidTeapot(0.7);
  glutSwapBuffers();
}

void process_keys(unsigned char key, int x, int y) {
  if (27 == key) {
    exit(0);
  }
}

void change_size(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  glOrtho(-1, 1, -1, 1, -1, 8);
  glMatrixMode(GL_MODELVIEW);
}

#if USE_SHADERS
void printInfoLog(GLuint obj) {
  int log_size = 0;
  int bytes_written = 0;
  glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_size);
  if (!log_size) {
    return;
  }
  char *infoLog = new char[log_size];
  glGetProgramInfoLog(obj, log_size, &bytes_written, infoLog);
  std:cerr << infoLog << std::endl;
  delete [] infoLog;
}

bool read_n_compile_shader(const chat* filename, GLuint &hdlr, GLenum shaderType) {
  std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);
  if (!is.is_open()) {
    std::cerr << "Unable to open file" << filename << std::endl;
    return false;
  }
  log size = is.tellg();
  char *buffer = new char[size + 1];
  is.seekg(0, std::ios::beg);
  is.read(buffer, size);
  is.close()
  buffer[size] = 0;

  // TODO
}
#endif

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  glutCreateWindow("GLSL tutorial");
  glClearColor(0.0, 0.0, 1.0, 1.0);

  glutDisplayFunc(render_scene);
  glutReshapeFunc(change_size);
  glutKeyboardFunc(process_keys);

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

  glutMainLoop();
  return 0;
}
