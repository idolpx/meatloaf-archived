// Meatloaf - A Commodore 1541 disk drive emulator
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

#ifndef DEVICE_DB_H
#define DEVICE_DB_H

#include "global_defines.h"
#include <ArduinoJson.h>

#if defined(USE_SPIFFS)
#include <SPIFFS.h>
#elif defined(USE_LITTLEFS)
#include <LittleFS.h>
#endif

#define RECORD_SIZE 256

class DeviceDB
{
public:
    DeviceDB(FS *fileSystem);
    ~DeviceDB();

    bool init(String database);
    bool check();
    bool save();

    String database;

    byte device();
    void device(byte device);
    byte drive();
    void drive(byte drive);
    byte partition();
    void partition(byte partition);
    String url();
    void url(String url);
    String path();
    void path(String path);
    String image();
    void image(String image);

    bool select(byte device);

private:
    bool m_dirty;
    FS *m_fileSystem;
    StaticJsonDocument<256> m_device;
};

#endif
