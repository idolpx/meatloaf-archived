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

#include "jiffydos.h"

#include "bus.h"
#include "iecProtocolBase.h"

#include "../../../include/debug.h"
#include "../../../include/pinmap.h"

using namespace Protocol;

// STEP 1: READY TO RECEIVE
// Sooner or later, the talker will want to talk, and send a character.
// When it's ready to go, it releases the Clock line to false.  This signal change might be
// translated as "I'm ready to send a character." The listener must detect this and 
// immediately start receiving data on both the clock and data lines.
int16_t  JiffyDOS::receiveByte ()
{
    IEC.flags and_eq CLEAR_LOW;

    IEC.pull ( PIN_IEC_SRQ );

    // Sometimes the C64 pulls ATN but doesn't pull CLOCK right away
    // pull ( PIN_IEC_SRQ );
    // if ( !wait ( 60 ) ) return -1;
    // release ( PIN_IEC_SRQ );

    // Release the Data line to signal we are ready
#ifndef IEC_SPLIT_LINES
    IEC.release(PIN_IEC_DATA_IN);
#endif

    // Wait for talker ready
    if ( timeoutWait ( PIN_IEC_CLK_IN, RELEASED, FOREVER ) == TIMED_OUT )
    {
        Debug_printv ( "Wait for talker ready" );
        IEC.flags or_eq ERROR;
        return -1; // return error because timeout
    }

    // STEP 2: RECEIVING THE BITS
    // As soon as the talker releases the Clock line we are expected to receive the bits
    uint8_t data = 0;
    uint8_t bitmask = 0xFF;

    // get bits 4,5
    data >>= 1; if ( gpio_get_level ( PIN_IEC_CLK_IN ) ) data |= 0x80;
    data >>= 1; if ( gpio_get_level ( PIN_IEC_DATA_IN ) ) data |= 0x80;
    wait( 8 );

    // get bits 6,7
    data >>= 1; if ( gpio_get_level ( PIN_IEC_CLK_IN ) ) data |= 0x80;
    data >>= 1; if ( gpio_get_level ( PIN_IEC_DATA_IN ) ) data |= 0x80;
    wait( 8 );

    // get bits 3,1
    data >>= 1; if ( gpio_get_level ( PIN_IEC_CLK_IN ) ) data |= 0x80;
    data >>= 1; if ( gpio_get_level ( PIN_IEC_DATA_IN ) ) data |= 0x80;
    wait( 8 );

    // get bits 2,0
    data >>= 1; if ( gpio_get_level ( PIN_IEC_CLK_IN ) ) data |= 0x80;
    data >>= 1; if ( gpio_get_level ( PIN_IEC_DATA_IN ) ) data |= 0x80;
    wait( 8 );
    IEC.release( PIN_IEC_SRQ );

    // rearrange bits
    data xor_eq bitmask;

    // STEP 3: CHECK FOR EOI
    if ( IEC.status ( PIN_IEC_CLK_IN ) == PULLED && IEC.status ( PIN_IEC_DATA_IN ) == RELEASED )
        IEC.flags or_eq EOI_RECVD;
    wait( 300 );

    // STEP 4: Acknowledge byte received
    IEC.pull ( PIN_IEC_DATA_OUT );

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
bool JiffyDOS::sendByte ( uint8_t data, bool signalEOI )
{
    IEC.flags and_eq CLEAR_LOW;

    // Initial handshake
    IEC.pull( PIN_IEC_CLK_OUT );
    IEC.pull( PIN_IEC_DATA_OUT );
    wait ( 3 );

//   if (loadmode) {
//     /* LOAD mode: start marker is data low */
//     while (!IEC_DATA) ; // wait until data actually is high again
//     llfl_wait_data(0, ATNABORT);
//   } else {
//     /* single byte mode: start marker is data high */
//     llfl_wait_data(1, ATNABORT);
//   }



    for ( uint8_t n = 0; n < 8; n++ )
    {
    
    #ifdef SPLIT_LINES
        // If data pin is PULLED, exit and cleanup
        if ( status ( PIN_IEC_DATA_IN ) == PULLED ) return false;
    #endif

        // set bit
        ( data bitand 1 ) ? IEC.release ( PIN_IEC_DATA_OUT ) : IEC.pull ( PIN_IEC_DATA_OUT );
        data >>= 1; // get next bit
        if ( !wait ( TIMING_Ts ) ) return false;

        // // Release data line after bit sent
        // release ( PIN_IEC_DATA_OUT );

        // tell listener bit is ready to read
        IEC.release ( PIN_IEC_CLK_OUT );
        if ( !wait ( TIMING_Tv ) ) return false;

        // tell listner to wait
        IEC.pull ( PIN_IEC_CLK_OUT );
    }
    // Release data line after byte sent
    IEC.release ( PIN_IEC_DATA_OUT );


    // STEP 4: FRAME HANDSHAKE
    // After the eighth bit has been sent, it's the listener's turn to acknowledge.  At this moment, the Clock line  is  true
    // and  the  Data  line  is  false.    The  listener  must  acknowledge  receiving  the  byte  OK  by pulling the Data
    // line to true. The talker is now watching the Data line.  If the listener doesn't pull the  Data  line  true  within
    // one  millisecond  -  one  thousand  microseconds  -  it  will  know  that something's wrong and may alarm appropriately.

    // Wait for listener to accept data
    if ( timeoutWait ( PIN_IEC_DATA_IN, PULLED, TIMEOUT_Tf ) >= TIMEOUT_Tf )
    {
        Debug_printv ( "Wait for listener to acknowledge byte received" );
        return false; // return error because timeout
    }

    // STEP 5: START OVER
    // We're  finished,  and  back  where  we  started.    The  talker  is  holding  the  Clock  line  true,
    // and  the listener is holding the Data line true. We're ready for step 1; we may send another character - unless EOI has
    // happened. If EOI was sent or received in this last transmission, both talker and listener "letgo."  After a suitable pause,
    // the Clock and Data lines are RELEASED to false and transmission stops.

    if ( signalEOI )
    {
        // EOI Received
        if ( !wait ( TIMING_Tfr ) ) return false;
        IEC.release ( PIN_IEC_CLK_OUT );
    }
    // else
    // {
    //     wait ( TIMING_Tbb );
    // }

    return true;
} // sendByte
