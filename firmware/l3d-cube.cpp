#ifdef _TEST
#include "stub.h"
#endif

#include <math.h>
#include "l3d-cube.h"

using namespace L3D;

/** Construct a new cube.
  @param s Size of one side of the cube in number of LEDs.
  @param mb Maximum brightness value. Used to prevent the LEDs from drawing too much current (which causes the colors to distort).

  @return A new Cube object.
*/
Cube::Cube(unsigned int s=8, unsigned int mb=50) : \
    size(s), maxBrightness(mb), strip(Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE)) {}

/** Set a voxel at a position to a color.

    @param x, y, z Coordinate of the LED to set.
    @param col Color to set the LED to.
*/
void Cube::setVoxel(int x, int y, int z, Color col)
{
  int index = (z*64) + (x*8) + y;
  strip.setPixelColor(index, strip.Color(col.red, col.green, col.blue));
}

/** Set a voxel at a position to a color.

    @param p Coordinate of the LED to set.
    @param col Color to set the LED to.
*/
void Cube::setVoxel(Point p, Color col)
{
  setVoxel(p.x, p.y, p.z, col);
}

/** Get the color of a voxel at a position.
    
    @param x, y, z Coordinate of the LED to get the color from.
*/
Color Cube::getVoxel(int x, int y, int z)
{
  int index = (z * this->size * this->size) + (x * this->size) + y;
  uint32_t col = strip.getPixelColor(index);
  Color pixelColor = Color((col>>16) & 0xff, (col>>8) & 0xff, col & 0xff);
  return pixelColor;
}

/** Get the color of a voxel at a position.
    
    @param p Coordinate of the LED to get the color from.
*/
Color Cube::getVoxel(Point p)
{
  return getVoxel(p.x, p.y, p.z);
}

/** Draw a line in 3D space.
    Uses the 3D form of Bresenham's algorithm.
    
    @param x1, y1, z1 Coordinate of start of line.
    @param x2, y2, z2 Coordinate of end of line.
    @param col Color of the line.
*/
void Cube::line(int x1, int y1, int z1, int x2, int y2, int z2, Color col)
{
  Point currentPoint = Point(x1, y1, z1);

  int dx = x2 - x1;
  int dy = y2 - y1;
  int dz = z2 - z1;
  int x_inc = (dx < 0) ? -1 : 1;
  int l = abs(dx);
  int y_inc = (dy < 0) ? -1 : 1;
  int m = abs(dy);
  int z_inc = (dz < 0) ? -1 : 1;
  int n = abs(dz);
  int dx2 = l << 1;
  int dy2 = m << 1;
  int dz2 = n << 1;

  if((l >= m) && (l >= n)) {
    int err_1 = dy2 - l;
    int err_2 = dz2 - l;

    for(int i = 0; i < l; i++) {
      this->setVoxel(currentPoint, col);

      if(err_1 > 0) {
        currentPoint.y += y_inc;
        err_1 -= dx2;
      }

      if(err_2 > 0) {
        currentPoint.z += z_inc;
        err_2 -= dx2;
      }

      err_1 += dy2;
      err_2 += dz2;
      currentPoint.x += x_inc;
    }
  } else if((m >= l) && (m >= n)) {
    int err_1 = dx2 - m;
    int err_2 = dz2 - m;

    for(int i = 0; i < m; i++) {
      this->setVoxel(currentPoint, col);
      
      if(err_1 > 0) {
        currentPoint.x += x_inc;
        err_1 -= dy2;
      }

      if(err_2 > 0) {
        currentPoint.z += z_inc;
        err_2 -= dy2;
      }

      err_1 += dx2;
      err_2 += dz2;
      currentPoint.y += y_inc;
    }
  } else {
    int err_1 = dy2 - n;
    int err_2 = dx2 - n;

    for(int i = 0; i < n; i++) {
      this->setVoxel(currentPoint, col);

      if(err_1 > 0) {
        currentPoint.y += y_inc;
        err_1 -= dz2;
      }

      if(err_2 > 0) {
        currentPoint.x += x_inc;
        err_2 -= dz2;
      }

      err_1 += dy2;
      err_2 += dx2;
      currentPoint.z += z_inc;
    }
  }

  this->setVoxel(currentPoint, col);
}

