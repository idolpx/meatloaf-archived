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

#include "iec.h"

using namespace CBM; 
using namespace Protocol;


// IEC::IEC() :
// 	m_state(noFlags)
// {
// } // ctor

// Set all IEC_signal lines in the correct mode
//
bool IEC::init()
{
	// make sure the output states are initially LOW
	protocol.release(IEC_PIN_ATN);
	protocol.release(IEC_PIN_CLK);
	protocol.release(IEC_PIN_DATA);
	protocol.release(IEC_PIN_SRQ);

	// initial pin modes in GPIO
	pinMode(IEC_PIN_ATN, INPUT);
	pinMode(IEC_PIN_CLK, INPUT);
	pinMode(IEC_PIN_DATA, INPUT);	
	pinMode(IEC_PIN_SRQ, INPUT);
	pinMode(IEC_PIN_RESET, INPUT);

#ifdef SPLIT_LINES
	pinMode(IEC_PIN_CLK_OUT, OUTPUT);
	pinMode(IEC_PIN_DATA_OUT, OUTPUT);
#endif

	protocol.m_state = noFlags;

	return true;
} // init

// IEC turnaround
bool IEC::turnAround(void)
{
	/*
	TURNAROUND
	An unusual sequence takes place following ATN if the computer wishes the remote device to
	become a talker. This will usually take place only after a Talk command has been sent.
	Immediately after ATN is protocol.released, the selected device will be behaving like a listener. After all, it's
	been listening during the ATN cycle, and the computer has been a talker. At this instant, we 
	have "wrong way" logic; the device is holding down the Data	line, and the computer is holding the 
	Clock line. We must turn this around. Here's the sequence:
	the computer quickly realizes what's going on, and pulls the Data line to true (it's already there), as
	well as releasing the Clock line to false. The device waits for this: when it sees the Clock line go
	true [sic], it protocol.releases the Data line (which stays true anyway since the computer is now holding it down)
	and then pulls down the Clock line. We're now in our starting position, with the talker (that's the
	device) holding the Clock true, and the listener (the computer) holding the Data line true. The
	computer watches for this state; only when it has gone through the cycle correctly will it be ready
	to receive data. And data will be signalled, of course, with the usual sequence: the talker protocol.releases
	the Clock line to signal that it's ready to send.
	*/
	Debug_printf("IEC turnAround: ");

	// Wait until clock is protocol.released
	while(protocol.status(IEC_PIN_CLK) != released);
		

	protocol.release(IEC_PIN_DATA);
	delayMicroseconds(TIMING_BIT);
	protocol.pull(IEC_PIN_CLK);
	delayMicroseconds(TIMING_BIT);

	Debug_println("complete");
	return true;
} // turnAround


// this routine will set the direction on the bus back to normal
// (the way it was when the computer was switched on)
bool IEC::undoTurnAround(void)
{
	protocol.pull(IEC_PIN_DATA);
	delayMicroseconds(TIMING_BIT);
	protocol.release(IEC_PIN_CLK);
	delayMicroseconds(TIMING_BIT);

	Debug_printf("IEC undoTurnAround: ");

	// wait until the computer protocol.releases the clock line
	while(protocol.status(IEC_PIN_CLK) != released);

	Debug_println("complete");
	return true;
} // undoTurnAround


/******************************************************************************
 *                                                                             *
 *                               Public functions                              *
 *                                                                             *
 ******************************************************************************/

