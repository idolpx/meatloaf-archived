// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// Meatloaf is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Meatloaf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.

#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include <Arduino.h>

#define PRODUCT_ID "MEATLOAF CBM"
#define FW_VERSION "20200923.01" // Dynamically set at compile time in "platformio.ini"
#define USER_AGENT PRODUCT_ID " [" FW_VERSION "]"
//#define UPDATE_URL      "http://meatloaf.cc/fw/?p=meatloaf&d={{DEVICE_ID}}&a="
#define UPDATE_URL "http://meatloaf.cc/fw/meatloaf.4MB.bin"
//#define UPDATE_URL      "http://meatloaf.cc/fw/meatloaf.16MB.bin"
#define SYSTEM_DIR "/.sys/"

#define HOSTNAME "meatloaf"
#define SERVER_PORT 80   // HTTPd & WebDAV Server Port
#define LISTEN_PORT 6400 // Listen to this if not connected. Set to zero to disable.

//#define DEVICE_MASK 0b01111111111111111111111111110000 //  Devices 4-30 are enabled by default
#define DEVICE_MASK   0b00000000000000000000111100000000 //  Devices 8-11
//#define DEVICE_MASK   0b00000000000000000000111000000000 //  Devices 9-11
//#define IMAGE_TYPES   "D64|D71|D80|D81|D82|D8B|G64|X64|Z64|TAP|T64|TCRT|CRT|D1M|D2M|D4M|DHD|HDD|DNP|DFI|M2I|NIB"
//#define FILE_TYPES    "C64|PRG|P00|SEQ|S00|USR|U00|REL|R00"
//#define ARCHIVE_TYPES "7Z|GZ|ZIP|RAR"

// ESP8266 GPIO to C64 User Port
#define TX_PIN               TX  // TX   //64-B+C+7  //64-A+1+N+12=GND, 64-2=+5v, 64-L+6
#define RX_PIN               RX  // RX   //64-M+5
//#define SWITCH_PIN  D4  // IO2              // Long press to reset to 300KBPS Mode

#if defined(ESP8266)
    #define CTS_PIN          D1  // IO5 IN  //64-D      // CTS Clear to Send, connect to host's RTS pin
    #define RTS_PIN          D2  // IO4 OUT //64-K      // RTS Request to Send, connect to host's CTS pin
    #define DCD_PIN          D3  // IO0 OUT //64-H      // DCD Carrier Status, GPIO0 (programming mode pin)
#elif defined(ESP32)
    #define CTS_PIN          21  // IO21 DATAIN IN  //64-D      // CTS Clear to Send, connect to host's RTS pin
    #define RTS_PIN          22  // IO22 PROC   OUT //64-K      // RTS Request to Send, connect to host's CTS pin
    #define DCD_PIN          26  // IO26 INT    OUT //64-H      // DCD Carrier Status
#endif

#define RING_INTERVAL        3000  // How often to print RING when having a new incoming connection (ms)
#define MAX_CMD_LENGTH       256   // Maximum length for AT command
#define TX_BUF_SIZE          256   // Buffer where to read from serial before writing to TCP


// CLK & DATA lines in/out are split between two pins
//#define SPLIT_LINES

// CLK_OUT & DATA_OUT are inverted
#define INVERTED_LINES      false

#if defined(ESP8266)
    // ESP8266 GPIO to C64 IEC Serial Port
    #define IEC_PIN_ATN          D5    // IO14  INPUT/OUTPUT
    #define IEC_PIN_CLK          D6    // IO12  INPUT/OUTPUT
    #define IEC_PIN_DATA         D7    // IO13  INPUT/OUTPUT
    #define IEC_PIN_SRQ          D1    // IO5   INPUT/OUTPUT
    #define IEC_PIN_RESET        D2    // IO4   INPUT/OUTPUT
#elif defined(ESP32)
    // ESP32 GPIO to C64 IEC Serial Port
    #define IEC_PIN_ATN          22      // PROC
    #define IEC_PIN_CLK          27      // CKI
    #define IEC_PIN_DATA         21      // DI
    #define IEC_PIN_SRQ          26      // INT
    #define IEC_PIN_RESET        39      // IO15
#endif

// IEC protocol timing consts:
#define TIMING_BIT           70    // bit clock hi/lo time     (us)
#define TIMING_NO_EOI        5     // delay before bits        (us)
#define TIMING_EOI_WAIT      200   // delay to signal EOI      (us)
#define TIMING_EOI_THRESH    20    // threshold for EOI detect (*10 us approx)
#define TIMING_STABLE_WAIT   20    // line stabilization       (us)
#define TIMING_ATN_PREDELAY  50    // delay required in atn    (us)
#define TIMING_ATN_DELAY     100   // delay required after atn (us)
#define TIMING_FNF_DELAY     100   // delay after fnf?         (us)

// See timeoutWait
#define TIMEOUT 65500

#define	ATN_CMD_MAX_LENGTH 	40

#define LED_PIN LED_BUILTIN // IO2
#define LED_ON LOW
#define LED_OFF HIGH
#define LED_TIME 15 // #ms between toggle

static void ledToggle(bool now = false)
{
    static uint8_t ledTime = 0;

    if (millis() - ledTime > LED_TIME || now)
    {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        ledTime = millis();
    }
}

inline static void ledON()
{
    digitalWrite(LED_PIN, LED_ON);
}

inline static void ledOFF()
{
    digitalWrite(LED_PIN, LED_OFF);
}

// Enable this for verbose logging of IEC interface
// May cause ?LOAD ERROR
#define DEBUG

#ifndef DEBUG_PORT
#define DEBUG_PORT Serial
#endif
#if defined(ESP8266) || defined(CORE_MOCK)
#define pathToFileName(p) p
#endif //ESP8266
#ifdef DEBUG
#define Debug_print(...) DEBUG_PORT.print(__VA_ARGS__)
#define Debug_println(...) DEBUG_PORT.println(__VA_ARGS__)
#define Debug_printf(...) DEBUG_PORT.printf(__VA_ARGS__)
#define Debug_printv(format, ...) {DEBUG_PORT.printf("[%s:%u] %s(): " format "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__);}
#else
#define Debug_print(...)
#define Debug_println(...)
#define Debug_printf(...)
#define Debug_printv(...)
#endif

// Enable this for a timing test pattern on ATN, CLK, DATA, SRQ pins
//#define DEBUG_TIMING

// Enable this to show the data stream while loading
// Make sure device baud rate and monitor_speed = 921600
// May cause ?LOAD ERROR
#define DATA_STREAM

// Select the FileSystem in PLATFORMIO.INI file
//#define USE_SPIFFS
//#define USE_LITTLEFS
//#define USE_SDFS


// Format storage if a valid file system is not found
#define AUTO_FORMAT true
#define FORMAT_LITTLEFS_IF_FAILED true

#if defined USE_SPIFFS
#define FS_TYPE "SPIFFS"
#elif defined USE_LITTLEFS
#define FS_TYPE "LITTLEFS"
#elif defined USE_SDFS
#define FS_TYPE "SDFS"
#endif



#endif // GLOBAL_DEFINES_H
