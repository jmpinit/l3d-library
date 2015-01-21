#ifdef _TEST
#include "l3d-cube.h"
#include "stub.h"
#else
#include "l3d-cube/l3d-cube.h"
#endif

using namespace L3D;

Color black = Color(0, 0, 0);
Color white = Color(255, 255, 255);
Color red = Color(255, 0, 0);
Color green = Color(0, 255, 0);

Cube cube = Cube(8, 50);
int coords[] { 0, 7 };

void setup()
{
  cube.background(black);
}

void loop()
{
  cube.background(black);

  for(int z1 : coords)
    for(int y1 : coords)
      for(int x1 : coords)
        for(int z2 : coords)
          for(int y2 : coords)
            for(int x2 : coords)
              cube.line(x1, y1, z1, x2, y2, z2, green);

  cube.sphere(0, 0, 0, 3, red);

  cube.setVoxel(4, 3, 2, white);
  cube.show();
  delay(1000);
  cube.setVoxel(4, 3, 2, black);
  cube.show();
  delay(1000);
}
