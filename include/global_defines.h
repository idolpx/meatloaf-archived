#ifndef GLOBAL_DEFINES_H
#define GLOBAL_DEFINES_H

#include <Arduino.h>

#define PRODUCT_ID "MEATLOAF CBM"
#define FW_VERSION "20200923.01" // Dynamically set at compile time in "platformio.ini"
//#define UPDATE_URL      "http://meatloaf.cc/fw/?p=meatloaf&d={{DEVICE_ID}}&a="
#define UPDATE_URL "http://meatloaf.cc/fw/meatloaf.4MB.bin"
//#define UPDATE_URL      "http://meatloaf.cc/fw/meatloaf.16MB.bin"
#define DEVICE_DB "/.sys/devices.db"
#define HOSTNAME "meatloaf"
#define SERVER_PORT 80   // WebDAV Server Port
#define LISTEN_PORT 6400 // Listen to this if not connected. Set to zero to disable.

// ESP8266 GPIO to C64 User Port
#define TX_PIN               TX  // TX   //64-B+C+7  //64-A+1+N+12=GND, 64-2=+5v, 64-L+6
#define RX_PIN               RX  // RX   //64-M+5
//#define SWITCH_PIN  D4  // IO2              // Long press to reset to 300KBPS Mode

#if defined(ESP8266)
    #define CTS_PIN          D1  // IO5  //64-D      // CTS Clear to Send, connect to host's RTS pin
    #define RTS_PIN          D2  // IO4  //64-K      // RTS Request to Send, connect to host's CTS pin
    #define DCD_PIN          D3  // IO0  //64-H      // DCD Carrier Status, GPIO0 (programming mode pin)
#elif defined(ESP32)
    #define CTS_PIN          21  // IO21 DATAIN  //64-D      // CTS Clear to Send, connect to host's RTS pin
    #define RTS_PIN          22  // IO22 PROC    //64-K      // RTS Request to Send, connect to host's CTS pin
    #define DCD_PIN          26  // IO26 INT     //64-H      // DCD Carrier Status
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
#define IEC_PIN_ATN          D5    // IO14
#define IEC_PIN_CLK          D6    // IO12
#define IEC_PIN_DATA         D7    // IO13
#define IEC_PIN_SRQ          D4    // IO2
//#define IEC_PIN_RESET       D8    // IO15
#elif defined(ESP32)
// ESP32 GPIO to C64 IEC Serial Port
#define IEC_PIN_ATN          22      // PROC
#define IEC_PIN_CLK          27      // CKI
#define IEC_PIN_DATA         32      // CKO
#define IEC_PIN_SRQ          26      // INT
//#define IEC_PIN_RESET       D8      // IO15
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

// Version 0.5 equivalent timings: 70, 5, 200, 20, 20, 50, 100, 100

// TIMING TESTING:
//
// The consts: 70,20,200,20,20,50,100,100 has been tested without debug print
// to work stable on my (Larsp)'s DTV at 700000 < F_CPU < 9000000
// using a 32 MB MMC card
//

#define	ATN_CMD_MAX_LENGTH 	40

// See timeoutWait
#define TIMEOUT 65500

#define LED_PIN LED_BUILTIN // IO2
#define LED_ON LOW
#define LED_OFF HIGH
#define LED_TIME 15 // #ms between toggle

//#define DEVICE_MASK     0b01111111111111111111111111110000  //  Devices 4-30 are enabled by default
#define DEVICE_MASK 0b00000000000000000000111100000000 //  Devices 8-11
#define IMAGE_TYPES "D64|D71|D80|D81|D82|D8B|G64|X64|Z64|TAP|T64|TCRT|CRT|D1M|D2M|D4M|DHD|HDD|DNP|DFI|M2I|NIB"
#define FILE_TYPES "C64|PRG|P00|SEQ|S00|USR|U00|REL|R00"


static void toggleLED(bool now = false)
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

// Enable this for verbose logging of IEC and CBM interfaces.
#define DEBUG // define in platformio.ini

#ifdef DEBUG
#define Debug_print(...) Serial.print(__VA_ARGS__)
#define Debug_println(...) Serial.println(__VA_ARGS__)
#define Debug_printf(...) Serial.printf(__VA_ARGS__)
#else
#define Debug_print(...)
#define Debug_println(...)
#define Debug_printf(...)
#endif

// Enable this for a timing test pattern on ATN, CLK, DATA, SRQ pins
//#define DEBUG_TIMING

// Enable this to show the data stream while loading
// Make sure device baud rate and monitor_speed = 921600
#define DATA_STREAM

// This should be defined if the RESET line is soldered in the IEC DIN connector. When defined it will give the
// arduino to go into a reset state and wait for the CBM to become ready for communication.
//#define HAS_RESET_LINE

// Define this to reset the commodore 64 when the esp2iec is reset
//#define RESET_C64

// Select the FileSystem in PLATFORMIO.INI file
//#define USE_SPIFFS
#define USE_LITTLEFS
//#define USE_SDFS

// Format storage if a valid file system is not found
#define AUTO_FORMAT true

#if defined USE_SPIFFS
#define FS_TYPE "SPIFFS"
#elif defined USE_LITTLEFS
#define FS_TYPE "LITTLEFS"
#elif defined USE_SDFS
#define FS_TYPE "SDFS"
#endif

#endif // GLOBAL_DEFINES_H
