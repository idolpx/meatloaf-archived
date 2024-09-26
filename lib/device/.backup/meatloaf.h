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

// https://ilesj.wordpress.com/2014/05/14/1541-why-so-complicated/
// https://en.wikipedia.org/wiki/Fast_loader

#ifndef DEVICE_MEATLOAF
#define DEVICE_MEATLOAF

#include "drive.h"

class devMeatloaf: public iecDrive
{
public:
	devMeatloaf();
	virtual ~devMeatloaf() {};

 	virtual uint8_t command( void ) { return 0; };
	virtual uint8_t execute( void ) { return 0; };
	virtual uint8_t status(void) { return 0; };

protected:
	// handler helpers.
	virtual void handleListenCommand( void ) override;
	virtual void handleListenData(void) override;
	virtual void handleTalk(uint8_t chan) override;
	//virtual void handleOpen( void ) override;
	//virtual void handleClose( void ) override;

private:
	void reset(void);

	// Meatloaf Specific
	void sendMeatloafSystemInformation(void);
	void sendMeatloafVirtualDeviceStatus(void);
};

#endif // DEVICE_MEATLOAF
