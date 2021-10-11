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

// BIT Flags
#define CLEAR           0x0C      // clear all flags
#define ATN_PULLED      (1 << 0)  // might be set by iec_receive
#define EOI_RECVD       (1 << 1)
#define COMMAND_RECVD   (1 << 2)
#define JIFFY_ACTIVE    (1 << 3)
#define JIFFY_LOAD      (1 << 4)
#define ERROR           (1 << 5)  // if this flag is set, something went wrong

// IEC protocol timing consts in microseconds (us)
#define TIMING_BIT           65    // bit clock hi/lo time
#define TIMING_NO_EOI        5     // delay before bits
#define TIMING_EOI_WAIT      256   // delay to signal EOI
#define TIMING_EOI_ACK       60    // threshold for Acknowledging EOI
#define TIMING_EOI_THRESH    20    // threshold for EOI detect
#define TIMING_STABLE_WAIT   20    // line stabilization
#define TIMING_ATN_PREDELAY  50    // delay required in ATN
#define TIMING_ATN_DELAY     100   // delay required after ATN
#define TIMING_FNF_DELAY     100   // delay after FNF
#define TIMING_BYTE_ACK      1000  // threshold for Acknowledging Byte

// IEC-Disected p10-11
#define TIMEOUT_Tat    1000    // ATN RESPONSE (REQUIRED) - min/typ/max -/-/1000us (If maximum time exceeded, device not present error.)
#define TIMING_Th      0       // LISTENER HOLD-OFF - min/typ/max 0/-/infinte
#define TIMING_Tne     40      // NON-EOI RESPONSE TO RFD - min/typ/max -/40us/200us (If maximum time exceeded, EOI response required.)
#define TIMEOUT_Tne    200
#define TIMING_Ts      70      // BIT SET-UP TALKER - min/typ/max 20us/70us/- (Tv and Tpr minimum must be 60μ s for external device to be a talker. )
#define TIMING_Tv      65      // DATA VALID - min/typ/max 20us/20us/-
#define TIMING_Tf      20      // FRAME HANDSHAKE - min/typ/max 0/20us/1000us (If maximum time exceeded, frame error.)
#define TIMEOUT_Tf     1000
#define TIMING_Tr      20      // FRAME TO RELEASE OF ATN - min/typ/max 20us/-/-
#define TIMING_Tbb     100     // BETWEEN BYTES TIME - min/typ/max 100us/-/-
#define TIMING_Tye     250     // EOI RESPONSE TIME - min/typ/max 200us/250us/-
#define TIMING_Tei     60      // EOI RESPONSE HOLD TIME - min/typ/max 60us/-/- (Tei minimum must be 80μ s for external device to be a listener.)
#define TIMING_Try     30      // TALKER RESPONSE LIMIT - min/typ/max 0/30us/60us
#define TIMEOUT_Try    60
#define TIMING_Tpr     30      // BYTE-ACKNOWLEDGE 4 - min/typ/max 20us/30us/- (Tv and Tpr minimum must be 60μ s for external device to be a talker.)
#define TIMING_Ttk     30      // TALK-ATTENTION RELEASE - min/typ/max 20us/30us/100us
#define TIMEOUT_Ttk    100
#define TIMING_Tdc     0       // TALK-ATTENTION ACKNOWLEDGE - min/typ/max 0/-/-
#define TIMING_Tda     80      // TALK-ATTENTION ACK. HOLD - min/typ/max 80us/-/-
#define TIMING_Tfr     60      // EOI ACKNOWLEDGE - min/typ/max 60us/-/-

// See timeoutWait
#define TIMEOUT 20000 // 1ms
#define TIMED_OUT 0
#define FOREVER 0

#define PULLED    true
#define RELEASED  false

namespace Protocol
{
	class CBMStandardSerial
	{
	public:
		// communication must be reset
		uint8_t flags = 0;

		virtual int16_t receiveByte(uint8_t device);
		virtual bool sendByte(uint8_t data, bool signalEOI);
		virtual size_t timeoutWait(uint8_t iecPIN, bool lineStatus, size_t wait = TIMEOUT, size_t step = 3);


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

		inline bool status(uint8_t pinNumber)
		{
			// To be able to read line we must be set to input, not driving.
			espPinMode(pinNumber, INPUT);
			return espDigitalRead(pinNumber) ? RELEASED : PULLED;
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
