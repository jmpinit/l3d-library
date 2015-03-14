#include <sstream>
#include <math.h>

#include "SparkWebSocketServer.h"

#include "l3d-cube/l3d-cube.h"

TCPServer server = TCPServer(2525);
Cube cube = Cube();

void handle(String &cmd, String &result);

SparkWebSocketServer mine(server);

char ipString[24];
void setup()
{
    CallBack cb = &handle;
    mine.setCallBack(cb);

    Serial.begin(1000000);

    while(Serial.available() == 0);

    IPAddress ip = WiFi.localIP();
    sprintf(ipString, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    Spark.connect();
    Spark.variable("ip", &ipString, STRING);

    server.begin();

    Serial.println(WiFi.localIP());
    Serial.println(WiFi.subnetMask());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.SSID());

    cube.begin();
    cube.background(black);

    Serial.println("Setup done");
}

/**
 * Handle client requests and reply.
 * @param data string from client
 * @param result string to client
 */
void handle(String &data, String &result)
{
    //Serial.print("in handle, got: ");
    //Serial.println(data.length());

    if(data.length() == 512) {
        for(unsigned int x = 0; x < 8; x++) {
            for(unsigned int y = 0; y < 8; y++) {
                for(unsigned int z = 0; z < 8; z++) {
                    int index = z*64 + y*8 + x;
                    //colors with max brightness set to 64
                    uint8_t red = (data[index]&0xE0)>>2;
                    uint8_t green = (data[index]&0x1C)<<1;
                    uint8_t blue = (data[index]&0x03)<<4;
                    Color pixelColor = Color(red, green, blue);
                    cube.setVoxel(x, y, z, pixelColor);
                }
            }
        }

        cube.show();
    }

    result += String(data.length());
}

void loop()
{
    mine.doIt();
}
