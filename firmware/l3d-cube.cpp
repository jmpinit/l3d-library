#include "l3d-cube.h"

namespace L3DCube
{
  void setVoxel(int x, int y, int z, Color col) {
    int index = (z*64) + (x*8) + y;
    strip.setPixelColor(index,strip.Color(col.red, col.green, col.blue));
  }

  void setVoxel(Point p, Color col) {
    setVoxel(p.x, p.y, p.z, col);
  }

  Color getVoxel(int x, int y, int z) {
    int index = (z*SIDE*SIDE) + (x*SIDE) + y;
    uint32_t col=strip.getPixelColor(index);
    Color pixelColor;
    pixelColor.red=(col>>16)&255;
    pixelColor.green=(col>>8)&255;
    pixelColor.blue=col&255;
    return pixelColor;
  }

  Color getVoxel(Point p) {
    return getVoxel(p.x, p.y, p.z);
  }

  void line(int x1, int y1, int z1, int x2, int y2, int z2, Color col) {
    int i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc, err_1, err_2, dx2, dy2, dz2;
    Point currentPoint={p1.x, p1.y, p1.z};
    dx = p2.x - p1.x;
    dy = p2.y - p1.y;
    dz = p2.z - p1.z;
    x_inc = (dx < 0) ? -1 : 1;
    l = abs(dx);
    y_inc = (dy < 0) ? -1 : 1;
    m = abs(dy);
    z_inc = (dz < 0) ? -1 : 1;
    n = abs(dz);
    dx2 = l << 1;
    dy2 = m << 1;
    dz2 = n << 1;

    if ((l >= m) && (l >= n)) {
      err_1 = dy2 - l;
      err_2 = dz2 - l;
      for (i = 0; i < l; i++) {
        setPixel(currentPoint, col);
        if (err_1 > 0) {
          currentPoint.y += y_inc;
          err_1 -= dx2;
        }
        if (err_2 > 0) {
          currentPoint.z += z_inc;
          err_2 -= dx2;
        }
        err_1 += dy2;
        err_2 += dz2;
        currentPoint.x += x_inc;
      }
    } else if ((m >= l) && (m >= n)) {
      err_1 = dx2 - m;
      err_2 = dz2 - m;
      for (i = 0; i < m; i++) {
        setPixel(currentPoint, col);
        if (err_1 > 0) {
          currentPoint.x += x_inc;
          err_1 -= dy2;
        }
        if (err_2 > 0) {
          currentPoint.z += z_inc;
          err_2 -= dy2;
        }
        err_1 += dx2;
        err_2 += dz2;
        currentPoint.y += y_inc;
      }
    } else {
      err_1 = dy2 - n;
      err_2 = dx2 - n;
      for (i = 0; i < n; i++) {
        setPixel(currentPoint, col);
        if (err_1 > 0) {
          currentPoint.y += y_inc;
          err_1 -= dz2;
        }
        if (err_2 > 0) {
          currentPoint.x += x_inc;
          err_2 -= dz2;
        }
        err_1 += dy2;
        err_2 += dx2;
        currentPoint.z += z_inc;
      }
    }

    setPixel(currentPoint, col);
  }

  void line(Point p1, Point p2, Color col) {
    line(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, col);
  }

  void sphere(int x, int y, int z, int r, Color col) {
    for(int dx=0;dx<SIDE;dx++) {
      for(int dy=0;dy<SIDE;dy++) {
        for(int dz=0;dz<SIDE;dz++) {
          if(sqrt(pow(x-dx, 2) + pow(y-dy, 2) + pow(z-dz, 2)) <= r) {
            setVoxel(x + dx, y + dy, z + dz, col);
          }
        }
      }
    }
  }

  void sphere(Point p, int r, Color col) {
    sphere(p.x, p.y, p.z, r, col);
  }

  void background(Color col) {
    for(int x=0;x<SIDE;x++)
      for(int y=0;y<SIDE;y++)
        for(int z=0;z<SIDE;z++)
          setVoxel(x, y, z, col);
  }

  Color colorMap(float var, float min, float max) {
    float range=1024;
    val=range*(val-min)/(max-min);
    Color colors[6];
    colors[0].red=0;
    colors[0].green=0;
    colors[0].blue=maxBrightness;

    colors[1].red=0;
    colors[1].green=maxBrightness;
    colors[1].blue=maxBrightness;

    colors[2].red=0;
    colors[2].green=maxBrightness;
    colors[2].blue=0;

    colors[3].red=maxBrightness;
    colors[3].green=maxBrightness;
    colors[3].blue=0;

    colors[4].red=maxBrightness;
    colors[4].green=0;
    colors[4].blue=0;

    colors[5].red=maxBrightness;
    colors[5].green=0;
    colors[5].blue=maxBrightness;

    if (val<=range/6)
      return(lerpColor(&colors[0], &colors[1], val, 0, range/6));
    else if (val<=2*range/6)
      return(lerpColor(&colors[1], &colors[2], val, range/6, 2*range/6));
    else if (val<=3*range/6)
      return(lerpColor(&colors[2], &colors[3], val, 2*range/6, 3*range/6));
    else if (val<=4*range/6)
      return(lerpColor(&colors[3], &colors[4], val, 3*range/6, 4*range/6));
    else if (val<=5*range/6)
      return(lerpColor(&colors[4], &colors[5], val, 4*range/6, 5*range/6));
    else
      return(lerpColor(&colors[5], &colors[0], val, 5*range/6, range));
  }
}
