Spark L3D Cube Library
======================

# About

This is the official library for use with the Spark Core in the [L3D Cube](http://l3dcube.com).

It provides 3D drawing functions for writing visual effects and utility functions for working with color. The library also takes care of communication with the LED strips making up the cube.

The API can be found in l3d-cube.h. Further documentation can be found on the [L3D Cube website](http://docs.l3dcube.com/).

# Example Usage

```C++
using namespace L3D;

Cube cube = Cube(8, 50); // 8^3 cube with max brightness of 50
cube.begin(); // initialize the cube

cube.background(Color(0, 0, 0)); // clear the cube
cube.sphere(4, 4, 4, 3, Color(255, 0, 0)); // draw a red sphere in the center
cube.show(); // update the LEDs to make the changes visible
```

# Building Locally

To compile a firmware binary using a local copy of the library:

1. Follow the instructions for compiling the [Spark firmware](https://github.com/spark/firmware#1-download-and-install-dependencies). Make sure you can successfuly compile the firmware before continuing.
2. Edit the FIRMWARE_DIR variable in the l3d-cube makefile to the path of the spark firmware repository on your machine.
3. Choose an example to compile or put your own code in firmware/examples.
4. Run `make bin/<name of example>.bin` to generate firmware for that example in the bin/ directory. For example, to compile examples/bit-of-everything.cpp run `make bin/bit-of-everything.bin`.
5. Flash the firmware using `spark flash` (the [Spark CLI tool](https://github.com/spark/spark-cli)) or dfu-util.
