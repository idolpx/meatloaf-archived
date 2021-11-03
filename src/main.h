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

#include "../include/global_defines.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#if defined(MDNS)
#include <ESP8266mDNS.h>
#endif
#if defined(WWW_SERVER)
#include <ESP8266WebServer.h>
#define WebServer ESP8266WebServer
#endif
#elif defined(ESP32)
#include <WiFi.h>
#if defined(MDNS)
#include <ESPmDNS.h>
#endif
#if defined(WWW_SERVER)
#include <WebServer.h>
#endif
#endif

// Setup FileSystem Object
#if defined(USE_SPIFFS)
#include <FS.h>
//#define FS_TYPE "SPIFFS"
#if defined(ESP8266)
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined(ESP32)
#include <SPIFFS.h>
#endif
FS *fileSystem = &SPIFFS;
#elif defined USE_LITTLEFS
//#define FS_TYPE "LittleFS"
#if defined(ESP8266)
#include <LittleFS.h>
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
#elif defined(ESP32)
#include <LITTLEFS.h>
FS *fileSystem = &LITTLEFS;
#endif
#elif defined USE_SDFS
#include <SDFS.h>
//#define FS_TYPE "SDFS"
FS *fileSystem = &SDFS;
SDFSConfig fileSystemConfig = SDFSConfig();
// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif


#include "meat_io.h"

#include "iec.h"
#include "iec_device.h"
#include "drive.h"
#include "ESPModem.h"
#include "ml_tests.h"

enum class statemachine
{
    idle,   // BUS is idle
    select, // ATN is PULLED read command
    data    // READY to receive or send data
};
statemachine bus_state = statemachine::idle;
void IRAM_ATTR onAttention();

String statusMessage;
bool initFailed = false;

static IEC iec;
static deviceDrive drive ( iec );


//Zimodem modem;
ESPModem modem;

// #if defined(ESP8266)
// ADC_MODE ( ADC_VCC ); // Set ADC for Voltage Monitoring
// #endif


//
// Web Server & WebDAV
//
#if defined(WEB_SERVER)
    #include "WebDav4WebServer.h"

    #if !WEBSERVER_HAS_HOOK
    #error This sketch needs ESP8266WebServer::HookFunction and ESP8266WebServer::addHook
    #endif

    WebServer www ( SERVER_PORT );
    ESPWebDAVCore dav;

    bool fsOK;
    String unsupportedFiles;
    File uploadFile;

    static const char TEXT_PLAIN[] PROGMEM = "text/plain";
    static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
    static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

    void replyOK();
    void replyOKWithMsg ( String msg );
    void replyNotFound ( String msg );
    void replyBadRequest ( String msg );
    void replyServerError ( String msg );

    void handleStatus();
    void handleFileList();
    bool handleFileRead ( String path );
    String lastExistingParent ( String path );
    void handleFileCreate();
    void deleteRecursive ( String path );
    void handleFileDelete();
    void handleFileUpload();
    void handleNotFound();
    void handleGetEdit();
    void setupWWW ( void );
    void notFound ();
#else if defined(WEBDAV)
    #include "ESPWebDAV.h"

    WiFiServer tcp ( SERVER_PORT );
    ESPWebDAV dav;
#endif

#ifdef USE_SPIFFS
String checkForUnsupportedPath ( String filename );
#endif


// Main Functions
void setup ( void );
void loop ( void );
