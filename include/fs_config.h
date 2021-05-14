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
	extern FS* fileSystem = &LittleFS;
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