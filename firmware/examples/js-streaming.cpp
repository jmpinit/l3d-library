#include <sstream>
#include <math.h>

#include "SparkWebSocketServer.h"

#include "l3d-cube/l3d-cube.h"
#include "crc32.h"

Cube cube = Cube();

void handle(String &cmd, String &result);

uint32_t crc32(uint32_t crc, String& buf, size_t size)
{
    int p = 0;
    crc = crc ^ ~0U;

    while (size--)
        crc = crc32_tab[(crc ^ buf[p++]) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;
}

void stream() {
    static bool initialized = false;

    static TCPServer server = TCPServer(2525);
    static SparkWebSocketServer streamServer(server);

    
    if (!initialized) {
        CallBack cb = &handle;
        streamServer.setCallBack(cb);

        server.begin();

        initialized = true;
    }

    streamServer.tick();
}

void setup()
{
    Serial.begin(1000000);

    //while(Serial.available() == 0);
    while(!WiFi.ready());

    char ipString[24];
    IPAddress ip = WiFi.localIP();
    sprintf(ipString, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    Spark.connect();
    Spark.variable("ip", &ipString, STRING);

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

    if(data.length() == 512 + 4) {
        uint32_t crc = crc32(0, data, 512);
        if (((crc >> 24) & 0xff) != data[512])
            goto done;
        if (((crc >> 16) & 0xff) != data[513])
            goto done;
        if (((crc >> 8) & 0xff) != data[514])
            goto done;
        if ((crc & 0xff) != data[515])
            goto done;

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

done:
    result += String(data.length());
}

void loop()
{
    stream();
}
