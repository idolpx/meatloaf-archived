#ifndef INTERFACE_H
#define INTERFACE_H

#if defined(ESP8266)

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#if defined(USE_SPIFFS)
#include <SPIFFS.h>
#elif defined(USE_LITTLEFS)
#include <LittleFS.h>
#endif

#include "global_defines.h"
#include "MemoryInfo.h"
#include "iec.h"
#include "cbmdefines.h"
#include "Petscii.h"
#include "device_db.h"
#include "helpers.h"

// enum  fsType {
// 	IS_FAIL = 0xFF, 	// IFail: SD card or fat not ok
// 	IS_NATIVE = 0,		// Regular file system file state
// 	IS_D64 = 1,
// 	IS_T64 = 2,
// 	IS_M2I = 3,
// 	IS_PRG = 4,
// };

enum OpenState
{
	O_NOTHING,		// Nothing to send / File not found error
	O_INFO,			// User issued a reload sd card
	O_FILE,			// A program file is opened
	O_DIR,			// A listing is requested
	O_FILE_ERR,		// Incorrect file format opened
	O_SAVE_REPLACE, // Save-with-replace is requested
	O_DEVICE_INFO,
	O_DEVICE_STATUS
};

// The base pointer of basic.
#define C64_BASIC_START 0x0801

class Interface
{
public:
	Interface(IEC &iec, FS *fileSystem);
	virtual ~Interface() {}

	bool begin();

	// The handler returns the current IEC state, see the iec.hpp for possible states.
	byte loop(void);

	// Keeping the system date and time as set on a specific moment. The millis() will then keep the elapsed time since
	// moment the time was set.
	void setDateTime(word year, byte month, byte day, byte hour, byte minute, byte second);

	// retrieve the date and time as strings. Current time will be updated according to the elapsed millis before formatting.
	// String will be of format "yyyymmdd hhmmss", if timeOnly is true only the time part will be returned as
	// "hhmmss", this fits the TIME$ variable of cbm basic 2.0 and later.
	char *dateTimeString(char *dest, bool timeOnly);

private:
	void reset(void);

	void sendStatus(void);
	void sendDeviceInfo(void);
	void sendDeviceStatus(void);

	void sendListing(void);
	void sendListingHTTP(void);
	uint16_t sendHeader(uint16_t &basicPtr);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, char *text);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, const char *format, ...);
	uint16_t sendFooter(uint16_t &basicPtr);
	void sendFile(void);
	void sendFileHTTP(void);

	void saveFile(void);

	// handler helpers.
	void handleATNCmdCodeOpen(IEC::ATNCmd &cmd);
	void handleATNCmdCodeDataListen(void);
	void handleATNCmdCodeDataTalk(byte chan);
	void handleATNCmdClose(void);

	void handleDeviceCommand(IEC::ATNCmd &cmd);
	void handleMeatLoafCommand(IEC::ATNCmd &cmd);

	// our iec low level driver:
	IEC &m_iec;

	// This var is set after an open command and determines what to send next
	byte m_openState; // see OpenState
	byte m_queuedError;

	// atn command buffer struct
	IEC::ATNCmd &m_atn_cmd;

	FS *m_fileSystem;
	StaticJsonDocument<256> m_jsonHTTP;
	String m_lineBuffer;
	//DynamicJsonDocument m_jsonHTTPBuffer;

	DeviceDB m_device;
	String m_filename;
	String m_filetype;
};

#elif defined(ESP32)

#include <WiFi.h>
#include <HTTPClient.h>

#if defined(USE_SPIFFS)
#include <SPIFFS.h>
#elif defined(USE_LITTLEFS)
#include <LittleFS.h>
#endif

#include "global_defines.h"
#include "MemoryInfo.h"
#include "iec.h"
#include "cbmdefines.h"
#include "Petscii.h"
#include "device_db.h"
#include "helpers.h"

// enum  fsType {
// 	IS_FAIL = 0xFF, 	// IFail: SD card or fat not ok
// 	IS_NATIVE = 0,		// Regular file system file state
// 	IS_D64 = 1,
// 	IS_T64 = 2,
// 	IS_M2I = 3,
// 	IS_PRG = 4,
// };

enum OpenState
{
	O_NOTHING,		// Nothing to send / File not found error
	O_INFO,			// User issued a reload sd card
	O_FILE,			// A program file is opened
	O_DIR,			// A listing is requested
	O_FILE_ERR,		// Incorrect file format opened
	O_SAVE_REPLACE, // Save-with-replace is requested
	O_DEVICE_INFO,
	O_DEVICE_STATUS
};

// The base pointer of basic.
#define C64_BASIC_START 0x0801

class Interface
{
public:
	Interface(IEC &iec, FS *fileSystem);
	virtual ~Interface() {}

	bool begin();

	// The handler returns the current IEC state, see the iec.hpp for possible states.
	byte loop(void);

	// Keeping the system date and time as set on a specific moment. The millis() will then keep the elapsed time since
	// moment the time was set.
	void setDateTime(word year, byte month, byte day, byte hour, byte minute, byte second);

	// retrieve the date and time as strings. Current time will be updated according to the elapsed millis before formatting.
	// String will be of format "yyyymmdd hhmmss", if timeOnly is true only the time part will be returned as
	// "hhmmss", this fits the TIME$ variable of cbm basic 2.0 and later.
	char *dateTimeString(char *dest, bool timeOnly);

private:
	void reset(void);

	void sendStatus(void);
	void sendDeviceInfo(void);
	void sendDeviceStatus(void);

	void sendListing(void);
	void sendListingHTTP(void);
	uint16_t sendHeader(uint16_t &basicPtr);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, char *text);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, const char *format, ...);
	uint16_t sendFooter(uint16_t &basicPtr);
	void sendFile(void);
	void sendFileHTTP(void);

	void saveFile(void);

	// handler helpers.
	void handleATNCmdCodeOpen(IEC::ATNCmd &cmd);
	void handleATNCmdCodeDataListen(void);
	void handleATNCmdCodeDataTalk(byte chan);
	void handleATNCmdClose(void);

	void handleDeviceCommand(IEC::ATNCmd &cmd);
	void handleMeatLoafCommand(IEC::ATNCmd &cmd);

	// our iec low level driver:
	IEC &m_iec;

	// This var is set after an open command and determines what to send next
	byte m_openState; // see OpenState
	byte m_queuedError;

	// atn command buffer struct
	IEC::ATNCmd &m_atn_cmd;

	FS *m_fileSystem;
	StaticJsonDocument<256> m_jsonHTTP;
	String m_lineBuffer;
	//DynamicJsonDocument m_jsonHTTPBuffer;

	DeviceDB m_device;
	String m_filename;
	String m_filetype;
};

#endif
#endif
