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

#ifndef IECBUS_H
#define IECBUS_H

#include <Arduino.h>
//#include "global_defines.h"
#include "global_defines.h"
#include "cbmdefines.h"
#include "Petscii.h"


class iecBus
{
public:
	enum IECline
	{
		pulled = true,
		released = false
	};

	enum IECState 
	{
		noFlags   = 0,
		eoiFlag   = (1 << 0),   // might be set by iec_receive
		atnFlag   = (1 << 1),   // might be set by iec_receive
		errorFlag = (1 << 2)    // If this flag is set, something went wrong and
	};

	// Return values for checkATN:
	enum ATNCheck 
	{
		ATN_IDLE = 0,           // Nothing recieved of our concern
		ATN_CMD = 1,            // A command is recieved
		ATN_CMD_LISTEN = 2,     // A command is recieved and data is coming to us
		ATN_CMD_TALK = 3,       // A command is recieved and we must talk now
		ATN_ERROR = 4,          // A problem occoured, reset communication
		ATN_RESET = 5		    // The IEC bus is in a reset state (RESET line).
	};

	// IEC ATN commands:
	enum ATNCommand 
       {
		ATN_CODE_GLOBAL = 0x00,	// 0x00 + cmd (global command)
		ATN_CODE_LISTEN = 0x20,	// 0x20 + device_id (LISTEN)
		ATN_CODE_UNLISTEN = 0x3F,     // 0x3F (UNLISTEN)
		ATN_CODE_TALK = 0x40,	       // 0x40 + device_id (TALK)
		ATN_CODE_UNTALK = 0x5F,       // 0x5F (UNTALK)
		ATN_CODE_DATA = 0x60,	       // 0x60 + channel (SECOND)
		ATN_CODE_CLOSE = 0xE0,  	// 0xE0 + channel (CLOSE)
		ATN_CODE_OPEN = 0xF0		// 0xF0 + channel (OPEN)
	};

	// ATN command struct maximum command length:
	enum 
	{
		ATN_CMD_MAX_LENGTH = 40
	};
	
	typedef struct _tagATNCMD 
	{
		int code;
		int command;
		int channel;
		int device;
		int str[ATN_CMD_MAX_LENGTH];
		int strLen;
	} ATNCmd;

	iecBus();
	~iecBus(){}

	// Initialise iec driver
	bool init();

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is recieved and stored in atn_cmd.
	ATNCheck checkATN(ATNCmd& atn_cmd);

	// Checks if CBM is sending a reset (setting the RESET line high). This is typicall
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
//	bool checkRESET();

	// Sends a int. The communication must be in the correct state: a load command
	// must just have been recieved. If something is not OK, FALSE is returned.
	bool send(int data);

	// Same as IEC_send, but indicating that this is the last int.
	bool sendEOI(int data);

	// A special send command that informs file not found condition
	bool sendFNF();

	// Recieves a int
	int receive();

	// Enabled Device Bit Mask
	uint32_t enabledDevices;
	bool isDeviceEnabled(const int deviceNumber);
	void enableDevice(const int deviceNumber);
	void disableDevice(const int deviceNumber);

	IECState state() const;


private:
	// IEC Bus Commands
	ATNCheck listen(ATNCmd& atn_cmd);            // 0x20 + device_id 	Listen, device (0–30)
//	ATNCheck unlisten(ATNCmd& atn_cmd);          // 0x3F		Unlisten, all devices
	ATNCheck talk(ATNCmd& atn_cmd);              // 0x40 + device_id 	Talk, device 
//	ATNCheck untalk(ATNCmd& atn_cmd);            // 0x5F		Untalk, all devices 
//	ATNCheck reopen(ATNCmd& atn_cmd);            // 0x60 + channel	Reopen, channel (0–15)
//	ATNCheck close(ATNCmd& atn_cmd);             // 0xE0 + channel	Close, channel
//	ATNCheck open(ATNCmd& atn_cmd);              // 0xF0 + channel	Open, channel

	ATNCheck receiveCommand(ATNCmd& atn_cmd);
	
	int receiveByte(void);
	bool sendByte(int data, bool signalEOI);
	bool timeoutWait(int iecPIN, IECline lineStatus);
	bool turnAround(void);
	bool undoTurnAround(void);

	// true => PULL => DIGI_LOW
	inline void pull(int pin)
	{
		set_pin_mode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}

	// false => RELEASE => DIGI_HIGH
	inline void release(int pin)
	{
		// releasing line can set to input mode, which won't drive the bus - simple way to mimic open collector
		set_pin_mode(pin, INPUT);
	}

	inline IECline status(int pin)
	{
		#ifdef TWO_IO_PINS
		if (pin == IEC_PIN_CLK)
			pin = IEC_PIN_CLK_IN;
		else if (pin == IEC_PIN_DATA)
			pin = IEC_PIN_DATA_IN;
		#endif
		
		// To be able to read line we must be set to input, not driving.
		#ifndef TWO_IO_PINS
		set_pin_mode(pin, INPUT);
		#endif
		return digitalRead(pin) ? released : pulled;
	}

	inline int get_bit(uint8_t pin) {
#if defined(ESP8266)
		return GPIP(pin);
#elif defined(ESP32)
		return digitalRead(pin);
#endif
	}

	inline void set_bit(uint8_t pin, uint8_t val) {
#if defined(ESP8266)
		if(val) GPOS = (1 << pin);
		else GPOC = (1 << pin);
#elif defined(ESP32)
		digitalWrite(pin, val);
#endif
	}


	inline void set_pin_mode(uint8_t pin, uint8_t mode) {
#if defined(ESP8266)		
		if(mode == OUTPUT){
			GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
			GPC(pin) = (GPC(pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
			GPES = (1 << pin); //Enable
		} else if(mode == INPUT){
			GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
			GPEC = (1 << pin); //Disable
			GPC(pin) = (GPC(pin) & (0xF << GPCI)) | (1 << GPCD); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
		}
#elif defined(ESP32)
		pinMode( pin, mode );
#endif
	}

	// communication must be reset
	int m_state;
};

#endif
