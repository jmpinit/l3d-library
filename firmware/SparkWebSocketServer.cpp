/*******************************************************************************
 * Websocket-Arduino, a websocket implementation for Arduino
 * Copyright 2014 NaAl (h20@alocreative.com)
 * Based on previous implementations by
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

#define DEBUG_WS // FIXME

#ifdef SUPPORT_HIXIE_76
#include "MD5.cpp"
#endif

static TCPClient* blankClient = new TCPClient(MAX_SOCK_NUM);

SparkWebSocketServer::SparkWebSocketServer(TCPServer &tcpServer)
{
    for(uint8_t i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = NULL;
    }

    server = &tcpServer;
    previousMillis = 0;
}

TCPClient** SparkWebSocketServer::getFreeClientSlot()
{
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == NULL)
            return &clients[i];
    }

    return NULL;
}

bool SparkWebSocketServer::handshake(TCPClient &client)
{
    uint8_t pos = 0;
    bool found = false;

    // look for a prior connection from this client
    /*for(pos = 0; pos < MAX_CLIENTS; pos++) {
        if(clients[pos] != NULL && client.equals(*clients[pos])) {
#ifdef DEBUG_WS
            String ip;
            client.getIP(ip);
            Serial.print(" SparkWebSocketServer::handshake, client[");
            Serial.print(pos);
            Serial.print("]: ");
            Serial.print(ip);
            Serial.println(", re-using!!");
            delay(10);
#endif
            found = true;
            break;
        }
    }*/

    if(!found) {
        // find an unused slot
        for(pos = 0; pos < MAX_CLIENTS; pos++) {
            if(clients[pos] == NULL) {
                break;
            }
        }
    }

    if(pos >= MAX_CLIENTS) {
        // no room for new connection
        disconnectClient(client);
#ifdef DEBUG_WS
        Serial.println("Handshake, but no room for new connection.");
        Serial.println("Disconnecting/no free space for client");
#endif
        return false;
    } else {
        // there is an empty spot
        // check request and look for websocket handshake
#ifdef DEBUG_WS
        String ip;
        //client.getIP(ip);
        Serial.print(" SparkWebSocketServer::handshake, client[");
        Serial.print(pos);
        Serial.print("]: ");
        Serial.print(ip);
        Serial.println(", analyzing");
#endif

        if(analyzeRequest(client)) {
            // connection established. add client
            clients[pos] = &client;
#ifdef DEBUG_WS
            String ip;
            //client.getIP(ip);
            Serial.print("SparkWebSocketServer established, ");
            Serial.println(ip);
            if(clients[pos]->connected())
                Serial.println("and client still connected.");
            else
                Serial.println("but client disconnected!");
#endif
            return true;
        } else {
            // might just need to break until out of socket_client loop
            return false;
        }
    }
}

/** Disconnect client from server.
  @param client Client to disconnect.
  */
void SparkWebSocketServer::disconnectClient(TCPClient &client)
{
#ifdef DEBUG_WS
    Serial.print("Terminating TCPClient: ");
    String ip;
    //client.getIP(ip);
    Serial.println(ip);
#endif

    if(hixie76style) {
#ifdef SUPPORT_HIXIE_76
        // should send 0xFF00 to server to tell it I'm quitting here.
        client.write((uint8_t) 0xFF);
        client.write((uint8_t) 0x00);
#endif
    } else {
        // should send 0x8700 to server to tell it I'm quitting here.
        client.write((uint8_t) 0x87);
        client.write((uint8_t) 0x00);
    }

    client.flush();
    delay(10);
    client.stop();

    // remove client from the list
    for(uint8_t i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != NULL && client == *clients[i]) {
#ifdef DEBUG_WS
            String ip;
            //client.getIP(ip);
            Serial.print("found: ");
            Serial.print(ip);
            Serial.println(" and removing it from the list of the clients");
#endif
            TCPClient *tmp = clients[i];
            delete tmp;
            clients[i] = NULL;
            break;
        }
    }
}

void SparkWebSocketServer::getData(String &data, TCPClient &client)
{
    if(hixie76style) {
#ifdef SUPPORT_HIXIE_76
        handleHixie76Stream(data, client);
#endif
    } else {
        handleStream(data, client);
    }
}

