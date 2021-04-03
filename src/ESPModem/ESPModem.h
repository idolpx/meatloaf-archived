/*
   WiFi SIXFOUR - A virtual WiFi modem based on the ESP 8266 chipset
   Copyright (C) 2016 Paul Rickards <rickards@gmail.com>
   Added EEPROM read/write, status/help pages, busy answering of incoming calls
   uses the readily available Sparkfun ESP8266 WiFi Shield which has 5v level
   shifters and 3.3v voltage regulation present-- easily connect to a C64
   https://www.sparkfun.com/products/13287

   based on
   ESP8266 based virtual modem
   Copyright (C) 2016 Jussi Salin <salinjus@gmail.com>

   https://github.com/jsalin/esp8266_modem

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ESPMODEM_H
#define ESPMODEM_H

#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#endif

#include <WiFiClient.h>
#include <WiFiServer.h>
#include <EEPROM.h>

#include "global_defines.h"

//***************************************************************************
// Global data section.                                                     *
//***************************************************************************
//float version               = 1.43;
//const char *appid           = HOSTNAME;


#define VERSIONA 0
#define VERSIONB 1
#define VERSION_ADDRESS 0    // EEPROM address
#define VERSION_LEN     2    // Length in bytes
#define SSID_ADDRESS    2
#define SSID_LEN        32
#define PASS_ADDRESS    34
#define PASS_LEN        63
#define IP_TYPE_ADDRESS 97   // for future use
#define STATIC_IP_ADDRESS 98 // length 4, for future use
#define STATIC_GW       102  // length 4, for future use
#define STATIC_DNS      106  // length 4, for future use
#define STATIC_MASK     110  // length 4, for future use
#define BAUD_ADDRESS    111
#define ECHO_ADDRESS    112
#define SERVEResult_PORT_ADDRESS 113 // 2 bytes
#define AUTO_ANSWEResult_ADDRESS 115 // 1 byte
#define TELNET_ADDRESS  116     // 1 byte
#define VERBOSE_ADDRESS 117
#define PET_TRANSLATE_ADDRESS 118
#define FLOW_CONTROL_ADDRESS 119
#define PIN_POLARITY_ADDRESS 120
#define DIAL0_ADDRESS   200
#define DIAL1_ADDRESS   250
#define DIAL2_ADDRESS   300
#define DIAL3_ADDRESS   350
#define DIAL4_ADDRESS   400
#define DIAL5_ADDRESS   450
#define DIAL6_ADDRESS   500
#define DIAL7_ADDRESS   550
#define DIAL8_ADDRESS   600
#define DIAL9_ADDRESS   650
#define BUSY_MSG_ADDRESS 700
#define BUSY_MSG_LEN    80
#define LAST_ADDRESS    780

// Telnet codes
#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe

void updateProgress(int cur, int total);
void updateEnd();    
bool updateFirmware();

class ESPModem {
public:
    const int bauds[12] = { 300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
    const int speedDialAddresses[10] = { DIAL0_ADDRESS, DIAL1_ADDRESS, DIAL2_ADDRESS, DIAL3_ADDRESS, DIAL4_ADDRESS, DIAL5_ADDRESS, DIAL6_ADDRESS, DIAL7_ADDRESS, DIAL8_ADDRESS, DIAL9_ADDRESS };
    String resultCodes[9] = { "OK", "CONNECT", "RING", "NO CARRIER", "ERROR", "", "NO DIALTONE", "BUSY", "NO ANSWER" };

    enum resultCodes_t { 
        Result_OK, 
        Result_CONNECT, 
        Result_RING, 
        Result_NOCARRIER, 
        Result_ERROR, 
        Result_NONE, 
        Result_NODIALTONE, 
        Result_BUSY, 
        Result_NOANSWER 
    };

    enum flowControl_t { 
        F_NONE, 
        F_HARDWARE, 
        F_SOFTWARE
    };

    enum pinPolarity_t {  // Is LOW (0) or HIGH (1) active?
        P_INVERTED, 
        P_NORMAL 
    };


    // Global variables
    String build = "20160621182048";
    String cmd = "";                    // Gather a new AT command to this string from serial

    bool cmdMode = true;                // Are we in AT command mode or connected mode
    bool callConnected = false;         // Are we currently in a call
    bool telnet = false;                // Is telnet control code handling enabled
    bool verboseResults = false;
    unsigned long lastRingMs = 0;       // Time of last "RING" message (millis())
    //long myBps;                       // What is the current BPS setting

    char plusCount = 0;                 // Go to AT mode at "+++" sequence, that has to be counted
    unsigned long plusTime = 0;         // When did we last receive a "+++" sequence
    unsigned long ledTime = 0;
    byte ringCount = 0;
    unsigned long connectTime = 0;    
    bool petTranslate = false;          // Fix PET MCTerm 1.26C Pet->ASCII encoding to actual ASCII
    bool hex = false;
    byte flowControl = F_NONE;          // Use flow control
    bool txPaused = false;              // Has flow control asked us to pause?
    byte pinPolarity = P_INVERTED;
    bool echo = true;
    bool autoAnswer = false;

    // (that direction is very blocking by the ESP TCP stack,
    // so we can't do one byte a time.)
    uint8_t txBuf[TX_BUF_SIZE];
    String speedDials[10];
    byte serialspeed;

    String ssid, password, busyMsg;


    String formatBytes (size_t bytes);
    String connectTimeString();

    void writeSettings();
    void readSettings();
    void defaultEEPROM();

    String getEEPROM(int startAddress, int len);
    void setEEPROM(String inString, unsigned int startAddress, unsigned int maxLen);
    void sendResult(int resultCode);
    void sendString(String msg);
 //   int checkButton();
    void connectWiFi();
    void updateLed();
    void disconnectWiFi();
    void setBaudRate(int inSpeed);
    void setCarrier(byte carrier);
    void displayNetworkStatus();
    void displayCurrentSettings();
    void displayStoredSettings();
    void waitForSpace();
    void displayHelp();
    void storeSpeedDial(byte num, String location);
#if defined(ESP8266)
    bool startWPSConnect();
#endif

    String ipToString(IPAddress ip);
    void hangUp();

    /*
    void handleWebHangUp();
    void handleRoot();
    */

    void answerCall();
    void handleIncomingConnection();
    void dialOut(String upCmd);

    /**
     Perform a command given in command mode
    */
    void command();

    // RTS/CTS protocol is a method of handshaking which uses one wire in each direction to allow each
    // device to indicate to the other whether or not it is ready to receive data at any given moment.
    // One device sends on RTS and listens on CTS; the other does the reverse. A device should drive
    // its handshake-output wire low when it is ready to receive data, and high when it is not. A device
    // that wishes to send data should not start sending any bytes while the handshake-input wire is low;
    // if it sees the handshake wire go high, it should finish transmitting the current byte and then wait
    // for the handshake wire to go low before transmitting any more.
    // http://electronics.stackexchange.com/questions/38022/what-is-rts-and-cts-flow-control
    void handleFlowControl();

    /**
     Arduino main loop function
    */
    void setup();
    void loop();
};

#endif