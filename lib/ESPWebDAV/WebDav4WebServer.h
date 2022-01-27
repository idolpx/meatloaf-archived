
#ifndef __WEBDAV4WEBSERVER
#define __WEBDAV4WEBSERVER

#include <ESPWebDAV.h>
#if defined(ARDUINO_ARCH_ESP8266) || defined(CORE_MOCK)
#include <ESP8266WebServer.h>
using WebServer = ESP8266WebServer;
#endif //ARDUINO_ARCH_ESP8266
#if defined(ARDUINO_ARCH_ESP32)
#include <WebServer.h>
#endif //ARDUINO_ARCH_ESP32

#if WEBSERVER_HAS_HOOK

// fsRootDIR is the FS destination path where the "DAV root dir" files are stored
// when not specified / by default, fsRootDir is the same as davRootDir.
// (see example)

WebServer::HookFunction hookWebDAVForWebserver(const String& davRootDir, ESPWebDAVCore& dav, const String& fsRootDir = emptyString);

#endif // WEBSERVER_HAS_HOOK

#endif // __WEBDAV4WEBSERVER
