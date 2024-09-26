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
// https://github.com/MEGA65/open-roms/blob/master/doc/Protocol-SauceDOS.md
// http://www.nlq.de/
// http://www.baltissen.org/newhtm/sourcecodes.htm
// https://www.amigalove.com/viewtopic.php?t=1734
// https://ar.c64.org/rrwiki/images/4/48/The_Transactor_Vol09_03_1989_Feb_JD_review.pdf
// https://web.archive.org/web/20090826145226/http://home.arcor.de/jochen.adler/ajnjil-t.htm
// https://web.archive.org/web/20220423162959/https://sites.google.com/site/h2obsession/CBM/C128/JiffySoft128
//

#ifndef PROTOCOL_SAUCEDOS_H
#define PROTOCOL_SAUCEDOS_H

#include "_protocol.h"

namespace Protocol
{
	class SauceDOS : public IECProtocol
	{
		public:
			SauceDOS() {

			};

		protected:
			uint8_t receiveByte(void) override;
			bool sendByte(uint8_t data, bool eoi) override;
	};
};

#endif // PROTOCOL_SAUCEDOS_H
