#include "iec.h"

using namespace CBM;


IEC::IEC() :
	m_state(noFlags)
{
} // ctor

// Set all IEC_signal lines in the correct mode
//
boolean  IEC::init()
{
	// make sure the output states are initially LOW.
	pinMode(IEC_PIN_ATN, OUTPUT);
	pinMode(IEC_PIN_DATA, OUTPUT);
	pinMode(IEC_PIN_CLK, OUTPUT);
//	pinMode(IEC_PIN_SRQ, OUTPUT);
	digitalWrite(IEC_PIN_ATN, false);
	digitalWrite(IEC_PIN_DATA, false);
	digitalWrite(IEC_PIN_CLK, false);
//	digitalWrite(IEC_PIN_SRQ, true);

//#ifdef RESET_C64
//	pinMode(m_resetPin, OUTPUT);
//	digitalWrite(m_resetPin, false);	// only early C64's could be reset by a slave going high.
//#endif

	// initial pin modes in GPIO.
	pinMode(IEC_PIN_ATN, INPUT);
	pinMode(IEC_PIN_CLK, INPUT);
	pinMode(IEC_PIN_DATA, INPUT);
//	pinMode(IEC_PIN_RESET, INPUT);


	// Set port low, we don't need internal pullup
	// and DDR input such that we release all signals
	// IEC_PORT and_eq compl(IEC_BIT_ATN bitor IEC_BIT_CLOCK bitor IEC_BIT_DATA);
	// IEC_DDR and_eq compl(IEC_BIT_ATN bitor IEC_BIT_CLOCK bitor IEC_BIT_DATA);

	m_state = noFlags;

	return true;
} // init

byte  IEC::timeoutWait(byte waitBit, boolean whileHigh)
{
	word t = 0;
	boolean c;
	
#if defined(ESP8266)
	ESP.wdtFeed();
#endif	
	while(t < TIMEOUT) {

		// Check the waiting condition:
		c = readPIN(waitBit);

		if(whileHigh)
		{
			c = not c;	
		}

		if(c)
		{
			//debugPrintln("timeoutWait: false");
			return false;
		}

		delayMicroseconds(1); // The aim is to make the loop at least 3 us
		t++;
	}
	// If down here, we have had a timeout.
	// Release lines and go to inactive state with error flag
	release(IEC_PIN_CLK);
	release(IEC_PIN_DATA);

	m_state = errorFlag;

	// Wait for ATN release, problem might have occured during attention
	//while(status(IEC_PIN_ATN) == pulled);

	// Note: The while above is without timeout. If ATN is held low forever,
	//       the CBM is out in the woods and needs a reset anyways.

	debugPrintf("\r\ntimeoutWait: true [%d] [%d] [%d] [%d]", waitBit, whileHigh, t, m_state);
	return true;
} // timeoutWait


// IEC Recieve byte standard function
//
// Returns data recieved
// Might set flags in iec_state
//
// FIXME: m_iec might be better returning bool and returning read byte as reference in order to indicate any error.
byte  IEC::receiveByte(void)
{
	m_state = noFlags;

	// Wait for talker ready
	if(timeoutWait(IEC_PIN_CLK, false))
		return 0;

	// Say we're ready
	release(IEC_PIN_DATA);

	// Record how long CLOCK is high, more than 200 us means EOI
	byte n = 0;
	while(status(IEC_PIN_CLK) == released && (n < 20)) {
		delayMicroseconds(10);  // this loop should cycle in about 10 us...
		n++;
	}

	if(n >= TIMING_EOI_THRESH) {
		// EOI intermission
		m_state or_eq eoiFlag;

		// Acknowledge by pull down data more than 60 us
		pull(IEC_PIN_DATA);
		delayMicroseconds(TIMING_BIT);
		release(IEC_PIN_DATA);

		// but still wait for clk
		if(timeoutWait(IEC_PIN_CLK, true))
			return 0;
	}

	// Sample ATN
	if(status(IEC_PIN_ATN) == pulled)
		m_state or_eq atnFlag;

	byte data = 0;
	// Get the bits, sampling on clock rising edge:
#if defined(ESP8266)
	ESP.wdtFeed();
#endif	
	for(n = 0; n < 8; n++) {
		data >>= 1;
		if(timeoutWait(IEC_PIN_CLK, false))
			return 0;
		data or_eq (readDATA() ? (1 << 7) : 0);
		if(timeoutWait(IEC_PIN_CLK, true))
			return 0;
	}
	//debugPrintf("%.2X ", data);

	// Signal we accepted data:
	pull(IEC_PIN_DATA);

	return data;
} // receiveByte