/** Read data from client.
  @param data String to read the received data into.
  @param client TCPClient to get the data from.
*/
void SparkWebSocketServer::handleStream(String &data, TCPClient &client)
{
    int length;
    uint8_t mask[4];

    if(client.connected()) {
        length = timedRead(client);
        if(!client.connected() || length==-1) {
            // no data to handle
            return ;
        }

        length = timedRead(client) & 127;
        if(!client.connected()) return ;

        if(length == 126) {
            length = timedRead(client) << 8;
            if (!client.connected()) return ;

            length |= timedRead(client);
            if (!client.connected()) return ;
        } else if(length == 127) {
#ifdef DEBUG_WS
            Serial.println("No support for over 16 bit sized messages");
#endif
            return;
        }

        // get the mask
        mask[0] = timedRead(client);
        if(!client.connected()) return;
        mask[1] = timedRead(client);
        if(!client.connected()) return;
        mask[2] = timedRead(client);
        if(!client.connected()) return;
        mask[3] = timedRead(client);
        if(!client.connected()) return;

        for(int i = 0; i < length; ++i) {
            data += (char) (timedRead(client) ^ mask[i % 4]);
            if(!client.connected()) return;
        }
    }
}

/** Read one value from a client.
  */
int SparkWebSocketServer::timedRead(TCPClient &client)
{
    uint8_t test = 0;

    while (test < 20 && !client.available() && client.connected()) {
        delay(1);
        test++;
    }

    if(client.connected()) {
        return client.read();
    }

    return -1;
}

/** Send a string to a client.
  */
void SparkWebSocketServer::sendEncodedData(char *str, TCPClient &client)
{
    int size = strlen(str);

    if(!client) return;

    // string type
    client.write(0x81);

    // NOTE: no support for > 16-bit sized messages
    if(size > 125) {
        client.write(126);
        client.write((uint8_t) (size >> 8));
        client.write((uint8_t) (size && 0xFF));
    } else {
        client.write((uint8_t) size);
    }

    for(int i = 0; i < size; ++i) {
        client.write(str[i]);
    }
}

/** Send a string to a client.
  */
void SparkWebSocketServer::sendEncodedData(String str, TCPClient &client)
{
    int size = str.length() + 1;
    char cstr[size];

    str.toCharArray(cstr, size);

    sendEncodedData(cstr, client);
}

/** Send a string to a client.
  */
void SparkWebSocketServer::sendData(const char *str, TCPClient &client)
{
    if(client && client.connected()) {
        if(hixie76style) {
            client.print(0x00); // Frame start
            client.print(str);
            client.write(0xFF); // Frame end
        } else {
            sendEncodedData(str, client);
        }
    }
}

/** Send a string to a client.
  */
void SparkWebSocketServer::sendData(String str, TCPClient &client)
{
    if(client && client.connected()) {
        if(hixie76style) {
            client.print((char)0x00); // Frame start
            client.print(str);
            client.write(0xFF); // Frame end
        } else {
            sendEncodedData(str, client);
        }
    }
}
void SparkWebSocketServer::doIt()
{
    // handle new clients

    unsigned long currentMillis = millis();
    bool beat = currentMillis - previousMillis > HB_INTERVAL;

    if(beat) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
    }

    static int count = 0; // FIXME

    /*Serial.print(count);
    Serial.print(" ) ");
    if(clients[0] == NULL || !clients[0]->connected()) {
        Serial.println("NOT CONNECTED");
    } else {
        Serial.println("CONNECTED");
    }*/

    TCPClient* client = blankClient;
    if(clients[0] == NULL || !clients[0]->connected()) {
        *client = server->available();

        if(client != NULL && client->connected()) {
#ifdef DEBUG_WS
            String ip;
            //client.getIP(ip);
            Serial.print(count);
            Serial.print(") new client connecting, testing: ");
            Serial.println(ip);
#endif
            // attempt to initiate connection
            bool success = handshake(*client);

            if(success) {
                Serial.println("handshake successful");
            } else {
                Serial.println("handshake FAILED");
            }
        }
    }

    for(uint8_t i = 0; i < MAX_CLIENTS; i++) {
        TCPClient *myClient = clients[i];

        if(myClient == NULL) continue;

        if(!myClient->connected()) {
#ifdef DEBUG_WS
            String ip;
            //myClient->getIP(ip);
            Serial.print(count);
            Serial.print(") Client[");
            Serial.print(i);
            Serial.print("]: ");
            Serial.print(ip);
            Serial.println(" disconnected!");
#endif
            disconnectClient(*myClient);
        } else {
            String req;
            getData(req, *myClient);

            if(req.length() > 0) {
#ifdef DEBUG_WS
                String ip;
                //myClient->getIP(ip);
                Serial.print("got : ");
                Serial.print(req + " from: ");
                Serial.println(ip);
                delay(1000);
#endif
                String result;
                (*cBack)(req, result);
#ifdef DEBUG_WS
                Serial.print("result: ");
                Serial.println(result);
#endif
                sendData(result, *myClient);
            } else {
                if(beat) {
                    Serial.println("beat");

                    if(myClient->connected()) {
#ifdef DEBUG_WS
                        String ip;
                        //myClient->getIP(ip);
                        Serial.print("sending HB to: ");
                        Serial.println(ip);
#endif
                        // TODO can this be removed?
                        sendData("HB", *myClient);
                    } else {
#ifdef DEBUG_WS
                        Serial.println("client found disconnected in beat.");
#endif
                        disconnectClient(*myClient);
                    }
                }
            }
        }
    }

    count++;
}

