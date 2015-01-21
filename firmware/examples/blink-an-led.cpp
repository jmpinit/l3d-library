#ifdef _TEST
#include "l3d-cube.h"
#include "stub.h"
#else
#include "l3d-cube/l3d-cube.h"
#endif

using namespace L3D;

Color black = Color(0, 0, 0);
Color white = Color(255, 255, 255);

Cube cube = Cube(8, 50);

void setup() {
  cube.background(black);
}

void loop() {
  cube.setVoxel(4, 3, 2, white);
  cube.show();
  delay(1000);
  cube.setVoxel(4, 3, 2, black);
  cube.show();
  delay(1000);
}
