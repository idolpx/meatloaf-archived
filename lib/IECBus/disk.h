// Meatloaf - A Commodore 64/128 multi-device emulator
// https://github.com/idolpx/meatloaf
// Copyright(C) 2020 James Johnston
//
// This file is part of Meatloaf but adapted for use in the FujiNet project
// https://github.com/FujiNetWIFI/fujinet-platformio
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

#ifndef IECDISK_H
#define IECDISK_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <string>

#include "../../include/global_defines.h"
#include "../../include/fs_config.h"
#include "../../include/cbmdefines.h"

#include "iec.h"
#include "utils.h"
#include "helpers.h"
#include "media.h"

class iecDisk : public iecDevice
{
private:
    DiskType *_disk = nullptr;

    void _read();
    void _write(bool verify);
    void _format();

    void _status();
    void _process_command();

    void shutdown() {};

	// handler helpers.
	void _open(void);
	void _listen_data(void);
	void _talk_data(uint8_t chan);
	void _close(void);

	void sendStatus(void);
	void sendSystemInfo(void);
	void sendDeviceStatus(void);

	void sendListing(void);
	void sendListingHTTP(void);
	uint16_t sendHeader(uint16_t &basicPtr);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, char* text);
	uint16_t sendLine(uint16_t &basicPtr, uint16_t blocks, const char* format, ...);
	uint16_t sendFooter(uint16_t &basicPtr);
	void sendFile(void);
	void sendFileHTTP(void);

	void saveFile(void);

    FS* m_fileSystem;
	StaticJsonDocument<256> m_jsonHTTP;
	String m_lineBuffer;

	std::string _command;
	int _drive;
	int _partition;
    std::string _url;
    std::string _path;
	std::string _archive; // Future streaming of images/files in archives .zip/.7z/.rar
    std::string _image;
	std::string _filename;
	std::string _extension;
	std::string _type; // CBM file type PRG, SEQ, REL
	std::string _mode; // Read / Write

public:
    iecDisk();
    disktype_t mount(FILE *f, const char *filename, uint32_t disksize, disktype_t disk_type = DISKTYPE_UNKNOWN);
    void unmount();
    bool write_blank(FILE *f, uint16_t sectorSize, uint16_t numSectors);

    disktype_t disktype() { return _disk == nullptr ? DISKTYPE_UNKNOWN : _disk->_disktype; };

	~iecDisk();
};

#endif // IECDISK_H
