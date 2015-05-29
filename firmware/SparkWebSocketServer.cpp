/*******************************************************************************
 * Websocket-Arduino, a websocket implementation for Arduino
 * Based on previous implementations by
 * Copyright 2014 NaAl (h20@alocreative.com)
 * and
 * Copyright 2011 Per Ejeklint
 * and
 * Copyright 2010 Ben Swanson
 * and
 * Copyright 2010 Randall Brewer
 * and
 * Copyright 2010 Oliver Smith

 * Some code and concept based off of Webduino library
 * Copyright 2009 Ben Combee, Ran Talbott

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * -------------
 * Now based off
 * http://www.whatwg.org/specs/web-socket-protocol/
 * 
 * - OLD -
 * Currently based off of "The Web Socket protocol" draft (v 75):
 * http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75
 *******************************************************************************/

#include "SparkWebSocketServer.h"

#include "Base64.h"
#include "tropicssl/sha1.h"

static TCPClient* blankClient = new TCPClient(MAX_SOCK_NUM);

SparkWebSocketServer::SparkWebSocketServer(TCPServer &tcpServer)
{
    source = NULL;
    server = &tcpServer;
}

bool SparkWebSocketServer::handshake(TCPClient &client)
{
    // there is an empty spot
    // check request and look for websocket handshake
#ifdef DEBUG_WS
    Serial.println("Handshake: analyzing.");
#endif

    if(analyzeRequest(client)) {
        // valid WebSocket connection
        if(source != NULL)
            disconnectClient();

        // keep track of new connection
        source = &client;

#ifdef DEBUG_WS
        Serial.println("WebSocket connection established.");
#endif
        return true;
    } else {
        return false;
    }
}

/** Disconnect client from server. */
void SparkWebSocketServer::disconnectClient()
{
#ifdef DEBUG_WS
    Serial.print("Terminating TCPClient.");
#endif

    // should send 0x8700 to server to tell it I'm quitting here.
    source->write((uint8_t) 0x88);
    source->write((uint8_t) 0x00);

    source->flush();
    delay(10);
    source->stop();

    source = NULL;
}

/** Read data from client.
  @param data String to read the received data into.
  @param client TCPClient to get the data from.
*/
PacketType SparkWebSocketServer::readPacket(String &data, TCPClient &client)
{
    char buffer[packetLen];
    PacketType type = TYPE_NONE;

    if (client.connected()) {
        for (int count = 0; client.available() > 0 && count < packetLen; count++) {
            buffer[count] = client.read();
        }

        int opcode = buffer[0] & 0xF;

        switch (opcode) {
            case 0:
                type = TYPE_CONTINUATION;
                break;
            case 1:
                type = TYPE_TEXT;
                break;
            case 2:
                type = TYPE_BINARY;
                break;
            case 8:
                type = TYPE_CLOSE;
                break;
            case 9:
                type = TYPE_PING;
                break;
            case 10:
                type = TYPE_PONG;
                break;
            default:
                type = TYPE_BAD;
        }

        if (type == TYPE_TEXT || type == TYPE_BINARY) {
            int lengthType = buffer[1] & 127;
            int length = (buffer[2] << 8) | buffer[3];
            if(lengthType != 126 || length != dataLen)
                return TYPE_BAD;

            for (int i = 0; i < length && i < packetLen; i++) {
                data += (char) (buffer[i+8] ^ buffer[4 + i % 4]);
            }
        }
    }

    return type;
}

/** Read one value from a client. */
int SparkWebSocketServer::checkedRead(TCPClient &client)
{
    while(!client.available());
    return client.read();
}

/** Send a string to a client. */
void SparkWebSocketServer::sendEncodedData(char *str, TCPClient &client)
{
    int size = strlen(str);

    if(!client) return;

    // NOTE: no support for > 16-bit sized messages
    if(size > 125) {
        uint8_t buffer[size+4];
        strcpy((char*)(buffer+4), str);

        buffer[0] = 0x81; // string type
        buffer[1] = 126;
        buffer[2] = size >> 8;
        buffer[3] = size & 0xFF;

        client.write(buffer, sizeof(buffer));
    } else {
        uint8_t buffer[size+2];
        strcpy((char*)(buffer+2), str);

        buffer[0] = 0x81; // string type
        buffer[1] = size;

        client.write(buffer, sizeof(buffer));
    }
}

/** Send a string to a client. */
void SparkWebSocketServer::sendEncodedData(String str, TCPClient &client)
{
    int size = str.length() + 1;
    char cstr[size];

    str.toCharArray(cstr, size);

    sendEncodedData(cstr, client);
}

/** Send a string to a client. */
void SparkWebSocketServer::sendData(const char *str, TCPClient &client)
{
    if (client && client.connected()) {
        sendEncodedData(str, client);
    }
}

/** Send a string to a client. */
void SparkWebSocketServer::sendData(String str, TCPClient &client)
{
    if(client && client.connected()) {
        sendEncodedData(str, client);
    }
}

