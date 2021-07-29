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

#include "cbmstandardserial.h"

using namespace Protocol;

// STEP 1: READY TO SEND
// Sooner or later, the talker will want to talk, and send a character. 
// When it's ready to go, it releases the Clock line to false.  This signal change might be 
// translated as "I'm ready to send a character." The listener must detect this and respond, 
// but it doesn't have to do so immediately. The listener will respond  to  the  talker's  
// "ready  to  send"  signal  whenever  it  likes;  it  can  wait  a  long  time.    If  it's  
// a printer chugging out a line of print, or a disk drive with a formatting job in progress, 
// it might holdback for quite a while; there's no time limit. 
int16_t  CBMStandardSerial::receiveByte(void)
{
	flags = CLEAR;

	// Wait for talker ready
	while(status(IEC_PIN_CLK) != released)
	{
		ESP.wdtFeed();
	}


	// Say we're ready
	// STEP 2: READY FOR DATA
	// When  the  listener  is  ready  to  listen,  it  releases  the  Data  
	// line  to  false.    Suppose  there  is  more  than one listener.  The Data line will go false 
	// only when all listeners have released it - in other words, when  all  listeners  are  ready  
	// to  accept  data.  What  happens  next  is  variable.
	release(IEC_PIN_DATA);
	while(status(IEC_PIN_DATA) != released); // Wait for all other devices to release the data line
	//timeoutWait(IEC_PIN_DATA, released, FOREVER, 1);

	// Either  the  talker  will pull the 
	// Clock line back to true in less than 200 microseconds - usually within 60 microseconds - or it  
	// will  do  nothing.    The  listener  should  be  watching,  and  if  200  microseconds  pass  
	// without  the Clock line going to true, it has a special task to perform: note EOI.

	uint8_t n = 0;
	while(status(IEC_PIN_CLK) == released && (n < 20)) {
		delayMicroseconds(10);  // this loop should cycle in about 10 us...
		n++;
	}

	if(n >= TIMING_EOI_THRESH)
	
	//if(timeoutWait(IEC_PIN_CLK, pulled, TIMING_EOI_WAIT))
	{
		// INTERMISSION: EOI
		// If the Ready for Data signal isn't acknowledged by the talker within 200 microseconds, the 
		// listener knows  that  the  talker  is  trying  to  signal  EOI.    EOI,  which  formally  
		// stands  for  "End  of  Indicator," means  "this  character  will  be  the  last  one."    
		// If  it's  a  sequential  disk  file,  don't  ask  for  more:  there will be no more.  If it's 
		// a relative record, that's the end of the record.  The character itself will still be coming, but 
		// the listener should note: here comes the last character. So if the listener sees the 200 microsecond  
		// time-out,  it  must  signal  "OK,  I  noticed  the  EOI"  back  to  the  talker,    I  does  this  
		// by pulling  the  Data  line  true  for  at  least  60  microseconds,  and  then  releasing  it.  
		// The  talker  will  then revert to transmitting the character in the usual way; within 60 microseconds 
		// it will pull the Clock line  true,  and  transmission  will  continue.  At  this point,  the  Clock  
		// line  is  true  whether  or  not  we have gone through the EOI sequence; we're back to a common 
		// transmission sequence.

		flags or_eq EOI_RECVD;

		// Acknowledge by pull down data more than 60us
		pull(IEC_PIN_DATA);
		delayMicroseconds(TIMING_Tye);
		release(IEC_PIN_DATA);

		// but still wait for CLK to be pulled
		if(timeoutWait(IEC_PIN_CLK, pulled) == TIMED_OUT)
		{
			Debug_printv("After Acknowledge EOI");
			return -1; // return error because timeout
		}		
	}

	// Sample ATN and set flag to indicate SELECT or DATA mode
	if(status(IEC_PIN_ATN) == pulled)
		flags or_eq ATN_PULLED;

	
	// STEP 3: SENDING THE BITS
	// The talker has eight bits to send.  They will go out without handshake; in other words, 
	// the listener had better be there to catch them, since the talker won't wait to hear from the listener.  At this 
	// point, the talker controls both lines, Clock and Data.  At the beginning of the sequence, it is holding the 
	// Clock true, while the Data line is released to false.  the Data line will change soon, since we'll sendthe data 
	// over it. The eights bits will go out from the character one at a time, with the least significant bit going first.  
	// For example, if the character is the ASCII question mark, which is  written  in  binary  as  00011111,  the  ones  
	// will  go out  first,  followed  by  the  zeros.  Now,  for  each bit, we set the Data line true or false according 
	// to whether the bit is one or zero.  As soon as that'sset, the Clock line is released to false, signalling "data ready."  
	// The talker will typically have a bit in  place  and  be  signalling  ready  in  70  microseconds  or  less.  Once  
	// the  talker  has  signalled  "data ready," it will hold the two lines steady for at least 20 microseconds timing needs 
	// to be increased to 60  microseconds  if  the  Commodore  64  is  listening,  since  the  64's  video  chip  may  
	// interrupt  the processor for 42 microseconds at a time, and without the extra wait the 64 might completely miss a 
	// bit. The listener plays a passive role here; it sends nothing, and just watches.  As soon as it sees the Clock line 
	// false, it grabs the bit from the Data line and puts it away.  It then waits for the clock line to go true, in order 
	// to prepare for the next bit. When the talker figures the data has been held for a sufficient  length  of  time,  it  
	// pulls  the  Clock  line true  and  releases  the  Data  line  to  false.    Then  it starts to prepare the next bit.

	// Listening for bits
#if defined(ESP8266)
	ESP.wdtFeed();
#endif
	uint8_t data = 0;
	uint8_t bit_time;  // Used to detect JiffyDOS
	for(uint8_t n = 0; n < 8; n++) {
		data >>= 1;

		// // if ATN is pulled, exit and cleanup
		// if(status(IEC_PIN_ATN) == pulled)
		// {	
		// 	return -1;
		// }

		// wait for bit to be ready to read
		if(timeoutWait(IEC_PIN_CLK, released) == TIMED_OUT)
		{
			Debug_printv("wait for bit to be ready to read");
			return -1; // return error because timeout
		}

		// get bit
		data or_eq (status(IEC_PIN_DATA) == released ? (1 << 7) : 0);

		// wait for talker to finish sending bit
		bit_time = timeoutWait(IEC_PIN_CLK, pulled);
		if(bit_time == TIMED_OUT)
		{
			Debug_printv("wait for talker to finish sending bit");
			return -1; // return error because timeout
		}
	}
	//Debug_printv("Last bit time %dus", bit_time);

	// STEP 4: FRAME HANDSHAKE
	// After the eighth bit has been sent, it's the listener's turn to acknowledge.  At this moment, the Clock line  is  true  
	// and  the  Data  line  is  false.    The  listener  must  acknowledge  receiving  the  byte  OK  by pulling the Data 
	// line to true. The talker is now watching the Data line.  If the listener doesn't pull the  Data  line  true  within  
	// one  millisecond  -  one  thousand  microseconds  -  it  will  know  that something's wrong and may alarm appropriately.

	// Acknowledge byte received
	pull(IEC_PIN_DATA);

	// STEP 5: START OVER
	// We're  finished,  and  back  where  we  started.    The  talker  is  holding  the  Clock  line  true,  
	// and  the listener is holding the Data line true. We're ready for step 1; we may send another character - unless EOI has 
	// happened. If EOI was sent or received in this last transmission, both talker and listener "letgo."  After a suitable pause, 
	// the Clock and Data lines are released to false and transmission stops. 

	if(flags bitand EOI_RECVD)
	{
		// EOI Received
		delayMicroseconds(TIMING_Tfr);
		// release(IEC_PIN_CLK);
		//release(IEC_PIN_DATA);
	}

	return data;
} // receiveByte


