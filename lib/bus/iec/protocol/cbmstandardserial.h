// // Meatloaf - A Commodore 64/128 multi-device emulator
// // https://github.com/idolpx/meatloaf
// // Copyright(C) 2020 James Johnston
// //
// // Meatloaf is free software : you can redistribute it and/or modify
// // it under the terms of the GNU General Public License as published by
// // the Free Software Foundation, either version 3 of the License, or
// // (at your option) any later version.
// //
// // Meatloaf is distributed in the hope that it will be useful,
// // but WITHOUT ANY WARRANTY; without even the implied warranty of
// // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// // GNU General Public License for more details.
// //
// // You should have received a copy of the GNU General Public License
// // along with Meatloaf. If not, see <http://www.gnu.org/licenses/>.

// // https://www.pagetable.com/?p=1135
// // https://codebase64.org/doku.php?id=base:how_the_vic_64_serial_bus_works
// // http://www.zimmers.net/anonftp/pub/cbm/programming/serial-bus.pdf
// // https://github.com/0cjs/sedoc/blob/master/8bit/cbm/serial-bus.md


// #ifndef PROTOCOL_CBMSTANDARDSERIAL_H
// #define PROTOCOL_CBMSTANDARDSERIAL_H

// #include "../../../include/global_defines.h"

// #include "fnSystem.h"

// //#define set_pin_mode      fnSystem.set_pin_mode
// #define digital_write     fnSystem.digital_write
// #define digital_read      fnSystem.digital_read
// #define INPUT             gpio_mode_t::GPIO_MODE_INPUT
// #define OUTPUT            gpio_mode_t::GPIO_MODE_OUTPUT
// #define delayMicroseconds fnSystem.delay_microseconds

// // BIT Flags
// #define CLEAR            0x0000    // clear all flags
// #define CLEAR_LOW        0xFF00    // clear low byte
// #define ERROR            (1 << 0)  // if this flag is set, something went wrong
// #define ATN_PULLED       (1 << 1)  // might be set by iec_receive
// #define EOI_RECVD        (1 << 2)
// #define EMPTY_STREAM     (1 << 3)
// #define COMMAND_RECVD    (1 << 4)

// #define JIFFY_ACTIVE        (1 << 8)
// #define JIFFY_LOAD          (1 << 9)
// #define DOLPHIN_ACTIVE      (1 << 10)
// #define WIC64_ACTIVE        (1 << 11)
// #define MEATLOADER_ACTIVE   (1 << 12)

// // IEC protocol timing consts in microseconds (us)
// // IEC-Disected p10-11         // Description              //   1541    C64     min     typical     max         // Notes
// #define TIMEOUT_Tat    1000    // ATN RESPONSE (REQUIRED)                       -       -           1000us      (If maximum time exceeded, device not present error.)
// #define TIMING_Th      60      // LISTENER HOLD-OFF             65us    39us    0       -           infinte
// #define TIMING_Tne     40      // NON-EOI RESPONSE TO RFD                       -       40us        200us       (If maximum time exceeded, EOI response required.)
// #define TIMEOUT_Tne    250
// #define TIMING_Ts      70      // BIT SET-UP TALKER                     71us    20us    70us        -           
// #define TIMING_Ts1     57      // BIT SET-UP LISTENER PRE       57us    47us
// #define TIMING_Ts2     28      // BIT SET-UP LISTENER POST      28us    24us
// #define TIMING_Tv      76      // DATA VALID                    76us    26us    20us    20us        -           (Tv and Tpr minimum must be 60μ s for external device to be a talker. )
// #define TIMING_Tf      45      // FRAME HANDSHAKE                               0       20us        1000us      (If maximum time exceeded, frame error.)
// #define TIMEOUT_Tf     1000
// #define TIMING_Tr      20      // FRAME TO RELEASE OF ATN                       20us    -           -
// #define TIMING_Tbb     100     // BETWEEN BYTES TIME                            100us   -           -
// #define TIMING_Tye     250     // EOI RESPONSE TIME                             200us   250us       -
// #define TIMING_Tei     80      // EOI RESPONSE HOLD TIME                        60us    -           -           (Tei minimum must be 80μ s for external device to be a listener.)
// #define TIMING_Try     30      // TALKER RESPONSE LIMIT                         0       30us        60us
// #define TIMEOUT_Try    60
// #define TIMING_Tpr     60      // BYTE-ACKNOWLEDGE                              20us    30us        -           (Tv and Tpr minimum must be 60μ s for external device to be a talker.)
// #define TIMING_Ttk     20      // TALK-ATTENTION RELEASE        20us            20us    30us        100us
// #define TIMEOUT_Ttk    100
// #define TIMING_Tdc     20      // TALK-ATTENTION ACKNOWLEDGE    20us            0       -           -
// #define TIMING_Tda     80      // TALK-ATTENTION ACK. HOLD                      80us    -           -
// #define TIMING_Tfr     60      // EOI ACKNOWLEDGE                               60us    -           -

