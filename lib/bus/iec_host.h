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

#ifndef IECHOST_H
#define IECHOST_H

#include <Arduino.h>

#include "../../include/global_defines.h"
#include "../../include/cbmdefines.h"
#include "../../include/petscii.h"

#include "iec.h"
#include "protocol/cbmstandardserial.h"

using namespace Protocol;

class iecHost: public IEC
{
private:

public:
    bool deviceExists(uint8_t deviceID);
    void getStatus(uint8_t deviceID);
    void dumpSector(uint8_t sector);
    void dumpTrack(uint8_t track);


    // listen    perform a listen on the IEC bus
    // talk      perform a talk on the IEC bus
    // unlisten  perform an unlisten on the IEC bus
    // untalk    perform an untalk on the IEC bus
    // open      perform an open on the IEC bus
    // close     perform a close on the IEC bus
    // read      read raw data from the IEC bus
    // write     write raw data to the IEC bus
    // put       put specified data to the IEC bus
    // status    give the status of the specified drive
    // command   issue a command to the specified drive
    // pcommand  deprecated; use 'cbmctrl --petscii command' instead!
    // dir       output the directory of the disk in the specified drive
    // download  download memory contents from the floppy drive
    // upload    upload memory contents to the floppy drive
    // clk       Set the clk line on the IEC bus.
    // uclk      Unset the clk line on the IEC bus.
    // reset     reset all drives on the IEC bus
    // detect    detect drives on the IEC bus
    // change    wait for a disk to be changed in the specified drive

private:
    CBMStandardSerial protocol;

};

#endif // IECHOST_H