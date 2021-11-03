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

#ifndef DEVICE_DRIVE_H
#define DEVICE_DRIVE_H


#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#if defined(USE_SPIFFS)
#include <FS.h>
#if defined(ESP32)
#include <SPIFFS.h>
#endif
#elif defined(USE_LITTLEFS)
#if defined(ESP8266)
#include <LittleFS.h>
#elif defined(ESP32)
#include <LITTLEFS.h>
#endif
#endif

#include "../../include/global_defines.h"
#include "../../include/cbmdefines.h"
#include "../../include/petscii.h"

#include "iec.h"
#include "iec_device.h"
#include "device_db.h"
#include "meat_io.h"
#include "MemoryInfo.h"
#include "helpers.h"
#include "utils.h"
#include "string_utils.h"

//#include "doscmd.h"

enum OpenState
{
	O_NOTHING,		// Nothing to send / File not found error
	O_STATUS,		// Command channel status is requested
	O_FILE,			// A program file is opened
	O_DIR,			// A listing is requested
	O_ML_INFO,		// Meatloaf Device Info
	O_ML_STATUS		// Meatloaf Virtual Device Status
};

class deviceDrive: public iecDevice
{
public:
	// Return values for service:
	enum DeviceState
	{
		DEVICE_IDLE = 0,       // Ready and waiting
		DEVICE_OPEN,           // Command received and channel opened
		DEVICE_DATA,           // Data sent or received
	};

	struct Channel
	{
		std::string name;
		uint32_t cursor;
		bool writing;
	};
	std::unordered_map<uint8_t, Channel> channels;

	deviceDrive(IEC &iec);
	virtual ~deviceDrive() {};

	bool begin();

	// The handler returns the current IEC state, see the iec.h for possible states.
	uint8_t service(void);
    virtual uint8_t process_command(void) override;

private:
	void reset(void);

	// Meatloaf Specific
	void sendMeatloafSystemInformation(void);
	void sendMeatloafVirtualDeviceStatus(void);

	// Directory Navigation & Listing
	bool m_show_extension = true;
	bool m_show_hidden = false;
	bool m_show_date = false;
	bool m_show_load_address = false;
	void changeDir(std::string url);
	uint16_t sendHeader(uint16_t &basicPtr, std::string header, std::string id);
	//uint16_t sendHeader(uint16_t &basicPtr, const char *format, ...);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, char *text);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, const char *format, ...);
	uint16_t sendFooter(uint16_t &basicPtr, uint16_t blocks_free, uint16_t block_size);
	void sendListing();

	// File LOAD / SAVE
	void prepareFileStream(std::string url);
	MFile* getPointed(MFile* urlFile);
	void sendFile();
	void saveFile();

	// Device Status
	std::string m_device_status = "";
	void sendStatus(void);
	void sendFileNotFound(void);
	void setDeviceStatus(int number, int track=0, int sector=0);

	CommandPathTuple parseLine(std::string commandLne, size_t channel);

	// This is set after an open command and determines what to send next
	byte m_openState;
	
	std::unique_ptr<MFile> m_mfile; // Always points to current directory
	std::string m_filename; // Always points to current or last loaded file
};


#endif
