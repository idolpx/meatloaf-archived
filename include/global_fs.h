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

#ifndef GLOBAL_FS_H
#define GLOBAL_FS_H

// Setup FileSystem Object
#if defined(USE_SPIFFS)
#if defined(ESP32)
#include <SPIFFS.h>
#endif
#include <FS.h>
const char* fsName = "SPIFFS";
FS *fileSystem = &SPIFFS;

#if defined(ESP8266)
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#endif
#elif defined USE_LITTLEFS
#if defined(ESP8266)
#include <LittleFS.h>
const char* fsName = "LittleFS";
FS *fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();
#endif
#if defined(ESP32)
#include <LITTLEFS.h>
FS *fileSystem = &LITTLEFS;
#endif
#elif defined USE_SDFS
#include <SDFS.h>
const char* fsName = "SDFS";
FS *fileSystem = &SDFS;
SDFSConfig fileSystemConfig = SDFSConfig();
// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif

#endif