// This function checks and deals with atn signal commands
//
// If a command is recieved, the atn_cmd.string is saved in atn_cmd. Only commands
// for *this* device are dealt with.
//
/** from Derogee's "IEC Disected"
 * ATN SEQUENCES
 * When ATN is pulled true, everybody stops what they are doing. The processor will quickly protocol.pull the
 * Clock line true (it's going to send soon), so it may be hard to notice that all other devices protocol.release the
 * Clock line. At the same time, the processor protocol.releases the Data line to false, but all other devices are
 * getting ready to listen and will each protocol.pull Data to true. They had better do this within one
 * millisecond (1000 microseconds), since the processor is watching and may sound an alarm ("device
 * not available") if it doesn't see this take place. Under normal circumstances, transmission now
 * takes place as previously described. The computer is sending commands rather than data, but the
 * characters are exchanged with exactly the same timing and handshakes as before. All devices
 * receive the commands, but only the specified device acts upon it. This results in a curious
 * situation: you can send a command to a nonexistent device (try "OPEN 6,6") - and the computer
 * will not know that there is a problem, since it receives valid handshakes from the other devices.
 * The computer will notice a problem when you try to send or receive data from the nonexistent
 * device, since the unselected devices will have dropped off when ATN ceased, leaving you with
 * nobody to talk to.
 */
// Return value, see IEC::BusState definition.
IEC::BusState IEC::service(ATNCmd& atn_cmd)
{
	IEC::BusState r = BUS_IDLE;
	bool releaseLines = false;

	// Checks if CBM is sending a reset (setting the RESET line high). This is typically
	// when the CBM is reset itself. In this case, we are supposed to reset all states to initial.
	// if(status(IEC_PIN_RESET) == pulled) 
	// {
	// 	if (status(IEC_PIN_ATN) == pulled)
	// 	{
	// 		// If RESET & ATN are both pulled then CBM is off
	// 		return BUS_IDLE;
	// 	}
		
	// 	return BUS_RESET;
	// }


	// Attention line is pulled, go to listener mode and get message.
	// Being fast with the next two lines here is CRITICAL!
	protocol.release(IEC_PIN_CLK);
	protocol.pull(IEC_PIN_DATA);
	delayMicroseconds(TIMING_ATN_PREDELAY);

	// Get first ATN byte, it is either LISTEN or TALK
	int8_t c = (Command)receive();
	Debug_printf("ATN: %.2X ", c);
	if(protocol.m_state bitand errorFlag)
	{
		Debug_printv("Get first ATN byte");
		return BUS_ERROR;
	}

	atn_cmd.code = c;
	
	int8_t cc = c;
	if(c != IEC_UNTALK && c != IEC_UNLISTEN)
	{
		// Is this a Listen or Talk command?
		cc = (c bitand IEC_LISTEN);
		if(cc == IEC_LISTEN)
		{
			atn_cmd.device = c ^ IEC_LISTEN; // device specified, '^' = XOR
		} 
		else
		{
			cc = (c bitand IEC_TALK);
			atn_cmd.device = c ^ IEC_TALK; // device specified
		}

		// Is this command for us?
		if ( isDeviceEnabled(atn_cmd.device) )
		{
			// Get the first cmd byte, the atn_cmd.code
			c = receive();
			if(protocol.m_state bitand errorFlag)
			{
				Debug_printv("Get the first cmd byte");
				return BUS_ERROR;
			}
			
			atn_cmd.code = c;
			atn_cmd.command = c bitand 0xF0; // upper nibble, command
			atn_cmd.channel = c bitand 0x0F; // lower nibble, channel

			if ( cc == IEC_LISTEN )
			{
				r = deviceListen(atn_cmd);
			}
			else if ( cc == IEC_TALK )
			{
				r = deviceTalk(atn_cmd);
			}
			releaseLines = protocol.m_state bitand errorFlag;				
		}
		else
		{
			// Command is not for us
			if ( cc == IEC_LISTEN )
			{
				Debug_printf("(20 LISTEN) (%.2d DEVICE)\r\n", atn_cmd.device);
			}
			else if ( cc == IEC_TALK )
			{
				Debug_printf("(40 TALK) (%.2d DEVICE)\r\n", atn_cmd.device);
			}
			releaseLines = true;
		}
	}
	else if ( cc == IEC_UNLISTEN )
	{
		Debug_println("UNLISTEN");
		releaseLines = true;
	}
	else if ( cc == IEC_UNTALK )
	{
		Debug_println("UNTALK");
		releaseLines = true;
	}		
			
	// Was there an error?
	if(releaseLines)
	{
		// Release lines
		protocol.release(IEC_PIN_CLK);
		protocol.release(IEC_PIN_DATA);

		// Wait for ATN to protocol.release and quit
		while(protocol.status(IEC_PIN_ATN) != released)
		{
			ESP.wdtFeed();
		}
		//delayMicroseconds(TIMING_ATN_DELAY);
	}
	// Don't do anything here or it could cause LOAD ERROR!!!

	return r;
} // service