// IEC Send byte standard function
//
// Sends the byte and can signal EOI
//
boolean  IEC::sendByte(byte data, boolean signalEOI)
{
	// //Listener must have accepted previous data
	// if(timeoutWait(IEC_PIN_DATA, true))
	// 	return false;

	// Say we're ready
	release(IEC_PIN_CLK);

	// Wait for listener to be ready
	if(timeoutWait(IEC_PIN_DATA, false))
		return false;

	if(signalEOI) {
		// FIXME: Make this like sd2iec and may not need a fixed delay here.
		//debugPrint("{EOI}");

		// Signal eoi by waiting 200 us
		delayMicroseconds(TIMING_EOI_WAIT);

		// get eoi acknowledge:
		if(timeoutWait(IEC_PIN_DATA, true))
			return false;

		if(timeoutWait(IEC_PIN_DATA, false))
			return false;
	}

	delayMicroseconds(TIMING_NO_EOI);

	// Send bits
#if defined(ESP8266)
	ESP.wdtFeed();
#endif	
	for(byte n = 0; n < 8; n++) {
		// FIXME: Here check whether data pin goes low, if so end (enter cleanup)!

		pull(IEC_PIN_CLK);
		// set data
		(data bitand 1) ? release(IEC_PIN_DATA) : pull(IEC_PIN_DATA);

		delayMicroseconds(TIMING_BIT);
		release(IEC_PIN_CLK);
		delayMicroseconds(TIMING_BIT);

		data >>= 1;
	}

	pull(IEC_PIN_CLK);
	release(IEC_PIN_DATA);

	// FIXME: Maybe make the following ending more like sd2iec instead.

	// Line stabilization delay
//	delayMicroseconds(TIMING_STABLE_WAIT);

	// Wait for listener to accept data
	if(timeoutWait(IEC_PIN_DATA, true))
		return false;

	return true;
} // sendByte


// IEC turnaround
boolean  IEC::turnAround(void)
{
	debugPrintf("\r\nturnAround: ");

	// Wait until clock is released
	if(timeoutWait(IEC_PIN_CLK, false))
	{
		debugPrint("false");
		return false;
	}
		

	release(IEC_PIN_DATA);
	delayMicroseconds(TIMING_BIT);
	pull(IEC_PIN_CLK);
	delayMicroseconds(TIMING_BIT);

	debugPrint("true");
	return true;
} // turnAround