// #define TIMING_EMPTY   512     // SIGNAL EMPTY STREAM
// #define TIMING_STABLE  70      // WAIT FOR BUS TO BE STABLE

// #define TIMING_JIFFY_DETECT   218  // JIFFYDOS ENABLED DELAY ON LAST BIT
// #define TIMING_JIFFY_ACK      101  // JIFFYDOS ACK RESPONSE

// // See timeoutWait
// #define TIMEOUT_DEFAULT 1000 // 1ms
// #define TIMED_OUT -1
// #define FOREVER 0

// #ifndef IEC_INVERTED_LINES
// // Not Inverted
// #define PULLED    true
// #define RELEASED  false
// #define LOW 0x00
// #define HIGH 0x01
// #else
// // Inverted
// #define PULLED    false
// #define RELEASED  true
// #define LOW 0x01
// #define HIGH 0x00
// #endif

// namespace Protocol
// {
//     class CBMStandardSerial
//     {
//         private:
//             virtual int16_t IRAM_ATTR receiveBits ();
//             virtual bool IRAM_ATTR sendBits ( uint8_t data );

//         public:
//             // communication must be reset
//             uint16_t flags = CLEAR;
//             uint32_t enabledDevices;

//             virtual int16_t receiveByte ();
//             virtual bool sendByte ( uint8_t data, bool signalEOI );
//             int16_t timeoutWait ( uint8_t pin, bool target_status, size_t wait = TIMEOUT_DEFAULT, bool watch_atn = true );
//             bool wait ( size_t wait, uint64_t size = 0 );


//             // true => PULL => LOW
//             inline void IRAM_ATTR pull ( uint8_t pin )
//             {
// #ifndef IEC_SPLIT_LINES
//                 set_pin_mode ( pin, OUTPUT );
// #endif
//                 digital_write ( pin, LOW );
//             }

//             // false => RELEASE => HIGH
//             inline void IRAM_ATTR release ( uint8_t pin )
//             {
// #ifndef IEC_SPLIT_LINES
//                 set_pin_mode ( pin, OUTPUT );
// #endif
//                 digital_write ( pin, HIGH );
//             }

//             inline bool IRAM_ATTR status ( uint8_t pin )
//             {
// #ifndef IEC_SPLIT_LINES
//                 set_pin_mode ( pin, INPUT );
// #endif
//                 return gpio_get_level ( ( gpio_num_t ) pin ) ? RELEASED : PULLED;
//             }

//             inline void IRAM_ATTR set_pin_mode ( uint8_t pin, gpio_mode_t mode )
//             {
//                 static uint64_t gpio_pin_modes;
//                 uint8_t b_mode = ( mode == 1 ) ? 1 : 0;

//                 // is this pin mode already set the way we want?
// #ifndef IEC_SPLIT_LINES
//                 if ( ( ( gpio_pin_modes >> pin ) & 1ULL ) != b_mode )
// #endif
//                 {
//                     // toggle bit so we don't change mode unnecessarily
//                     gpio_pin_modes ^= ( -b_mode ^ gpio_pin_modes ) & ( 1ULL << pin );

//                     gpio_config_t io_conf =
//                     {
//                         .pin_bit_mask = ( 1ULL << pin ),            // bit mask of the pins that you want to set
//                         .mode = mode,                               // set as input mode
//                         .pull_up_en = GPIO_PULLUP_DISABLE,          // disable pull-up mode
//                         .pull_down_en = GPIO_PULLDOWN_DISABLE,      // disable pull-down mode
//                         .intr_type = GPIO_INTR_DISABLE              // interrupt of falling edge
//                     };
//                     //configure GPIO with the given settings
//                     gpio_config ( &io_conf );
//                 }
//             }

//     };
// };

// #endif