/** Analyze request and respond if it is for a Websocket connection.
  @param client Client making the request.

  @return True if successful Websocket connection is made. False otherwise.
  */
bool SparkWebSocketServer::analyzeRequest(TCPClient &client)
{
    // Use String library to do some sort of read() magic here.
    String temp;

    int bite;
    bool foundUpgrade = false;
    String oldKey[2];
    String newKey;

    hixie76style = false;

#ifdef DEBUG_WS
    Serial.println("Analyzing request headers");
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
                // OK, it's a websockets handshake for sure
                foundUpgrade = true;
                hixie76style = true;
            } else if(!foundUpgrade && temp.startsWith("Upgrade: websocket")) {
                foundUpgrade = true;
                hixie76style = false;
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
#ifdef SUPPORT_HIXIE_76
        if (hixie76style && host.length() > 0 && oldKey[0].length() > 0 && oldKey[1].length() > 0) {
            // All ok, proceed with challenge and MD5 digest
            char key3[9] = {0};
            // What now is in temp should be the third key
            temp.toCharArray(key3, 9);

            // Process keys
            for(int i = 0; i <= 1; i++) {
                unsigned int spaces = 0;
                String numbers;

                for(int c = 0; c < oldKey[i].length(); c++) {
                    char ac = oldKey[i].charAt(c);

                    if(ac >= '0' && ac <= '9') {
                        numbers += ac;
                    }

                    if(ac == ' ') {
                        spaces++;
                    }
                }

                char numberschar[numbers.length() + 1];
                numbers.toCharArray(numberschar, numbers.length() + 1);
                intkey[i] = strtoul(numberschar, NULL, 10) / spaces;
            }

            unsigned char challenge[16] = {0};
            challenge[0] = (unsigned char) ((intkey[0] >> 24) & 0xFF);
            challenge[1] = (unsigned char) ((intkey[0] >> 16) & 0xFF);
            challenge[2] = (unsigned char) ((intkey[0] >>  8) & 0xFF);
            challenge[3] = (unsigned char) ((intkey[0]      ) & 0xFF);
            challenge[4] = (unsigned char) ((intkey[1] >> 24) & 0xFF);
            challenge[5] = (unsigned char) ((intkey[1] >> 16) & 0xFF);
            challenge[6] = (unsigned char) ((intkey[1] >>  8) & 0xFF);
            challenge[7] = (unsigned char) ((intkey[1]      ) & 0xFF);

            memcpy(challenge + 8, key3, 8);

            unsigned char md5Digest[16];
            MD5(challenge, md5Digest, 16);

            client.print("HTTP/1.1 101 Web Socket Protocol Handshake\r\n");
            client.print("Upgrade: WebSocket\r\n");
            client.print("Connection: Upgrade\r\n");
            client.print("Sec-WebSocket-Origin: ");
            client.print(origin);
            client.print(CRLF);

            // The "Host:" value should be used as location
            client.print("Sec-WebSocket-Location: ws://");
            client.print(host);
            client.print(socket_urlPrefix);
            client.print(CRLF);
            client.print(CRLF);

            client.write(md5Digest, 16);

            return true;
        }
#endif

        if (!hixie76style && newKey.length() > 0) {
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
            return false;
        }
    } else {
        // Nope, failed handshake. Disconnect
#ifdef DEBUG_WS
        Serial.println("Header mismatch");
#endif
        return false;
    }
}

#ifdef SUPPORT_HIXIE_76
String SparkWebSocketServer::handleHixie76Stream(String &socketString, TCPClient &client)
{
    int bite;
    int frameLength = 0;
    // String to hold bytes sent by client to server.

    if (client.connected() && client.available()) {
        bite = timedRead(client);

        if (bite != -1) {
            if (bite == 0)
                continue; // Frame start, don't save

            if ((uint8_t) bite == 0xFF) {
                // Frame end. Process what we got.

            } else {
                socketString += (char)bite;
                frameLength++;

                if (frameLength > MAX_FRAME_LENGTH) {
                    // Too big to handle!
#ifdef DEBUG_WS
                    Serial.print("Client send frame exceeding ");
                    Serial.print(MAX_FRAME_LENGTH);
                    Serial.println(" bytes");
#endif
                    return;
                }
            }
        }
    }

}

#endif