// STEP 1: READY TO SEND
// Sooner or later, the talker will want to talk, and send a character. 
// When it's ready to go, it releases the Clock line to false.  This signal change might be 
// translated as "I'm ready to send a character." The listener must detect this and respond, 
// but it doesn't have to do so immediately. The listener will respond  to  the  talker's  
// "ready  to  send"  signal  whenever  it  likes;  it  can  wait  a  long  time.    If  it's  
// a printer chugging out a line of print, or a disk drive with a formatting job in progress, 
// it might holdback for quite a while; there's no time limit. 
bool CBMStandardSerial::sendByte(uint8_t data, bool signalEOI)
{
	//flags = CLEAR;

	// Say we're ready
	delayMicroseconds(60);
	release(IEC_PIN_CLK);

	// Wait for listener to be ready
	// STEP 2: READY FOR DATA
	// When  the  listener  is  ready  to  listen,  it  releases  the  Data  
	// line  to  false.    Suppose  there  is  more  than one listener.  The Data line will go false 
	// only when all listeners have released it - in other words, when  all  listeners  are  ready  
	// to  accept  data.  What  happens  next  is  variable.
	while(status(IEC_PIN_DATA) != released)
	{
		ESP.wdtFeed();
	}

	// Either  the  talker  will pull the 
	// Clock line back to true in less than 200 microseconds - usually within 60 microseconds - or it  
	// will  do  nothing.    The  listener  should  be  watching,  and  if  200  microseconds  pass  
	// without  the Clock line going to true, it has a special task to perform: note EOI.
	if(signalEOI) {

		// INTERMISSION: EOI
		// If the Ready for Data signal isn't acknowledged by the talker within 200 microseconds, the 
		// listener knows  that  the  talker  is  trying  to  signal  EOI.    EOI,  which  formally  
		// stands  for  "End  of  Indicator," means  "this  character  will  be  the  last  one."    
		// If  it's  a  sequential  disk  file,  don't  ask  for  more:  there will be no more.  If it's 
		// a relative record, that's the end of the record.  The character itself will still be coming, but 
		// the listener should note: here comes the last character. So if the listener sees the 200 microsecond  
		// time-out,  it  must  signal  "OK,  I  noticed  the  EOI"  back  to  the  talker,  It  does  this  
		// by pulling  the  Data  line  true  for  at  least  60  microseconds,  and  then  releasing  it.  
		// The  talker  will  then revert to transmitting the character in the usual way; within 60 microseconds 
		// it will pull the Clock line  true,  and  transmission  will  continue.  At  this point,  the  Clock  
		// line  is  true  whether  or  not  we have gone through the EOI sequence; we're back to a common 
		// transmission sequence.

		//flags or_eq EOI_RECVD;

		// Signal eoi by waiting 200 us
		delayMicroseconds(TIMING_EOI_WAIT);

		// get eoi acknowledge:
		if(timeoutWait(IEC_PIN_DATA, pulled) == TIMED_OUT)
		{
			Debug_printv("Get EOI acknowledge");
			return false; // return error because timeout
		}

		if(timeoutWait(IEC_PIN_DATA, released) == TIMED_OUT)
		{
			Debug_printv("Listener didn't release DATA");
			return false; // return error because timeout
		}
	}
	else
	{
		delayMicroseconds(TIMING_Tne);		
	}

	// STEP 3: SENDING THE BITS
	// The talker has eight bits to send.  They will go out without handshake; in other words, 
	// the listener had better be there to catch them, since the talker won't wait to hear from the listener.  At this 
	// point, the talker controls both lines, Clock and Data.  At the beginning of the sequence, it is holding the 
	// Clock true, while the Data line is released to false.  the Data line will change soon, since we'll sendthe data 
	// over it. The eights bits will go out from the character one at a time, with the least significant bit going first.  
	// For example, if the character is the ASCII question mark, which is  written  in  binary  as  00011111,  the  ones  
	// will  go out  first,  followed  by  the  zeros.  Now,  for  each bit, we set the Data line true or false according 
	// to whether the bit is one or zero.  As soon as that'sset, the Clock line is released to false, signalling "data ready."  
	// The talker will typically have a bit in  place  and  be  signalling  ready  in  70  microseconds  or  less.  Once  
	// the  talker  has  signalled  "data ready," it will hold the two lines steady for at least 20 microseconds timing needs 
	// to be increased to 60  microseconds  if  the  Commodore  64  is  listening,  since  the  64's  video  chip  may  
	// interrupt  the processor for 42 microseconds at a time, and without the extra wait the 64 might completely miss a 
	// bit. The listener plays a passive role here; it sends nothing, and just watches.  As soon as it sees the Clock line 
	// false, it grabs the bit from the Data line and puts it away.  It then waits for the clock line to go true, in order 
	// to prepare for the next bit. When the talker figures the data has been held for a sufficient  length  of  time,  it  
	// pulls  the  Clock  line true  and  releases  the  Data  line  to  false.    Then  it starts to prepare the next bit.

	// Send bits
#if defined(ESP8266)
	ESP.wdtFeed();
#endif

	// tell listner to wait
	pull(IEC_PIN_CLK);
	delayMicroseconds(TIMING_Tv);

	for(uint8_t n = 0; n < 8; n++) 
	{
		// FIXME: Here check whether data pin is pulled, if so end (enter cleanup)!


		// set bit
		(data bitand 1) ? release(IEC_PIN_DATA) : pull(IEC_PIN_DATA);
		delayMicroseconds(TIMING_Tv);

		// tell listener bit is ready to read
		release(IEC_PIN_CLK);
		delayMicroseconds(TIMING_Tv);

		// if ATN is pulled, exit and cleanup
		if(status(IEC_PIN_ATN) == pulled)
		{	
			return false;
		}

		pull(IEC_PIN_CLK);
		delayMicroseconds(22);
		release(IEC_PIN_DATA);
		delayMicroseconds(14);

		data >>= 1; // get next bit		
	}

	// STEP 4: FRAME HANDSHAKE
	// After the eighth bit has been sent, it's the listener's turn to acknowledge.  At this moment, the Clock line  is  true  
	// and  the  Data  line  is  false.    The  listener  must  acknowledge  receiving  the  byte  OK  by pulling the Data 
	// line to true. The talker is now watching the Data line.  If the listener doesn't pull the  Data  line  true  within  
	// one  millisecond  -  one  thousand  microseconds  -  it  will  know  that something's wrong and may alarm appropriately.

	// Wait for listener to accept data
	// if(timeoutWait(IEC_PIN_DATA, pulled, 250) == TIMED_OUT)
	// {
	// 	Debug_printv("Wait for listener to acknowledge byte received");
	// 	return false; // return error because timeout
	// }

	uint8_t n = 0;
	while(status(IEC_PIN_ATN) != released && status(IEC_PIN_DATA) != pulled && (n < 100)) {
		delayMicroseconds(10);  // this loop should cycle in about 10 us...
		n++;
	}
	delayMicroseconds(TIMING_Tbb);

	// STEP 5: START OVER
	// We're  finished,  and  back  where  we  started.    The  talker  is  holding  the  Clock  line  true,  
	// and  the listener is holding the Data line true. We're ready for step 1; we may send another character - unless EOI has 
	// happened. If EOI was sent or received in this last transmission, both talker and listener "letgo."  After a suitable pause, 
	// the Clock and Data lines are released to false and transmission stops. 

	// if(signalEOI)
	// {
	// 	// EOI Received
	// 	delayMicroseconds(TIMING_Tfr);
	// 	release(IEC_PIN_CLK);
	// 	// release(IEC_PIN_DATA);
	// }

	return true;
} // sendByte


// Wait indefinitely if wait = 0
size_t CBMStandardSerial::timeoutWait(byte iecPIN, IECline lineStatus, size_t wait, size_t step)
{

#if defined(ESP8266)
	ESP.wdtFeed();
#endif

	size_t t = 0;
	if(wait == FOREVER)
	{
		while(status(iecPIN) != lineStatus) {
			ESP.wdtFeed();
			delayMicroseconds(step);
			t++;	
		}
		return (t * step);
	}
	else
	{
		while(status(iecPIN) != lineStatus && (t < wait)) {
			delayMicroseconds(step);
			t++;	
		}
		// Check the waiting condition:
		if(t < wait)
		{
			// Got it!  Continue!
			return (t * step);
		}		
	}

	Debug_printv("pin[%d] state[%d] wait[%d] step[%d] t[%d]", iecPIN, lineStatus, wait, step, t);
	return 0;
} // timeoutWait

