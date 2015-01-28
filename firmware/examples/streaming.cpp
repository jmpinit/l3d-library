#include "l3d-cube/l3d-cube.h"
#include <math.h>

Cube cube = Cube();

void setup()
{
  cube.begin();
}

void loop()
{
  cube.listen();
}