void SparkWebSocketServer::tick()
{
    // heartbeat

    unsigned long currentTime = millis();
    bool beat = currentTime - lastBeatTime > HB_INTERVAL;

    if(beat)
        lastBeatTime = currentTime;

    // check for client

    TCPClient* client = blankClient;

    if(source == NULL || !source->connected()) {
        *client = server->available();

        if(client != NULL && client->connected()) {
            // attempt to initiate connection
#ifdef DEBUG_WS
            bool success = handshake(*client);

            if(success) {
                Serial.println("Handshake successful.");
            } else {
                Serial.println("Handshake FAILED.");
            }
#else
            handshake(*client);
#endif
            lastBeatTime = millis();
            lastContactTime = millis();
        }
    }

    // tick client

    if(source != NULL) {
        if(!source->connected()) {
#ifdef DEBUG_WS
            Serial.println("Found disconnect in tick.");
#endif
            disconnectClient();
        } else {
            String req;
            PacketType type = readPacket(req, *source);

            if (type == TYPE_BINARY) {
#ifdef DEBUG_WS
                Serial.print("got : ");
                Serial.println(req);
#endif
                String result;
                (*cBack)(req, result);
                lastContactTime = millis();
#ifdef DEBUG_WS
                Serial.print("result: ");
                Serial.println(result);
#endif

                sendData(result, *source);
            } else if (type == TYPE_CLOSE) {
                disconnectClient();
            }
        }

        // disconnect client on timeout
        if(millis() - lastContactTime > TIMEOUT)
            disconnectClient();
    }
}

/** Analyze request and respond if it is for a Websocket connection.
  @param client Client making the request.
  @return True if successful Websocket connection is made. False otherwise.
  */
bool SparkWebSocketServer::analyzeRequest(TCPClient &client)
{
    String temp;

    int bite;
    bool foundUpgrade = false;
    String oldKey[2];
    String newKey;

#ifdef DEBUG_WS
    Serial.println("Analyzing request headers.");
#endif

    // TODO: More robust string extraction
    while((bite = client.read()) != -1) {
        temp += (char)bite;

        // end of line?
        if((char)bite == '\n') {
#ifdef DEBUG_WS
            Serial.print("Got Line: " + temp);
#endif
            // TODO: Should ignore case when comparing and allow 0-n whitespace after ':'. See the spec:
            // http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
            if(!foundUpgrade && temp.startsWith("Upgrade: WebSocket")) {
                foundUpgrade = true; // OK, it's a websockets handshake for sure
#ifdef DEBUG_WS
                Serial.println("hixie76style not supported.");
#endif
                disconnectClient();
            } else if(!foundUpgrade && temp.startsWith("Upgrade: websocket")) {
                foundUpgrade = true; // OK, it's a websockets handshake for sure
            } else if(temp.startsWith("Origin: ")) {
                origin = temp.substring(8, temp.length() - 2); // Don't save last CR+LF
            } else if(temp.startsWith("Host: ")) {
                host = temp.substring(6, temp.length() - 2); // Don't save last CR+LF
            } else if(temp.startsWith("Sec-WebSocket-Key1: ")) {
                oldKey[0] = temp.substring(20, temp.length() - 2); // Don't save last CR+LF
            } else if(temp.startsWith("Sec-WebSocket-Key2: ")) {
                oldKey[1] = temp.substring(20, temp.length() - 2); // Don't save last CR+LF
            } else if(temp.startsWith("Sec-WebSocket-Key: ")) {
                newKey = temp.substring(19, temp.length() - 2); // Don't save last CR+LF
            }

            temp = "";
        }

        if(!client.available()) {
            delay(20);
        }
    }

    if(!client.connected()) {
        return false;
    }

    temp += 0; // Terminate string

    // Assert that we have all headers that are needed. If so, go ahead and
    // send response headers.
    if(foundUpgrade == true) {
        if (newKey.length() > 0) {
#ifdef DEBUG_WS
            Serial.println("!hixie76style: " + newKey);
#endif
            // add the magic string
            newKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

            uint8_t hash[100];
            char result[21];
            char b64Result[30];

            //char hexstring[41];
            unsigned char a[newKey.length()+1];
            a[newKey.length()] = 0;
            memcpy(a, newKey.c_str(), newKey.length());
            sha1(a, newKey.length(), hash); // 10 is the length of the string
            //toHexString(hash, hexstring);

            for (uint8_t i = 0; i < 20; ++i) {
                result[i] = (char)hash[i];
            }

            result[20] = '\0';

            base64_encode(b64Result, result, 20);

            client.print("HTTP/1.1 101 Web Socket Protocol Handshake\r\n");
            client.print("Upgrade: websocket\r\n");
            client.print("Connection: Upgrade\r\n");
            client.print("Sec-WebSocket-Accept: ");
            client.print(b64Result);
            client.print(CRLF);
            client.print(CRLF);

            return true;
        } else {
            // something went horribly wrong
#ifdef DEBUG_WS
            Serial.println("Something went horribly wrong.");
#endif
            return false;
        }
    } else {
        // Nope, failed handshake. Disconnect
#ifdef DEBUG_WS
        Serial.println("Header mismatch.");
#endif
        return false;
    }
}
