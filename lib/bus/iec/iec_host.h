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

// /*
// cbmctrl version 0.4.99.104, built on Jan 24 2022 at 10:52:39

// control serial CBM devices

//  Synopsis:  cbmctrl  [global_options] [action] [action_opt] [--] [action_args]

//  Global options:

//    -h, --help:    Output this help screen if specified without an action.
//                   Outputs some help information about a specific action
//                   if an action is specified.
//    -V, --version: Output version information
//    -@, --adapter: Tell OpenCBM which backend plugin and bus to use. This option
//                   requires an argument of the form <plugin>[:<bus>].
//                   <plugin> is the backend plugin's name to use (e.g.: xa1541)
//                   <bus>    is a bus unit identifier, if supported by the backend;
//                            look up the backend's documentation for the supported
//                            bus unit identifier(s) and the format for <bus>
//    -p, --petscii: Convert input or output parameter from CBM format (PETSCII)
//                   into PC format (ASCII). Default with all actions but 'open'
//                   and 'command'
//    -r, --raw:     Do not convert data between CBM and PC format.
//                   Default with 'open' and 'command'.
//    --             Delimiter between action_opt and action_args; if any of the
//                   arguments in action_args starts with a '-', make sure to set
//                   the '--' so the argument is not treated as an option,
//                   accidentially.

//  action: one of:

//   lock      Lock the parallel port for the use by cbm4win/cbm4linux.
//   unlock    Unlock the parallel port for the use by cbm4win/cbm4linux.
//   listen    perform a listen on the IEC bus
//   talk      perform a talk on the IEC bus
//   unlisten  perform an unlisten on the IEC bus
//   untalk    perform an untalk on the IEC bus
//   open      perform an open on the IEC bus
//   popen     deprecated; use 'cbmctrl --petscii open' instead!
//   close     perform a close on the IEC bus
//   read      read raw data from the IEC bus
//   write     write raw data to the IEC bus
//   put       put specified data to the IEC bus
//   status    give the status of the specified drive
//   command   issue a command to the specified drive
//   pcommand  deprecated; use 'cbmctrl --petscii command' instead!
//   dir       output the directory of the disk in the specified drive
//   download  download memory contents from the floppy drive
//   upload    upload memory contents to the floppy drive
//   srq       Set the srq line on the IEC bus.
//   usrq      Unset the srq line on the IEC bus.
//   clk       Set the clk line on the IEC bus.
//   uclk      Unset the clk line on the IEC bus.
//   data      Set the data line on the IEC bus.
//   udata     Unset the data line on the IEC bus.
//   atn       Set the atn line on the IEC bus.
//   uatn      Unset the atn line on the IEC bus.
//   ireset    Set the reset line on the IEC bus.
//   uireset   Unset the reset line on the IEC bus.
//   reset     reset all drives on the IEC bus
//   detect    detect drives on the IEC bus

//   change    wait for a disk to be changed in the specified drive

//   */

// #ifndef MEATLOAF_BUS_IECHOST
// #define MEATLOAF_BUS_IECHOST

// //#include <Arduino.h>

// #include "../../include/global_defines.h"
// #include "../../include/cbmdefines.h"
// #include "../../include/petscii.h"

// #include "iec.h"

// using namespace Protocol;

// class iecHost: public systemBus
// {
// private:

// public:
//     bool deviceExists(uint8_t deviceID);
//     void getStatus(uint8_t deviceID);
//     void dumpSector(uint8_t sector);
//     void dumpTrack(uint8_t track);
// };

// extern iecHost IECHOST;

// #endif // IECHOST_H