IEC::BusState IEC::deviceListen(ATNCmd& atn_cmd)
{
	byte i=0;

	// Okay, we will listen.
	Debug_printf("(20 LISTEN) (%.2d DEVICE) ", atn_cmd.device);

	// If the command is SECONDARY and it is not to expect just a small command on the command channel, then
	// we're into something more heavy. Otherwise read it all out right here until UNLISTEN is received.
	if(atn_cmd.command == IEC_SECONDARY && atn_cmd.channel not_eq CMD_CHANNEL) 
	{
		// A heapload of data might come now, too big for this context to handle so the caller handles this, we're done here.
		Debug_printf("(%.2X SECONDARY) (%.2X CHANNEL)\r\n", atn_cmd.command, atn_cmd.channel);
		return BUS_LISTEN;
	}

	// OPEN
	else if(atn_cmd.command == IEC_SECONDARY || atn_cmd.command == IEC_OPEN) 
	{
		Debug_printf("(%.2X OPEN) (%.2X CHANNEL) ", atn_cmd.command, atn_cmd.channel);

		// Some other command. Record the cmd string until UNLISTEN is sent
		for(;;) 
		{
			uint8_t c = receive();
			if(protocol.m_state bitand errorFlag)
			{
				Debug_printv("Some other command [%.2X]", c);
				return BUS_ERROR;
			}
				

			//if((m_state bitand atnFlag) and (IEC_UNLISTEN == c))
			if(c == IEC_UNLISTEN)
			{
				Debug_printf("[%s] (%.2X UNLISTEN)\r\n", atn_cmd.str, c);
				break;
			}

			if(i >= IEC_CMD_MAX_LENGTH) 
			{
				// Buffer is going to overflow, this is an error condition
				// FIXME: here we should propagate the error type being overflow so that reading error channel can give right code out.
				Debug_printv("IEC_CMD_MAX_LENGTH");
				return BUS_ERROR;
			}
			if(c != 0x0D)
			{
				atn_cmd.str[i++] = (uint8_t)c;
				atn_cmd.str[i] = '\0';			
			}
		}		
	}

	// CLOSE Named Channel
	else if(atn_cmd.command == IEC_CLOSE) 
	{
		Debug_printf("(%.2X CLOSE) (%.2X CHANNEL)\r\n", atn_cmd.command, atn_cmd.channel);
		return BUS_COMMAND;
	}

	// Unknown
	else
	{
		Debug_printv("OTHER (%.2X COMMAND) (%.2X CHANNEL) ", atn_cmd.command, atn_cmd.channel);
	}

	if(strlen(atn_cmd.str))
		return BUS_COMMAND;
	else
		return BUS_IDLE;
}

void IEC::deviceUnListen(void)
{
	Debug_printv("");

	// Release lines
	protocol.release(IEC_PIN_CLK);
	protocol.release(IEC_PIN_DATA);

	// Wait for ATN to protocol.release and quit
	while(protocol.status(IEC_PIN_ATN) == pulled)
	{
		ESP.wdtFeed();
	}
}

IEC::BusState IEC::deviceTalk(ATNCmd& atn_cmd)
{
	byte i = 0;

	// Okay, we will talk soon
	Debug_printf("(40 TALK) (%.2d DEVICE) (%.2X SECONDARY) (%.2X CHANNEL)\r\n", atn_cmd.device, atn_cmd.command, atn_cmd.channel);

	// Record the cmd string until ATN is protocol.released
	while(protocol.status(IEC_PIN_ATN) == pulled) 
	{
		int16_t c = receive();

		if(i >= IEC_CMD_MAX_LENGTH) 
		{
			// Buffer is going to overflow, this is an error condition
			// FIXME: here we should propagate the error type being overflow so that reading error channel can give right code out.
			Debug_printv("IEC_CMD_MAX_LENGTH");
			return BUS_ERROR;
		}
		atn_cmd.str[i++] = (uint8_t)c;
		atn_cmd.str[i] = '\0';
	}

	// Delay after ATN is protocol.released
	//delayMicroseconds(TIMING_BIT);

	// Now do bus turnaround
	if(not turnAround())
		return BUS_ERROR;

	// We have recieved a CMD and we should talk now:
	return BUS_TALK;
}

