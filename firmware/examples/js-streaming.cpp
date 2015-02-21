#include <math.h>

#include "SparkWebSocketServer.h"

#include "l3d-cube/l3d-cube.h"

void debug_output_(const char* msg) {
    Serial.print(msg);
}

TCPServer server = TCPServer(2525);
Cube cube = Cube();

void handle(String &cmd, String &result);

SparkWebSocketServer mine(server);

char ipString[24];
void setup()
{
    Spark.disconnect();

    CallBack cb = &handle;
    mine.setCallBack(cb);

    Serial.begin(9600);

    //Spark.connect();

    // run some test code so we know the core is running!
    RGB.control(true);
    bool s = false;

    while(!Serial.available()) {
        s = !s; // toggle the state
        RGB.brightness(10);

        if(s) {
            RGB.color(0,0,255);
            delay(500);
        } else {
            RGB.color(0,255,0);
            delay(200);
            RGB.color(255,0,255);
            delay(300);
        }
    }

    RGB.color(0, 0, 0);
    RGB.control(false);

    //netapp_ipconfig(&ip_config);
    server.begin();

    Serial.println(WiFi.localIP());
    Serial.println(WiFi.subnetMask());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.SSID());

    IPAddress ip = WiFi.localIP();
    sprintf(ipString, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    //Spark.variable("ip", &ipString, STRING);

    cube.begin();
    cube.background(black);

    Serial.println("Setup done");
}

/**
 * check the cmd (client request), do what needs to be done and store the result
 * int the result
 * @param cmd  client request
 * @param result the string to be send back to the client
 */
void handle(String &data, String &result)
{
    Serial.print("in handle, got: ");
    Serial.println(data.length());

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
                    Serial.print((int)data[index]);
                    Serial.print(", ");
                    cube.setVoxel(x, y, z, pixelColor);
                }
            }
        }
    }

    cube.show();
}

void loop()
{
    mine.doIt();
    delay(100);
}
