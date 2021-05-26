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

#ifndef CONFIG_FS_H
#define CONFIG_FS_H

#define FORMAT_LITTLEFS_IF_FAILED true

#if defined(USE_SPIFFS)
#if defined(ESP32)
	#include <SPIFFS.h>
#endif
	#include <FS.h>
	FS* fileSystem = &SPIFFS;
	
#if defined(ESP8266)
	SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#endif
#elif defined USE_LITTLEFS
#if defined(ESP8266)
	#include <LittleFS.h>
	FS* fileSystem = &LittleFS;
	LittleFSConfig fileSystemConfig = LittleFSConfig();	
#endif
#if defined(ESP32)
	#include <LITTLEFS.h>
	FS* fileSystem = &LITTLEFS;
#endif
#elif defined USE_SDFS
	#include <SDFS.h>
	#define CHIP_SELECT_PIN	15
	#define SPI_SETTINGS SPI_FULL_SPEED
	FS* fileSystem = &SDFS;
	SDFSConfig fileSystemConfig = SDFSConfig();	
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif


//StaticJsonDocument<9216> jsonConfigBuffer;
//JsonObject configJSON = jsonConfigBuffer.as<JsonObject>();

//StaticJsonDocument<1024> jsonHTTPBuffer;
//JsonObject httpJSON = jsonHTTPBuffer.as<JsonObject>();

#endif // CONFIG_FS_H