void IEC::deviceUnTalk(void)
{
	Debug_printv("");

	// Release lines
	protocol.release(IEC_PIN_CLK);
	protocol.release(IEC_PIN_DATA);

	// Wait for ATN to protocol.release and quit
	while(protocol.status(IEC_PIN_ATN) == pulled)
	{
		ESP.wdtFeed();
	}
}

// boolean  IEC::checkRESET()
// {
// 	return readRESET();
// 	return false;
// } // checkRESET


// IEC_receive receives a byte
//
int16_t IEC::receive()
{
	int16_t data;
	data = protocol.receiveByte(); // Standard CBM Timing
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif
	if(data < 0)
		protocol.m_state = errorFlag;

	return data;
} // receive


// IEC_send sends a byte
//
bool IEC::send(uint8_t data)
{
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif	
	return protocol.sendByte(data, false); // Standard CBM Timing
} // send

bool IEC::send(std::string data)
{
	for (int i = 0; i < data.length(); ++i)
		send(data[i]);

	return true;
}


// Same as IEC_send, but indicating that this is the last byte.
//
bool IEC::sendEOI(uint8_t data)
{
#ifdef DATA_STREAM
	Debug_printf("%.2X ", data);
#endif	
	Debug_println("\r\nEOI Sent!");
	if(protocol.sendByte(data, true)) 
	{
		// As we have just send last byte, turn bus back around
		if(undoTurnAround())
		{
			return true;
		}
	}

	return false;
} // sendEOI


// A special send command that informs file not found condition
//
bool IEC::sendFNF()
{
	// Message file not found by just releasing lines
	protocol.release(IEC_PIN_DATA);
	protocol.release(IEC_PIN_CLK);

	// Hold back a little...
	delayMicroseconds(TIMING_FNF_DELAY);

	Debug_println("\r\nFNF Sent!");
	return true;
} // sendFNF


bool IEC::isDeviceEnabled(const uint8_t deviceNumber)
{
	return (enabledDevices & (1<<deviceNumber));
} // isDeviceEnabled

void IEC::enableDevice(const uint8_t deviceNumber)
{
	enabledDevices |= 1UL << deviceNumber;
} // enableDevice

void IEC::disableDevice(const uint8_t deviceNumber)
{
	enabledDevices &= ~(1UL << deviceNumber);
} // disableDevice


IECState IEC::state()
{
	return static_cast<IECState>(protocol.m_state);
} // state


void IEC::debugTiming()
{
	int pin = IEC_PIN_ATN;
	protocol.pull(pin);
	delayMicroseconds(1000); // 1000
	protocol.release(pin);
	delayMicroseconds(1000);

	pin = IEC_PIN_CLK;
	protocol.pull(pin);
	delayMicroseconds(20); // 20
	protocol.release(pin);
	delayMicroseconds(1);

	pin = IEC_PIN_DATA;
	protocol.pull(pin);
	delayMicroseconds(50); // 50
	protocol.release(pin);
	delayMicroseconds(1);

	pin = IEC_PIN_SRQ;
	protocol.pull(pin);
	delayMicroseconds(60); // 60
	protocol.release(pin);
	delayMicroseconds(1);

	pin = IEC_PIN_ATN;
	protocol.pull(pin);
	delayMicroseconds(100); // 100
	protocol.release(pin);
	delayMicroseconds(1);

	pin = IEC_PIN_CLK;
	protocol.pull(pin);
	delayMicroseconds(200); // 200
	protocol.release(pin);
	delayMicroseconds(1);
}