/** Draw a line in 3D space.
    Uses the 3D form of Bresenham's algorithm.
    
    @param p1 Coordinate of start of line.
    @param p2 Coordinate of end of line.
    @param col Color of the line.
*/
void Cube::line(Point p1, Point p2, Color col)
{
  line(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, col);
}

/** Draw a filled sphere.

    @param x, y, z Position of the center of the sphere.
    @param r Radius of the sphere.
    @param col Color of the sphere.
*/
void Cube::sphere(int x, int y, int z, int r, Color col)
{
  for(unsigned int dx = 0; dx < size; dx++) {
    for(unsigned int dy = 0; dy < size; dy++) {
      for(unsigned int dz = 0; dz < size; dz++) {
        if(sqrt(pow(x-dx, 2) + pow(y-dy, 2) + pow(z-dz, 2)) <= r) {
          setVoxel(x + dx, y + dy, z + dz, col);
        }
      }
    }
  }
}

/** Draw a filled sphere.

    @param p Position of the center of the sphere.
    @param r Radius of the sphere.
    @param col Color of the sphere.
*/
void Cube::sphere(Point p, int r, Color col)
{
  sphere(p.x, p.y, p.z, r, col);
}

/** Set the entire cube to one color.
    
    @param col The color to set all LEDs in the cube to.
*/
void Cube::background(Color col)
{
  for(unsigned int x = 0; x < this->size; x++)
    for(unsigned int y = 0; y < this->size; y++)
      for(unsigned int z = 0; z < this->size; z++)
        setVoxel(x, y, z, col);
}

/** Map a value into a color.
    The set of colors fades from blue to green to red and back again.

    @param val Value to map into a color.
    @param min Minimum value that val will take.
    @param max Maximum value that val will take.

    @return Color from value.
*/
Color Cube::colorMap(float val, float min, float max)
{
  const float range = 1024;
  val = range * (val-min) / (max-min);

  Color colors[6];

  colors[0].red = 0;
  colors[0].green = 0;
  colors[0].blue = this->maxBrightness;

  colors[1].red = 0;
  colors[1].green = this->maxBrightness;
  colors[1].blue = this->maxBrightness;

  colors[2].red = 0;
  colors[2].green = this->maxBrightness;
  colors[2].blue = 0;

  colors[3].red = this->maxBrightness;
  colors[3].green = this->maxBrightness;
  colors[3].blue = 0;

  colors[4].red = this->maxBrightness;
  colors[4].green = 0;
  colors[4].blue = 0;

  colors[5].red = this->maxBrightness;
  colors[5].green = 0;
  colors[5].blue = this->maxBrightness;

  if(val <= range/6)
    return lerpColor(colors[0], colors[1], val, 0, range/6);
  else if(val <= 2 * range / 6)
    return(lerpColor(colors[1], colors[2], val, range / 6, 2 * range / 6));
  else if(val <= 3 * range / 6)
    return(lerpColor(colors[2], colors[3], val, 2 * range / 6, 3*range / 6));
  else if(val <= 4 * range / 6)
    return(lerpColor(colors[3], colors[4], val, 3 * range / 6, 4*range / 6));
  else if(val <= 5 * range / 6)
    return(lerpColor(colors[4], colors[5], val, 4 * range / 6, 5*range / 6));
  else
    return(lerpColor(colors[5], colors[0], val, 5 * range / 6, range));
}

/** Linear interpolation between colors.

    @param a, b The colors to interpolate between.
    @param val Position on the line between color a and color b.
        When equal to min the output is color a, and when equal to max the output is color b.
    @param min Minimum value that val will take.
    @param max Maximum value that val will take.

    @return Color between colors a and b.
*/
Color Cube::lerpColor(Color a, Color b, int val, int min, int max)
{
  int red = a.red + (b.red-a.red) * (val-min) / (max-min);
  int green = a.green + (b.green-a.green) * (val-min) / (max-min);
  int blue = a.blue + (b.blue-a.blue) * (val-min) / (max-min);

  return Color(red, green, blue);
}

/** Make changes to the cube visible.
    Causes pixel data to be written to the LED strips.
*/
void Cube::show()
{
  strip.show();
}
