#ifndef _L3D_CUBE
#define _L3D_CUBE

namespace L3DCube
{
  typedef struct
  {
    unsigned char red, green, blue;
    Color(int r, int g, int b) : red(r), green(g), blue(b) {}
  } Color;

  typedef struct
  {
    float x;
    float y;
    float z;
  } Point;

  void setVoxel(int x, int y, int z, Color col);
  void setVoxel(point p, Color col);
  Color getVoxel(int x, int y, int z);
  Color getVoxel(point p);
  void line(int x1, int y1, int z1, int x1, int y1, int z1, Color col);
  void line(point p1, point p2, Color col);
  void sphere(int x, int y, int z, Color col);
  void sphere(point p, Color col);
  void background(Color col);
  Color colorMap(float var, float min, float max);
}

#endif
