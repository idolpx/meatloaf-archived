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

#ifndef PROTOCOL_CBMSTANDARDSERIAL_H
#define PROTOCOL_CBMSTANDARDSERIAL_H

#include "../../../include/global_defines.h"

#define TIMED_OUT 0

namespace Protocol
{
	enum IECline
	{
		pulled = true,
		released = false
	};

	enum IECState
	{
		noFlags = 0,
		eoiFlag = (1 << 0),	 // might be set by iec_receive
		atnFlag = (1 << 1),	 // might be set by iec_receive
		errorFlag = (1 << 2) // If this flag is set, something went wrong and
	};

	class CBMStandardSerial
	{
	public:
		// communication must be reset
		uint8_t m_state = noFlags;

		virtual int16_t receiveByte(void);
		virtual bool sendByte(uint8_t data, bool signalEOI);
		virtual size_t timeoutWait(uint8_t iecPIN, IECline lineStatus, size_t wait = TIMEOUT, size_t step = 3);


		// true => PULL => DIGI_LOW
		inline void pull(uint8_t pinNumber)
		{
			espPinMode(pinNumber, OUTPUT);
			espDigitalWrite(pinNumber, LOW);
		}

		// false => RELEASE => DIGI_HIGH
		inline void release(uint8_t pinNumber)
		{
			espPinMode(pinNumber, OUTPUT);
			espDigitalWrite(pinNumber, HIGH);
		}

		inline IECline status(uint8_t pinNumber)
		{
			// To be able to read line we must be set to input, not driving.
			espPinMode(pinNumber, INPUT);
			return espDigitalRead(pinNumber) ? released : pulled;
		}

	private:
		inline void IRAM_ATTR espPinMode(uint8_t pin, uint8_t mode) {
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

		inline void IRAM_ATTR espDigitalWrite(uint8_t pin, uint8_t val) {
	#if defined(ESP8266)
			if(val) GPOS = (1 << pin);
			else GPOC = (1 << pin);
	#elif defined(ESP32)
			digitalWrite(pin, val);
	#endif
		}

		inline int IRAM_ATTR espDigitalRead(uint8_t pin) {
			int val = -1;
	#if defined(ESP8266)
			val = GPIP(pin);
	#elif defined(ESP32)
			val = digitalRead(pin);
	#endif
			return val;
		}
	};

};

#endif
