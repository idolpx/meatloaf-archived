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

#ifndef IEC_H
#define IEC_H

#include <Arduino.h>

#include "../../include/global_defines.h"
#include "../../include/cbmdefines.h"
#include "../../include/petscii.h"
#include "string_utils.h"

#include "protocol/cbmstandardserial.h"
//#include "protocol/jiffydos.h"

#define EOI_RECVD       (1<<0)
#define COMMAND_RECVD   (1<<1)
#define JIFFY_ACTIVE    (1<<2)
#define JIFFY_LOAD      (1<<3)

#define	IEC_CMD_MAX_LENGTH 	100

using namespace Protocol;

class IEC
{
public:
	// Return values for service:
	enum BusState
	{
		BUS_IDLE = 0,		  // Nothing recieved of our concern
		BUS_COMMAND = 1,		  // A command is recieved
		BUS_LISTEN = 2,   // A command is recieved and data is coming to us
		BUS_TALK = 3,	  // A command is recieved and we must talk now
		BUS_ERROR = 5,		  // A problem occoured, reset communication
		BUS_RESET = 6		  // The IEC bus is in a reset state (RESET line).
	};

	// IEC ATN commands:
	enum ATNCommand
	{
		ATN_CODE_GLOBAL = 0x00,	      // 0x00 + cmd (global command)
		ATN_CODE_LISTEN = 0x20,	      // 0x20 + device_id (LISTEN)
		ATN_CODE_UNLISTEN = 0x3F,     // 0x3F (UNLISTEN)
		ATN_CODE_TALK = 0x40,	      // 0x40 + device_id (TALK)
		ATN_CODE_UNTALK = 0x5F,	      // 0x5F (UNTALK)
		ATN_CODE_SECONDARY = 0x60,	  // 0x60 + channel (SECONDARY)
		ATN_CODE_CLOSE = 0xE0,	      // 0xE0 + channel (CLOSE NAMED CHANNEL) (0-15)
		ATN_CODE_OPEN = 0xF0	      // 0xF0 + channel (OPEN NAMED CHANNEL) (0-15)
	};

	typedef struct _tagATNCMD
	{
		uint8_t code;
		uint8_t command;
		uint8_t channel;
		uint8_t device;
		char str[IEC_CMD_MAX_LENGTH];
	} ATNCmd;

	// IEC()
	// ~IEC() {}

	// Initialise iec driver
	bool init();

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is recieved and stored in atn_cmd.
	BusState service(ATNCmd &atn_cmd);

	// Checks if CBM is sending a reset (setting the RESET line high). This is typicall
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
//	bool checkRESET();

	// Sends a byte. The communication must be in the correct state: a load command
	// must just have been recieved. If something is not OK, FALSE is returned.
	bool send(byte data);
	bool send(std::string data);

	// Same as IEC_send, but indicating that this is the last byte.
	bool sendEOI(byte data);

	// A special send command that informs file not found condition
	bool sendFNF();

	// Recieves a byte
	int16_t receive();

	// Enabled Device Bit Mask
	uint32_t enabledDevices;
	bool isDeviceEnabled(const uint8_t deviceNumber);
	void enableDevice(const uint8_t deviceNumber);
	void disableDevice(const uint8_t deviceNumber);

	void debugTiming();

	IECState state();	

private:
	// IEC Bus Commands
	BusState deviceListen(ATNCmd &atn_cmd);	  // 0x20 + device_id   Listen, device (0–30)
	void deviceUnListen(void);                // 0x3F               Unlisten, all devices
	BusState deviceTalk(ATNCmd &atn_cmd);	  // 0x40 + device_id 	Talk, device
	void deviceUnTalk(void);                  // 0x5F               Untalk, all devices
	//BusState deviceReopen(ATNCmd &atn_cmd);  // 0x60 + channel     Reopen, channel (0–15)
	BusState deviceClose(ATNCmd &atn_cmd);     // 0xE0 + channel     Close, channel
	//BusState deviceOpen(ATNCmd &atn_cmd);    // 0xF0 + channel     Open, channel

	bool turnAround(void);
	bool undoTurnAround(void);	

protected:
	CBMStandardSerial protocol;
};

#endif
