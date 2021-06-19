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

#ifndef ML_CONFIG_H
#define ML_CONFIG_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESPWebDAV.h>
#include <WebDav4WebServer.h>

#if !WEBSERVER_HAS_HOOK
#error This sketch needs ESP8266WebServer::HookFunction and ESP8266WebServer::addHook
#endif


#include "../include/global_defines.h"
#include "meat_io.h"


static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

class MLHttpd
{
    private:

        static ESPWebDAVCore dav;
        static ESP8266WebServer server;
        //WiFiServer tcp(80);
        //ESPWebDAV dav;

        static bool fsOK;
        const String unsupportedFiles = String();

        static File uploadFile;


    public:
        MLHttpd(int port) {
                server = ESP8266WebServer( port );
        };
        ~MLHttpd();

        static void setup ( void );
        static void replyOK();
        static void replyOKWithMsg ( String msg );
        static void replyNotFound ( String msg );
        static void replyBadRequest ( String msg );
        static void replyServerError ( String msg );

#ifdef USE_SPIFFS
        String checkForUnsupportedPath ( String filename );
#endif

        static void handleStatus();
        static void handleFileList();
        static bool handleFileRead ( String path );
        static String lastExistingParent ( String path );
        static void handleFileCreate();
        static void deleteRecursive ( String path );
        static void handleFileDelete();
        static void handleFileUpload();
        static void handleNotFound();
        static void handleGetEdit();

};

#endif