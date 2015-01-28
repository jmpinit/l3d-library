#include "l3d-cube/l3d-cube.h"
#include <math.h>

using namespace L3D;

Cube cube = Cube();

void setup()
{
  cube.begin();
}

void loop()
{
  cube.listen();
}