// this routine will set the direction on the bus back to normal
// (the way it was when the computer was switched on)
boolean  IEC::undoTurnAround(void)
{
	pull(IEC_PIN_DATA);
	delayMicroseconds(TIMING_BIT);
	release(IEC_PIN_CLK);
	delayMicroseconds(TIMING_BIT);

	debugPrintf("\r\nundoTurnAround:");

	// wait until the computer releases the clock line
	if(timeoutWait(IEC_PIN_CLK, true))
	{
		debugPrint("false");
		return false;
	}

	debugPrint("true");
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
// Return value, see IEC::ATNCheck definition.
IEC::ATNCheck  IEC::checkATN(ATNCmd& atn_cmd)
{
	ATNCheck ret = ATN_IDLE;
	byte i = 0;

	if(status(IEC_PIN_ATN) == pulled) {

		// Attention line is active, go to listener mode and get message. Being fast with the next two lines here is CRITICAL!
		pull(IEC_PIN_DATA);
		release(IEC_PIN_CLK);
		delayMicroseconds(TIMING_ATN_PREDELAY);

		// Get first ATN byte, it is either LISTEN or TALK
		ATNCommand c = (ATNCommand)receive();
		debugPrintf("\r\ncheckATN: %.2X ", c);
		if(m_state bitand errorFlag)
		{
			debugPrintf("\r\nm_state bitand errorFlag 0");
			return ATN_ERROR;
		}

		atn_cmd.code = c;
		
		ATNCommand cc = c;
		if(c != ATN_CODE_UNTALK && c != ATN_CODE_UNLISTEN)
		{
			// Is this a Listen or Talk command
			cc = (ATNCommand)(c bitand ATN_CODE_LISTEN);
			if(cc == ATN_CODE_LISTEN)
			{
				atn_cmd.device = c ^ ATN_CODE_LISTEN; // device specified
			} 
			else
			{
				cc = (ATNCommand)(c bitand ATN_CODE_TALK);
				atn_cmd.device = c ^ ATN_CODE_TALK; // device specified
			}

			// Get the first cmd byte, the atn_cmd.code
			c = (ATNCommand)receive();
			if(m_state bitand errorFlag)
			{
				debugPrintf("\r\nm_state bitand errorFlag 1");
				return ATN_ERROR;
			}
			
			atn_cmd.code = c;
			atn_cmd.command = c bitand 0xF0; // upper nibble, the command itself
			atn_cmd.channel = c bitand 0x0F; // lower nibble is the channel			
		}

		if ( cc == ATN_CODE_LISTEN && isDeviceEnabled(atn_cmd.device) )
		{
			ret = deviceListen(atn_cmd);
		}
		else if ( cc == ATN_CODE_TALK && isDeviceEnabled(atn_cmd.device) )
		{
			ret = deviceTalk(atn_cmd);
		}
		else 
		{
			// Either the message is not for us or insignificant, like unlisten.
			delayMicroseconds(TIMING_ATN_DELAY);
			release(IEC_PIN_DATA);
			release(IEC_PIN_CLK);

			if ( cc == ATN_CODE_UNTALK )
				debugPrint("UNTALK");
			if ( cc == ATN_CODE_UNLISTEN )
				debugPrint("UNLISTEN");
			
			debugPrintf(" (%.2d DEVICE)", atn_cmd.device);			

			// Wait for ATN to release and quit
			while(status(IEC_PIN_ATN) == pulled);
			debugPrintf("\r\ncheckATN: ATN Released\r\n");
		}

		// some delay is required before more ATN business can take place.
		delayMicroseconds(TIMING_ATN_DELAY);

		atn_cmd.strLen = i;			
	}
	else 
	{
		// No ATN, keep lines in a released state.
		release(IEC_PIN_DATA);
		release(IEC_PIN_CLK);
	}

	return ret;
} // checkATN

IEC::ATNCheck  IEC::deviceListen(ATNCmd& atn_cmd)
{
	byte i=0;
	ATNCommand c;

	// Okay, we will listen.
	debugPrintf("(20 LISTEN) (%.2d DEVICE)", atn_cmd.device);

	// If the command is DATA and it is not to expect just a small command on the command channel, then
	// we're into something more heavy. Otherwise read it all out right here until UNLISTEN is received.
	if(atn_cmd.command == ATN_CODE_DATA and atn_cmd.channel not_eq CMD_CHANNEL) 
	{
		// A heapload of data might come now, too big for this context to handle so the caller handles this, we're done here.
		debugPrintf("\r\ncheckATN: %.2X (DATA)      (%.2X COMMAND) (%.2X CHANNEL)", atn_cmd.code, atn_cmd.command, atn_cmd.channel);
		return ATN_CMD_LISTEN;
	}
	else if(atn_cmd.command not_eq ATN_CODE_UNLISTEN)
	//if(c not_eq ATN_CODE_UNLISTEN)
	{

		if(atn_cmd.command == ATN_CODE_OPEN) 
		{
			debugPrintf("\r\ncheckATN: %.2X (%.2X OPEN) (%.2X CHANNEL)", atn_cmd.code, atn_cmd.command, atn_cmd.channel);
		}
		else if(atn_cmd.command == ATN_CODE_CLOSE) 
		{
			debugPrintf("\r\ncheckATN: %.2X (%.2X CLOSE) (%.2X CHANNEL)", atn_cmd.code, atn_cmd.command, atn_cmd.channel);
		}

		// Some other command. Record the cmd string until UNLISTEN is sent
		for(;;) 
		{
			c = (ATNCommand)receive();
			if(m_state bitand errorFlag)
			{
				debugPrintf("\r\nm_state bitand errorFlag 2");
				return ATN_ERROR;
			}
				

			if((m_state bitand atnFlag) and (ATN_CODE_UNLISTEN == c)) 
			{
				debugPrintf(" [%s]", atn_cmd.str);
				debugPrintf("\r\ncheckATN: %.2X (UNLISTEN)", c);
				break;
			}

			if(i >= ATN_CMD_MAX_LENGTH) 
			{
				// Buffer is going to overflow, this is an error condition
				// FIXME: here we should propagate the error type being overflow so that reading error channel can give right code out.
				debugPrintf("\r\nATN_CMD_MAX_LENGTH");
				return ATN_ERROR;
			}
			atn_cmd.str[i++] = c;
			atn_cmd.str[i] = '\0';
		}
		return ATN_CMD;
	}
	return ATN_IDLE;
}

IEC::ATNCheck  IEC::deviceUnListen(ATNCmd& atn_cmd)
{

}

IEC::ATNCheck  IEC::deviceTalk(ATNCmd& atn_cmd)
{
	byte i=0;
	ATNCommand c;

	// Okay, we will talk soon
	debugPrintf("(40 TALK) (%.2d DEVICE)", atn_cmd.device);
	debugPrintf("\r\ncheckATN: %.2X (%.2X SECOND) (%.2X CHANNEL)", atn_cmd.code, atn_cmd.command, atn_cmd.channel);

	while(status(IEC_PIN_ATN) == pulled) {
		if(status(IEC_PIN_CLK) == released) {
			c = (ATNCommand)receive();
			if(m_state bitand errorFlag)
				return ATN_ERROR;

			if(i >= ATN_CMD_MAX_LENGTH) {
				// Buffer is going to overflow, this is an error condition
				// FIXME: here we should propagate the error type being overflow so that reading error channel can give right code out.
				return ATN_ERROR;
			}
			atn_cmd.str[i++] = c;
			atn_cmd.str[i] = '\0';
		}
	}

	// Now ATN has just been released, do bus turnaround
	if(not turnAround())
		return ATN_ERROR;

	// We have recieved a CMD and we should talk now:
	return ATN_CMD_TALK;
}

IEC::ATNCheck  IEC::deviceUnTalk(ATNCmd& atn_cmd)
{

}

//boolean  IEC::checkRESET()
//{
//	return readRESET();
//	return false;
//} // checkRESET


// IEC_receive receives a byte
//
byte  IEC::receive()
{
	byte data;
	data = receiveByte();
	return data;
} // receive


// IEC_send sends a byte
//
boolean  IEC::send(byte data)
{
	return sendByte(data, false);
} // send


// Same as IEC_send, but indicating that this is the last byte.
//
boolean  IEC::sendEOI(byte data)
{
	debugPrintf("\r\nEOI Sent!");
	if(sendByte(data, true)) {
		//debugPrint("true");

		// As we have just send last byte, turn bus back around
		if(undoTurnAround())
		{
			return true;
		}
	}

	//debugPrint("false");
	return false;
} // sendEOI


// A special send command that informs file not found condition
//
boolean  IEC::sendFNF()
{
	// Message file not found by just releasing lines
	release(IEC_PIN_DATA);
	release(IEC_PIN_CLK);

	// Hold back a little...
	delayMicroseconds(TIMING_FNF_DELAY);

	debugPrintf("\r\nsendFNF: true");
	return true;
} // sendFNF





bool  IEC::isDeviceEnabled(const byte deviceNumber)
{
	return (enabledDevices & (1<<deviceNumber));
} // isDeviceEnabled

IEC::IECState  IEC::state() const
{
	return static_cast<IECState>(m_state);
} // state
