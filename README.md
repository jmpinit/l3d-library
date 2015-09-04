Spark Core L3D Cube Library
===========================

## About

This library provides 3D drawing functions for writing visual effects and utility functions for working with color. The library also takes care of communication with the LED strips making up the cube.

The API can be found in l3d-cube.h.

## Example Usage

```C++
Cube cube = Cube(8, 50); // 8^3 cube with max brightness of 50
cube.begin(); // initialize the cube

cube.background(Color(0, 0, 0)); // clear the cube
cube.sphere(4, 4, 4, 3, Color(255, 0, 0)); // draw a red sphere in the center
cube.show(); // update the LEDs to make the changes visible
```
