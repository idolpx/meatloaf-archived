#ifndef IEC_H
#define IEC_H

#include <Arduino.h>
//#include "global_defines.h"
#include "global_defines.h"
#include "cbmdefines.h"
#include "Petscii.h"

class IEC
{
public:
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

	// Return values for checkATN:
	enum ATNCheck
	{
		ATN_IDLE = 0,		// Nothing recieved of our concern
		ATN_CMD = 1,		// A command is recieved
		ATN_CMD_LISTEN = 2, // A command is recieved and data is coming to us
		ATN_CMD_TALK = 3,	// A command is recieved and we must talk now
		ATN_ERROR = 4,		// A problem occoured, reset communication
		ATN_RESET = 5		// The IEC bus is in a reset state (RESET line).
	};

	// IEC ATN commands:
	enum ATNCommand
	{
		ATN_CODE_GLOBAL = 0x00,	  // 0x00 + cmd (global command)
		ATN_CODE_LISTEN = 0x20,	  // 0x20 + device_id (LISTEN)
		ATN_CODE_UNLISTEN = 0x3F, // 0x3F (UNLISTEN)
		ATN_CODE_TALK = 0x40,	  // 0x40 + device_id (TALK)
		ATN_CODE_UNTALK = 0x5F,	  // 0x5F (UNTALK)
		ATN_CODE_DATA = 0x60,	  // 0x60 + channel (SECOND)
		ATN_CODE_CLOSE = 0xE0,	  // 0xE0 + channel (CLOSE)
		ATN_CODE_OPEN = 0xF0	  // 0xF0 + channel (OPEN)
	};

	// ATN command struct maximum command length:
	enum
	{
		ATN_CMD_MAX_LENGTH = 40
	};

	typedef struct _tagATNCMD
	{
		byte code;
		byte command;
		byte channel;
		byte device;
		byte str[ATN_CMD_MAX_LENGTH];
		byte strLen;
	} ATNCmd;

	IEC();
	~IEC() {}

	// Initialise iec driver
	boolean init();

	// Checks if CBM is sending an attention message. If this is the case,
	// the message is recieved and stored in atn_cmd.
	ATNCheck checkATN(ATNCmd &atn_cmd);

	// Checks if CBM is sending a reset (setting the RESET line high). This is typicall
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
//	boolean checkRESET();

	// Sends a byte. The communication must be in the correct state: a load command
	// must just have been recieved. If something is not OK, FALSE is returned.
	boolean send(byte data);

	// Same as IEC_send, but indicating that this is the last byte.
	boolean sendEOI(byte data);

	// A special send command that informs file not found condition
	boolean sendFNF();

	// Recieves a byte
	byte receive();

	// Enabled Device Bit Mask
	uint32_t enabledDevices;
	bool isDeviceEnabled(const byte deviceNumber);
	void enableDevice(const byte deviceNumber);
	void disableDevice(const byte deviceNumber);

	IECState state() const;

	// inline boolean readATN()
	// {
	// 	return readPIN(IEC_PIN_ATN);
	// }

	// inline boolean readCLOCK()
	// {
	// 	return readPIN(IEC_PIN_CLK);
	// }

	// inline boolean readDATA()
	// {
	// 	return readPIN(IEC_PIN_DATA);
	// }

	// inline boolean readSRQ()
	// {
	// 	return readPIN(IEC_PIN_SRQ);
	// }

	// inline boolean readRESET()
	// {
	// 	return readPIN(IEC_PIN_RESET);
	// }

private:
	// IEC Bus Commands
	ATNCheck deviceListen(ATNCmd &atn_cmd);	  // 0x20 + device_id 	Listen, device (0–30)
	ATNCheck deviceUnListen(ATNCmd &atn_cmd); // 0x3F 				Unlisten, all devices
	ATNCheck deviceTalk(ATNCmd &atn_cmd);	  // 0x40 + device_id 	Talk, device
	ATNCheck deviceUnTalk(ATNCmd &atn_cmd);	  // 0x5F 				Untalk, all devices
	ATNCheck deviceReopen(ATNCmd &atn_cmd);	  // 0x60 + channel		Reopen, channel (0–15)
	ATNCheck deviceClose(ATNCmd &atn_cmd);	  // 0xE0 + channel		Close, channel
	ATNCheck deviceOpen(ATNCmd &atn_cmd);	  // 0xF0 + channel		Open, channel

