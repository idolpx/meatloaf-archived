
#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

unsigned char h2int(char c);
void printProgress(uint16_t total, uint16_t current);

String urlencode(String str);
String urldecode(String str);
String ipToString ( IPAddress ip );
String formatBytes ( size_t bytes );

#endif