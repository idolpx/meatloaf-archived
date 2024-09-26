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

//
// https://github.com/MEGA65/open-roms/blob/master/doc/Protocol-DolphinDOS.md
// https://mega65.github.io/open-roms/doc/Protocol-DolphinDOS.html
// https://github.com/FeralChild64/open-roms/blob/master/src/kernal/iec_fast/dolphindos_detect.s
// http://sta.c64.org/cbmpar41c.html
// http://sta.c64.org/cbmpar71c.html
//


#ifndef PROTOCOL_DOLPHINDOS_H
#define PROTOCOL_DOLPHINDOS_H

#include "_protocol.h"
#include "../userport.h"

namespace Protocol
{
    class DolphinDOS: public IECProtocol
    {
		protected:
			uint8_t receiveByte(void) override;
			bool sendByte(uint8_t data, bool signalEOI) override;
    };
};

#endif // PROTOCOL_DOLPHINDOS_H
