/*
Websocket-Arduino, a websocket implementation for Arduino
Copyright 2014 NaAl (h20@alocreative.com)
Based on previous implementations by
Copyright 2011 Per Ejeklint
and
Copyright 2010 Ben Swanson
and
Copyright 2010 Randall Brewer
and
Copyright 2010 Oliver Smith

Some code and concept based off of Webduino library
Copyright 2009 Ben Combee, Ran Talbott

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

-------------
Now based off
http://www.whatwg.org/specs/web-socket-protocol/

- OLD -
Currently based off of "The Web Socket protocol" draft (v 75):
http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-75
 */

#ifndef _SPARK_WEB_SOCKET_H_
#define _SPARK_WEB_SOCKET_H_

#include "application.h"
#include "spark_utilities.h"

#define CRLF "\r\n"

#define HB_INTERVAL 2500
#define TIMEOUT 5000

#ifndef CALLBACK_FUNCTIONS
#define CALLBACK_FUNCTIONS 1
#endif

/**
 * call back function pointer.
 * the main app should create a function pointer and set it.
 * the function will be called with request string
 */
typedef void (*CallBack)(String&, String&);

class SparkWebSocketServer {
  public:
    SparkWebSocketServer(TCPServer &server);

    void setCallBack(CallBack &callBack){
      cBack = callBack;
    }

    bool handshake(TCPClient &client);

    bool getData(String &data, TCPClient &client);

    void sendData(const char *str, TCPClient &client);
    void sendData(String str, TCPClient &client);

    void doIt();

    CallBack cBack;

  private:
    const int packetLen = 520;
    const int dataLen = packetLen - 8; // length bytes and mask

    unsigned long lastBeatTime;
    unsigned long lastContactTime;
    TCPServer* server;
    TCPClient* source;

    String origin;
    String host;

    bool analyzeRequest(TCPClient &client);
    bool handleStream(String &data, TCPClient &client);
    int packetHealth(char* buffer);

    void disconnectClient(void);

    int checkedRead(TCPClient &client);

    void sendEncodedData(char *str, TCPClient &client);
    void sendEncodedData(String str, TCPClient &client);
};

#endif
