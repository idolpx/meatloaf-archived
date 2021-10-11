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

#ifndef DEVICE_DB_H
#define DEVICE_DB_H

#include "../../include/global_defines.h"
#include "meat_io.h"

#include <ArduinoJson.h>

#define RECORD_SIZE 512

class DeviceDB
{
public:
    DeviceDB(uint8_t device);
    ~DeviceDB();

    bool save();

    std::string config_file;

    uint8_t id();
    void id(uint8_t device);
    uint8_t media();
    void media(uint8_t media);
    uint8_t partition();
    void partition(uint8_t partition);
    std::string url();
    void url(std::string url);
    std::string path();
    void path(std::string path);
    std::string archive();
    void archive(std::string archive);
    std::string image();
    void image(std::string image);

    bool select(uint8_t device);

private:
    bool m_dirty;
    StaticJsonDocument<RECORD_SIZE> m_device;
};

#endif
