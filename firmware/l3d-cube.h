#ifndef _L3D_CUBE_H
#define _L3D_CUBE_H

#include "application.h"
#include "neopixel.h"

#define PIXEL_COUNT 512
#define PIXEL_PIN D0
#define PIXEL_TYPE WS2812B

#define INTERNET_BUTTON D2
#define MODE D3

#define STREAMING_PORT 2222

namespace L3D
{
  /**   An RGB color. */
  struct Color
  {
    unsigned char red, green, blue;

    Color(int r, int g, int b) : red(r), green(g), blue(b) {}
    Color() : red(0), green(0), blue(0) {}
  };

  /**   A point in 3D space.  */
  struct Point
  {
    float x;
    float y;
    float z;

    Point(int x, int y, int z) : x(x), y(y), z(z) {}
  };

  /**   An L3D LED cube.
        Provides methods for drawing in 3D. Controls the LED hardware.
  */
  class Cube
  {
    unsigned int size;
    unsigned int maxBrightness;
    bool onlinePressed;
    bool lastOnline;
    Adafruit_NeoPixel strip;
    UDP udp;
    int lastUpdated;
    char localIP[24];
    char macAddress[20];
    int port;

    public:
      Cube(unsigned int s, unsigned int mb);
      Cube(void);

      void setVoxel(int x, int y, int z, Color col);
      void setVoxel(Point p, Color col);
      Color getVoxel(int x, int y, int z);
      Color getVoxel(Point p);
      void line(int x1, int y1, int z1, int x2, int y2, int z2, Color col);
      void line(Point p1, Point p2, Color col);
      void sphere(int x, int y, int z, int r, Color col);
      void sphere(Point p, int r, Color col);
      void background(Color col);

      Color colorMap(float val, float min, float max);
      Color lerpColor(Color a, Color b, int val, int min, int max);

      void begin(void);
      void show(void);
      void listen(void);
      void initCloudButton(void);
      void checkCloudButton(void);
      void updateNetworkInfo(void);
      int setPort(String port);
  };
}

#endif