	byte timeoutWait(byte waitBit, boolean whileHigh);
	byte receiveByte(void);
	boolean sendByte(byte data, boolean signalEOI);
	boolean turnAround(void);
	boolean undoTurnAround(void);

	// true => PULL => DIGI_LOW
	inline void pull(int pinNumber)
	{
		espPinMode(pinNumber, OUTPUT);
		espDigitalWrite(pinNumber, LOW);
	}

	// false => RELEASE => DIGI_HIGH
	inline void release(int pinNumber)
	{
		// releasing line can set to input mode, which won't drive the bus - simple way to mimic open collector
		espPinMode(pinNumber, OUTPUT);
		espDigitalWrite(pinNumber, HIGH);
	}

	inline IECline status(int pinNumber)
	{
		// To be able to read line we must be set to input, not driving.
		espPinMode(pinNumber, INPUT);
		return espDigitalRead(pinNumber) ? released : pulled;
	}

	// true == PULL == LOW, false == RELEASE == HIGH
	bool readPIN(byte pinNumber)
	{
		// To be able to read line we must be set to input, not driving.
		espPinMode(pinNumber, INPUT);
		return espDigitalRead(pinNumber) ? true : false;
	}

	// true == PULL == LOW, false == RELEASE == HIGH
	void writePIN(byte pinNumber, boolean state)
	{
		espPinMode(pinNumber, state ? OUTPUT : INPUT);
		espDigitalWrite(pinNumber, state ? LOW : HIGH);
	}

	// inline void writeATN(boolean state)
	// {
	// 	writePIN(IEC_PIN_ATN, state);
	// }

	// inline void writeCLOCK(boolean state)
	// {
	// 	writePIN(IEC_PIN_CLK, state);
	// }

	// inline void writeDATA(boolean state)
	// {
	// 	writePIN(IEC_PIN_DATA, state);
	// }

	// inline void writeSRQ(boolean state)
	// {
	// 	writePIN(IEC_PIN_SRQ, state);
	// }

	inline void ICACHE_RAM_ATTR espPinMode(uint8_t pin, uint8_t mode) {
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

		// uint32_t pinFunction = 0, pinControl = 0;

		// //lock gpio
		// if(mode & INPUT) {
		// 	if(pin < 32) {
		// 		GPIO.enable_w1tc = ((uint32_t)1 << pin);
		// 	} else {
		// 		GPIO.enable1_w1tc.val = ((uint32_t)1 << (pin - 32));
		// 	}
		// } else if(mode & OUTPUT) {
		// 	if(pin > 33){
		// 		//unlock gpio
		// 		return;//pins above 33 can be only inputs
		// 	} else if(pin < 32) {
		// 		GPIO.enable_w1ts = ((uint32_t)1 << pin);
		// 	} else {
		// 		GPIO.enable1_w1ts.val = ((uint32_t)1 << (pin - 32));
		// 	}
		// }

		// pinFunction |= ((uint32_t)2 << FUN_DRV_S);//what are the drivers?
		// pinFunction |= FUN_IE;//input enable but required for output as well?

		// if(mode & (INPUT | OUTPUT)) {
		// 	pinFunction |= ((uint32_t)2 << MCU_SEL_S);
		// } else if(mode == SPECIAL) {
		// 	pinFunction |= ((uint32_t)(((pin)==1||(pin)==3)?0:1) << MCU_SEL_S);
		// } else {
		// 	pinFunction |= ((uint32_t)(mode >> 5) << MCU_SEL_S);
		// }

		// ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[pin].reg) = pinFunction;

		// if(mode & OPEN_DRAIN) {
		// 	pinControl = (1 << GPIO_PIN0_PAD_DRIVER_S);
		// }

		// GPIO.pin[pin].val = pinControl;
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

	inline int ICACHE_RAM_ATTR espDigitalRead(uint8_t pin) {
		int val = -1;
#if defined(ESP8266)
		val = GPIP(pin);
#elif defined(ESP32)
		val = digitalRead(pin);
#endif
		return val;
	}

	// communication must be reset
	byte m_state;
};

#endif
