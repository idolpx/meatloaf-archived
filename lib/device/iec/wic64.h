
//
// WiC64 - C64 connected to ESP32
//
// written by KiWI 2020-2021
//
// Using ESP32 Arduino Release 1.0.5 based on ESP-IDE v3.3
//          WiC64 Hardware & Software - Copyright (c) 2021
//
//               Thomas "GMP" Müller <gmp@wic64.de>
//             Sven Oliver "KiWi" Arke <kiwi@wic64.de>
//          Hardy "Lazy Jones" Ullendahl <lazyjones@wic64.de>
//             Henning "Yps" Harperath <yps@wic64.de>
//
//
//           All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//
// C64 Pinout -> ESP Wroom >>30<< Pin Version
//
// https://www.c64-wiki.de/wiki/Userport
//
// https://randomnerdtutorials.com/getting-started-with-esp32/      30 Pin Version !!! Not the 36 !!!
//


#ifndef DEVICE_WIC64_H
#define DEVICE_WIC64_H

#include <string>

class WiC64 
{
public:

    bool displayattached = false;
    bool inputmode = true;
    bool transferdata = false;
    bool ex = false;                          // Flag whether ESP is currently working on a command

    unsigned long crashcounter = 0;           // Counter for the number of commandos - only for debugging
    unsigned long payloadsize;                // Size of the data to be transferred
    unsigned long timeout;                    // Timeout Counter in Millis
    unsigned long count = 0;
    unsigned long sizel;                      // Counter Number of bytes from the C64 low byte
    unsigned long sizeh;                      // Counter Number of bytes from the C64 High Byte
    unsigned long transsize;                  // Number of bytes that should come from the C64

    std::string defaultserver = "http://www.wic64.de/prg/"; // To test the Wic64 Kernel Load ! 

    int databyte = 0;                         // The byte what is read or written on the bus

    bool killswitch = false;                  // Deactivate WIC64 from user port or activate
    bool killled = false;                     // LED on or off
    std::string payload;                      // Data buffer for transmission
    std::string errorcode;                    // Loading error Errorcode
    std::string act;                          // Display display what is happening ...
    std::string input;                        // Input test from the C64
    std::string lastinput;                    // Buffer input ring
    std::string setserver;                    // std::stringpuffer for SETS
    std::string httpstring;                   // HTTP Address
    std::string wic64hostname;                // WiC64 Hostname consisting of WIC64+MAC of the ESP
    std::string localip;                      // IP of the C64 WiC
    std::string localssid;                    // WLAN to which was connected
    char sep = 1;                             // Byte for separating the individual std::strings (separation)
    int udpport = 8080;                       // UDP message port
    int tcpport = 8081;                       // TCP message port
    std::string udpmsg;
    std::string udpmsgr;
    std::string remotedata;
    uint8_t buffer[50];
    // IPAddress ip;
    // WiFiUDP udp;
    // Preferences preferences;
    // WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80
    // HTTPClient http;         // HTTP Client create
    // WiFiClient client;       // WiFi Client create
    // WiFiClientSecure clientsec; // WiFi Client https create


    void setup();
    void loop();


    // Subs Commands HTTP & Error codes

    std::string sendmessage ( std::string errorcode );
    void disablewic();
    std::string setwlan();

    void loader ( std::string httpstring );
    void sendudpmsg ( std::string udpmsg );
    std::string getudpmsg();

    void startudpport();
    void settcpport();

    std::string getValue ( std::string data, char separator, int index );

    void stringtohexdump ( std::string data );

    std::string getWLAN();
    std::string setWLAN_list();

    std::string en_code ( std::string httpstring );
    std::string convertspecial ( std::string passworddata );
};

extern WiC64 wic64;

#endif // DEVICE_WIC64_H
