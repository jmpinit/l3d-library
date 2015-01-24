#include "l3d-cube/l3d-cube.h"
#include <math.h>

using namespace L3D;

Cube cube = Cube(8, 50);

void setup()
{
  cube.begin();
}

void loop()
{
  cube.listen();
}
