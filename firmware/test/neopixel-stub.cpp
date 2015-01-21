#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "stub.h"
#include "neopixel.h"

using namespace std;

Adafruit_NeoPixel::Adafruit_NeoPixel(uint16_t n, uint8_t p, uint8_t t) : \
  numLEDs(n), numBytes(n*3), type(t), pin(p), pixels(NULL)
{
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
  }
}

Adafruit_NeoPixel::~Adafruit_NeoPixel() {
  if(pixels) free(pixels);
}

void Adafruit_NeoPixel::begin(void) {
}

void Adafruit_NeoPixel::show(void) {
  if(!pixels) return;

  cout << "show()";
}

// Set the output pin number
void Adafruit_NeoPixel::setPin(uint8_t p) {
  pin = p;
  cout << "setPin(" << p << ")";
}

// Set pixel color from separate R,G,B components:
void Adafruit_NeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 3];
    switch(type) {
      case WS2812B: // WS2812 & WS2812B is GRB order.
        *p++ = g;
        *p++ = r;
        *p = b;
        break;
      case TM1829: // TM1829 is special RBG order
        if(r == 255) r = 254; // 255 on RED channel causes display to be in a special mode.
        *p++ = r;
        *p++ = b;
        *p = g;
        break;
      case WS2811: // WS2811 is RGB order
      case TM1803: // TM1803 is RGB order
      default:     // default is RGB order
        *p++ = r;
        *p++ = g;
        *p = b;
        break;
    }
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void Adafruit_NeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 3];
    switch(type) {
      case WS2812B: // WS2812 & WS2812B is GRB order.
        *p++ = g;
        *p++ = r;
        *p = b;
        break;
      case TM1829: // TM1829 is special RBG order
        if(r == 255) r = 254; // 255 on RED channel causes display to be in a special mode.
        *p++ = r;
        *p++ = b;
        *p = g;
        break;
      case WS2811: // WS2811 is RGB order
      case TM1803: // TM1803 is RGB order
      default:     // default is RGB order
        *p++ = r;
        *p++ = g;
        *p = b;
        break;
    }
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_NeoPixel::getPixelColor(uint16_t n) const {

  if(n < numLEDs) {
    uint16_t ofs = n * 3;
    return (uint32_t)(pixels[ofs + 2]) |
      ((uint32_t)(pixels[ofs    ]) <<  8) |
      ((uint32_t)(pixels[ofs + 1]) << 16);
  }

  return 0; // Pixel # is out of bounds
}

uint8_t *Adafruit_NeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t Adafruit_NeoPixel::numPixels(void) const {
  return numLEDs;
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void Adafruit_NeoPixel::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if(newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(uint16_t i=0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}
