#include "l3d-cube/l3d-cube.h"

Color black = Color(0, 0, 0);
Color white = Color(255, 255, 255);

void setup() {
  L3DCube::background(black);
}

void loop() {
  L3DCube::setVoxel(4, 3, 2, white);
  delay(1000);
  L3DCube::setVoxel(4, 3, 2, black);
  delay(1000);
}
