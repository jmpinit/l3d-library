#include "l3d-cube/l3d-cube.h"
#include <math.h>

using namespace L3D;

Cube cube = Cube();

void setup()
{
  cube.begin();
  cube.background(black);
}

void loop()
{
  static int t = 0;
  static const int stretch = 512;

  Color color = cube.colorMap(t % stretch, 0, stretch);
  Color opposite = cube.colorMap((t + stretch / 2) % stretch, 0, stretch);

  cube.background(color);
  t++;

  float slow_t = (float) t / 32;
  cube.sphere(4 + 3*cos(slow_t), 4 + 3*sin(slow_t), 4 + 3*sin(slow_t), 2, red);
  cube.line(0, 0, 0, 7, 7, 7, opposite);

  if(t / 16 % 2 == 0) {
    cube.setVoxel(7, 0, 7, black);
    cube.setVoxel(0, 7, 0, white);
  } else {
    cube.setVoxel(7, 0, 7, white);
    cube.setVoxel(0, 7, 0, black);
  }

  cube.show();
}
