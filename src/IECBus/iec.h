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

#ifndef IECBUS_H
#define IECBUS_H

#include <Arduino.h>
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
		ATN_CODE_GLOBAL = 0x00,     // 0x00 + cmd (global command)
		ATN_CODE_LISTEN = 0x20,     // 0x20 + device_id (LISTEN)
		ATN_CODE_UNLISTEN = 0x3F,   // 0x3F (UNLISTEN)
		ATN_CODE_TALK = 0x40,	    // 0x40 + device_id (TALK)
		ATN_CODE_UNTALK = 0x5F,     // 0x5F (UNTALK)
		ATN_CODE_DATA = 0x60,	    // 0x60 + channel (SECOND)
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
		uint8_t code;
		uint8_t command;
		uint8_t channel;
		uint8_t device;
		char str[ATN_CMD_MAX_LENGTH];
		uint8_t strLen;
	} ATNCmd;

	iecBus();
	~iecBus(){}

	// Initialise iec driver
	bool init();

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is recieved and stored in atn_cmd.
	ATNCheck checkATN(ATNCmd& atn_cmd);

	// Checks if CBM is sending a reset (setting the RESET line high). This is typically
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
//	bool checkRESET();

	// Sends a byte. The communication must be in the correct state: a load command
	// must just have been recieved. If something is not OK, FALSE is returned.
	bool send(uint8_t data);

	// Same as IEC_send, but indicating that this is the last byte.
	bool sendEOI(uint8_t data);

	// A special send command that informs file not found condition
	bool sendFNF(void);

	// Recieves a byte
	uint8_t receive(void);

	// Enabled Device Bit Mask
	uint32_t enabledDevices;
	bool isDeviceEnabled(const uint8_t deviceNumber);
	void enableDevice(const uint8_t deviceNumber);
	void disableDevice(const uint8_t deviceNumber);

	IECState state() const;


private:
	// IEC Bus Commands
	ATNCheck listen(ATNCmd& atn_cmd);            // 0x20 + device_id 	Listen, device (0–30)
//	ATNCheck unlisten(ATNCmd& atn_cmd);          // 0x3F				Unlisten, all devices
	ATNCheck talk(ATNCmd& atn_cmd);              // 0x40 + device_id 	Talk, device 
//	ATNCheck untalk(ATNCmd& atn_cmd);            // 0x5F				Untalk, all devices 
//	ATNCheck reopen(ATNCmd& atn_cmd);            // 0x60 + channel		Reopen, channel (0–15)
//	ATNCheck close(ATNCmd& atn_cmd);             // 0xE0 + channel		Close, channel
//	ATNCheck open(ATNCmd& atn_cmd);              // 0xF0 + channel		Open, channel

	ATNCheck receiveCommand(ATNCmd& atn_cmd);
	
	uint8_t receiveByte(void);
	bool sendByte(uint8_t data, bool signalEOI);
	bool timeoutWait(uint8_t iecPIN, IECline lineStatus);
	bool turnAround(void);
	bool undoTurnAround(void);

	// true => PULL => DIGI_LOW
	inline void pull(uint8_t pin)
	{
		set_pin_mode(pin, OUTPUT);
		espDigitalWrite(pin, LOW);
	}

	// false => RELEASE => DIGI_HIGH
	inline void release(uint8_t pin)
	{
		set_pin_mode(pin, OUTPUT);
		espDigitalWrite(pin, HIGH);
	}

	inline IECline status(uint8_t pin)
	{
		#ifndef SPLIT_LINES
			// To be able to read line we must be set to input, not driving.
			set_pin_mode(pin, INPUT);
		#endif

		return espDigitalRead(pin) ? released : pulled;
	}

	inline uint8_t get_bit(uint8_t pin)
       {
		return espDigitalRead(pin);
	}

	inline void set_bit(uint8_t pin, uint8_t bit)
	{
		return espDigitalWrite(pin, bit);
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

	inline void ICACHE_RAM_ATTR espDigitalWrite(uint8_t pin, uint8_t val) {
#if defined(ESP8266)
		if(val) GPOS = (1 << pin);
		else GPOC = (1 << pin);
#elif defined(ESP32)
		digitalWrite(pin, val);
#endif
	}

	inline uint8_t ICACHE_RAM_ATTR espDigitalRead(uint8_t pin) {
		uint8_t val = -1;
#if defined(ESP8266)
		val = GPIP(pin);
#elif defined(ESP32)
		val = digitalRead(pin);
#endif
		return val;
	}

	// communication must be reset
	uint8_t m_state;
};

#endif
