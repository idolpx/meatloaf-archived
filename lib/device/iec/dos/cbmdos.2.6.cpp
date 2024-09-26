/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007-2013  Ingo Korb <ingo@akana.de>

   Inspired by MMC2IEC by Lars Pontoppidan et al.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   doscmd.c: Command channel parser

*/

#include "cbmdos.2.6.h"

// #include <ctype.h>
// #include <stdlib.h>
// #include <string.h>
// #include "config.h"
// #include "crc.h"
// #include "d64ops.h"
// #include "dirent.h"
// #include "diskchange.h"
// #include "diskio.h"
// #include "display.h"
// #include "eeprom.h"
// #include "errormsg.h"
// #include "fastloader.h"
// #include "fastloader-ll.h"
// #include "fatops.h"
// #include "ff.h"
// #include "flags.h"
// #include "iec.h"
// #include "led.h"
// #include "parser.h"
// #include "system.h"
// #include "time.h"
// #include "rtc.h"
// #include "uart.h"
// #include "ustring.h"
// #include "utils.h"
// #include "wrapops.h"
// #include "doscmd.h"

// #define CURSOR_RIGHT 0x1d

// static FIL romfile;

// /* ---- Fastloader tables ---- */

// enum {
//   RXTX_NONE,
//   RXTX_GEOS_1MHZ,
//   RXTX_GEOS_2MHZ,
//   RXTX_GEOS_1581_21,
//   RXTX_WHEELS_1MHZ,
//   RXTX_WHEELS_2MHZ,
//   RXTX_WHEELS44_1541,
//   RXTX_WHEELS44_1581,
// };

// typedef uint8_t (*fastloader_rx_t)(void);
// typedef void    (*fastloader_tx_t)(uint8_t byte);
// typedef void    (*fastloader_handler_t)(uint8_t param);

// struct fastloader_rxtx_s {
//   fastloader_rx_t rxfunc;
//   fastloader_tx_t txfunc;
// };

// static const PROGMEM struct fastloader_rxtx_s fl_rxtx_table[] = {
// #ifdef CONFIG_LOADER_GEOS
//   [RXTX_GEOS_1MHZ]     = { geos_get_byte_1mhz,     geos_send_byte_1mhz     },
//   [RXTX_GEOS_2MHZ]     = { geos_get_byte_2mhz,     geos_send_byte_2mhz     },
//   [RXTX_GEOS_1581_21]  = { geos_get_byte_2mhz,     geos_send_byte_1581_21  },
// # ifdef CONFIG_LOADER_WHEELS
//   [RXTX_WHEELS_1MHZ]   = { wheels_get_byte_1mhz,   wheels_send_byte_1mhz   },
//   [RXTX_WHEELS_2MHZ]   = { geos_get_byte_2mhz,     geos_send_byte_1581_21  },
//   [RXTX_WHEELS44_1541] = { wheels44_get_byte_1mhz, wheels_send_byte_1mhz   },
//   [RXTX_WHEELS44_1581] = { wheels44_get_byte_2mhz, wheels44_send_byte_2mhz },
// # endif
// #endif
// };

// struct fastloader_crc_s {
//   uint16_t crc;
//   uint8_t  loadertype;
//   uint8_t  rxtx;
// };

// static const PROGMEM struct fastloader_crc_s fl_crc_table[] = {
// #ifdef CONFIG_LOADER_TURBODISK
//   { 0x9c9f, FL_TURBODISK,        RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_FC3
//   { 0xdab0, FL_FC3_LOAD,         RXTX_NONE          }, // Final Cartridge III
//   { 0x973b, FL_FC3_LOAD,         RXTX_NONE          }, // Final Cartridge III variation
//   { 0x7e38, FL_FC3_LOAD,         RXTX_NONE          }, // EXOS v3
//   { 0x1b30, FL_FC3_SAVE,         RXTX_NONE          }, // note: really early CRC; lots of C64 code at the end
//   { 0x8b0e, FL_FC3_SAVE,         RXTX_NONE          }, // variation
//   { 0x9930, FL_FC3_FREEZED,      RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_DREAMLOAD
//   { 0x2e69, FL_DREAMLOAD,        RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_ULOAD3
//   { 0xdd81, FL_ULOAD3,           RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_ELOAD1
//   { 0x393e, FL_ELOAD1,           RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_EPYXCART
//   { 0x5a01, FL_EPYXCART,         RXTX_NONE          },
// #endif
// #ifdef CONFIG_LOADER_GEOS
//   { 0x4d79, FL_GEOS_S23_1541,    RXTX_GEOS_1MHZ     }, // GEOS 64 1541 stage 2
//   { 0xb2bc, FL_GEOS_S23_1541,    RXTX_GEOS_1MHZ     }, // GEOS 128 1541 stage 2
//   { 0xb272, FL_GEOS_S23_1541,    RXTX_GEOS_1MHZ     }, // GEOS 64/128 1541 stage 3 (Configure)
//   { 0xdaed, FL_GEOS_S23_1571,    RXTX_GEOS_2MHZ     }, // GEOS 64/128 1571 stage 3 (Configure)
//   { 0x3f8d, FL_GEOS_S23_1581,    RXTX_GEOS_2MHZ     }, // GEOS 64/128 1581 Configure 2.0
//   { 0xc947, FL_GEOS_S23_1581,    RXTX_GEOS_1581_21  }, // GEOS 64/128 1581 Configure 2.1
// # ifdef CONFIG_LOADER_WHEELS
//   { 0xf140, FL_WHEELS_S1_64,     RXTX_WHEELS_1MHZ   }, // Wheels 64 stage 1
//   { 0x737e, FL_WHEELS_S1_128,    RXTX_WHEELS_1MHZ   }, // Wheels 128 stage 1
//   { 0x755a, FL_WHEELS_S2,        RXTX_WHEELS_1MHZ   }, // Wheels 64 1541 stage 2
//   { 0x2920, FL_WHEELS_S2,        RXTX_WHEELS_1MHZ   }, // Wheels 128 1541 stage 2
//   { 0x18e9, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 1571
//   { 0x9804, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 1581
//   { 0x48f5, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 FD native partition
//   { 0x1356, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 FD emulation partition
//   { 0xe885, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 HD native partition
//   { 0x4eca, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 64 HD emulation partition
//   { 0xdbf6, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 1571
//   { 0xe4ab, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 1581
//   { 0x6de5, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 FD native
//   { 0x30ff, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 FD emulation
//   { 0x46e7, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 HD native
//   { 0x2253, FL_WHEELS_S2,        RXTX_WHEELS_2MHZ   }, // Wheels 128 HD emulation
//   { 0xc26a, FL_WHEELS44_S2,      RXTX_WHEELS44_1541 }, // Wheels 64/128 4.4 1541
//   { 0x550c, FL_WHEELS44_S2,      RXTX_WHEELS44_1541 }, // Wheels 64/128 4.4 1571
//   { 0x825b, FL_WHEELS44_S2_1581, RXTX_WHEELS44_1581 }, // Wheels 64/128 4.4 1581
//   { 0x245b, FL_WHEELS44_S2_1581, RXTX_WHEELS44_1581 }, // Wheels 64/128 4.4 1581
//   { 0x7021, FL_WHEELS44_S2_1581, RXTX_WHEELS44_1581 }, // Wheels 64/128 4.4 1581
//   { 0xd537, FL_WHEELS44_S2_1581, RXTX_WHEELS44_1581 }, // Wheels 64/128 4.4 1581
//   { 0xf635, FL_WHEELS44_S2_1581, RXTX_WHEELS44_1581 }, // Wheels 64/128 4.4 1581
// # endif
// #endif
// #ifdef CONFIG_LOADER_NIPPON
//   { 0x43c1, FL_NIPPON,           RXTX_NONE          }, // Nippon
// #endif
// #ifdef CONFIG_LOADER_AR6
//   { 0x4870, FL_AR6_1581_LOAD,    RXTX_NONE          },
//   { 0x2925, FL_AR6_1581_SAVE,    RXTX_NONE          },
// #endif

//   { 0, FL_NONE, 0 }, // end marker
// };

// struct fastloader_handler_s {
//   uint16_t             address;
//   uint8_t              loadertype;
//   fastloader_handler_t handler;
//   uint8_t              parameter;
// };

// static const PROGMEM struct fastloader_handler_s fl_handler_table[] = {
// #ifdef CONFIG_LOADER_TURBODISK
//   { 0x0303, FL_TURBODISK,        load_turbodisk, 0 },
// #endif
// #ifdef CONFIG_LOADER_FC3
//   { 0x059a, FL_FC3_LOAD,         load_fc3,       0 }, // FC3
//   { 0x0400, FL_FC3_LOAD,         load_fc3,       0 }, // EXOS
//   { 0x059c, FL_FC3_SAVE,         save_fc3,       0 },
//   { 0x059a, FL_FC3_SAVE,         save_fc3,       0 }, // variation
//   { 0x0403, FL_FC3_FREEZED,      load_fc3,       1 },
// #endif
// #ifdef CONFIG_LOADER_DREAMLOAD
//   { 0x0700, FL_DREAMLOAD,        load_dreamload, 0 },
// #endif
// #ifdef CONFIG_LOADER_ULOAD3
//   { 0x0336, FL_ULOAD3,           load_uload3,    0 },
// #endif
// #ifdef CONFIG_LOADER_ELOAD1
//   { 0x0300, FL_ELOAD1,           load_eload1,    0 },
// #endif
// #ifdef CONFIG_LOADER_GIJOE
//   { 0x0500, FL_GI_JOE,           load_gijoe,     0 },
// #endif
// #ifdef CONFIG_LOADER_EPYXCART
//   { 0x01a9, FL_EPYXCART,         load_epyxcart,  0 },
// #endif
// #ifdef CONFIG_LOADER_GEOS
//   { 0x0457, FL_GEOS_S1,          load_geos_s1,   0 },
//   { 0x0470, FL_GEOS_S1,          load_geos_s1,   1 },
//   { 0x03e2, FL_GEOS_S23_1541,    load_geos,      0 },
//   { 0x03dc, FL_GEOS_S23_1541,    load_geos,      0 },
//   { 0x03ff, FL_GEOS_S23_1571,    load_geos,      0 },
//   { 0x040f, FL_GEOS_S23_1581,    load_geos,      0 },
// # ifdef CONFIG_LOADER_WHEELS
//   { 0x0400, FL_WHEELS_S1_64,     load_wheels_s1, 0 },
//   { 0x0400, FL_WHEELS_S1_128,    load_wheels_s1, 1 },
//   { 0x0300, FL_WHEELS_S2,        load_wheels_s2, 0 },
//   { 0x0400, FL_WHEELS44_S2,      load_wheels_s2, 0 },
//   { 0x0300, FL_WHEELS44_S2_1581, load_wheels_s2, 0 },
//   { 0x0500, FL_WHEELS44_S2_1581, load_wheels_s2, 0 },
// # endif
// #endif
// #ifdef CONFIG_LOADER_NIPPON
//   { 0x0300, FL_NIPPON,           load_nippon,    0 },
// #endif
// #ifdef CONFIG_LOADER_AR6
//   { 0x0500, FL_AR6_1581_LOAD,    load_ar6_1581,  0 },
//   { 0x05f4, FL_AR6_1581_SAVE,    save_ar6_1581,  0 },
// #endif

//   { 0, FL_NONE, NULL, 0 }, // end marker
// };

// /* ---- Minimal drive rom emulation ---- */

// typedef struct magic_value_s {
//   uint16_t address;
//   uint8_t  val[2];
// } magic_value_t;

// /* These are address/value pairs used by some programs to detect a 1541. */
// /* Currently we remember two bytes per address since that's the longest  */
// /* block required. */
// static const PROGMEM magic_value_t c1541_magics[] = {
//   { 0xfea0, { 0x0d, 0xed } }, /* used by DreamLoad and ULoad Model 3 */
//   { 0xe5c6, { 0x34, 0xb1 } }, /* used by DreamLoad and ULoad Model 3 */
//   { 0xfffe, { 0x00, 0x00 } }, /* Disable AR6 fastloader */
//   { 0,      { 0, 0 } }        /* end mark */
// };

// /* System partition G-P answer */
// static const PROGMEM uint8_t system_partition_info[] = {
//   0xff,0xe2,0x00,0x53,0x59,0x53,0x54,0x45,
//   0x4d,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,
//   0xa0,0xa0,0xa0,0x00,0x00,0x00,0x00,0x00,
//   0x00,0x00,0x00,0x00,0x00,0x00,0x0d
// };

// #ifndef __AVR__
// /* AVR uses GPIOR for this */
// uint8_t globalflags;
// #endif

// uint8_t command_buffer[CONFIG_COMMAND_BUFFER_SIZE+2];
// uint8_t command_length,original_length;

// date_t date_match_start;
// date_t date_match_end;

// uint16_t datacrc = 0xffff;
// static uint8_t previous_loader;

// #ifdef CONFIG_STACK_TRACKING
// //FIXME: AVR-only code
// uint16_t minstack = RAMEND;

// void __cyg_profile_func_enter (void *this_fn, void *call_site) __attribute__((no_instrument_function));
// void __cyg_profile_func_exit  (void *this_fn, void *call_site) __attribute__((alias("__cyg_profile_func_enter")));

// void __cyg_profile_func_enter (void *this_fn, void *call_site) {
//   if (SP < minstack) minstack = SP;
// }
// #endif

// #ifdef HAVE_RTC
// /* Days of the week as used by the CMD FD */
// static const PROGMEM uint8_t downames[] = "SUN.MON.TUESWED.THURFRI.SAT.";

// /* Skeleton of the ASCII time format */
// static const PROGMEM uint8_t asciitime_skel[] = " xx/xx/xx xx:xx:xx xM\r";
// #endif

// #ifdef CONFIG_CAPTURE_LOADERS
// static uint8_t loader_buffer[CONFIG_CAPTURE_BUFFER_SIZE];
// static uint8_t *loader_ptr = loader_buffer;
// static uint8_t capture_count = 0;

// /* ------------------------------------------------------------------------- */
// /*  Capture helpers                                                          */
// /* ------------------------------------------------------------------------- */

// /* Convert byte to two-character hex string */
// static uint8_t *byte_to_hex(uint8_t num, uint8_t *str) {
//   uint8_t tmp;

//   tmp = (num & 0xf0) >> 4;
//   if (tmp < 10)
//     *str++ = '0' + tmp;
//   else
//     *str++ = 'a' + tmp - 10;

//   tmp = num & 0x0f;
//   if (tmp < 10)
//     *str++ = '0' + tmp;
//   else
//     *str++ = 'a' + tmp - 10;

//   return str;
// }

// /* Copy command to capture buffer */
// static void dump_command(void) {
//   if (loader_ptr - loader_buffer + command_length + 2+1 < CONFIG_CAPTURE_BUFFER_SIZE) {
//     *loader_ptr++ = 'C';
//     *loader_ptr++ = command_length;
//     memcpy(loader_ptr, command_buffer, command_length);
//     loader_ptr += command_length;
//   } else {
//     *loader_ptr = 'X';
//   }
// }

// /* Dump buffer state */
// static void dump_buffer_state(void) {
//   if (CONFIG_CAPTURE_BUFFER_SIZE - (loader_ptr - loader_buffer) > sizeof(buffer_t)*CONFIG_BUFFER_COUNT+2) {
//     *loader_ptr++ = 'B';
//     *loader_ptr++ = sizeof(buffer_t);
//     *loader_ptr++ = CONFIG_BUFFER_COUNT;
//     memcpy(loader_ptr, buffers, sizeof(buffer_t) * CONFIG_BUFFER_COUNT);
//     loader_ptr += sizeof(buffer_t) * CONFIG_BUFFER_COUNT;
//   }
// }

// /* Save capture buffer to disk */
// static void save_capbuffer(void) {
//   uint8_t *ptr;
//   FIL datafile;

//   /* Use CRC and counter as file name */
//   ptr = byte_to_hex(datacrc >> 8, entrybuf);
//   ptr = byte_to_hex(datacrc & 0xff, ptr);
//   *ptr++ = '-';
//   ptr = byte_to_hex(capture_count++, ptr);
//   *ptr++ = '.';
//   *ptr++ = 'd';
//   *ptr++ = 'm';
//   *ptr++ = 'p';
//   *ptr   = 0;

//   FRESULT res = f_open(&partition[0].fatfs, &datafile, entrybuf, FA_WRITE | FA_CREATE_ALWAYS);
//   if (res == FR_OK) {
//     UINT byteswritten;

//     /* Skip the error checks here */
//     f_write(&datafile, loader_buffer, loader_ptr - loader_buffer, &byteswritten);
//     f_close(&datafile);
//   }

//   /* Reset capture buffer */
//   loader_ptr = loader_buffer;

//   /* Notify the user */
//   set_error(ERROR_DRIVE_NOT_READY);
// }

// #endif

// /* ------------------------------------------------------------------------- */
// /*  Parsing helpers                                                          */
// /* ------------------------------------------------------------------------- */

// /* Fill the end of the command buffer with 0x00  */
// /* so C string functions can work on file names. */

// static void clean_cmdbuffer(void) {
//   memset(command_buffer+command_length, 0, sizeof(command_buffer)-command_length);
// }

// /* Parse parameters of block commands in the command buffer */
// /* Returns number of parameters (up to 4) or <0 on error    */
// static int8_t parse_blockparam(uint8_t values[]) {
//   uint8_t paramcount = 0;
//   uint8_t *str;

//   str = ustrchr(command_buffer, ':');
//   if (!str) {
//     if (ustrlen(command_buffer) < 3)
//       return -1;
//     str = command_buffer + 2;
//   }

//   str++;

//   while (*str && paramcount < 4) {
//     /* Skip all spaces, cursor-rights and commas - CC7C */
//     while (*str == ' ' || *str == CURSOR_RIGHT || *str == ',') str++;
//     if (!*str)
//       break;

//     values[paramcount++] = parse_number(&str);
//   }

//   return paramcount;
// }

// static uint8_t parse_bool(void) {
//   switch (command_buffer[2]) {
//   case '+':
//     return 1;

//   case '-':
//     return 0;

//   default:
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return 255;
//   }
// }

// /* ------------------------------------------------------------------------- */
// /*  Command handlers                                                         */
// /* ------------------------------------------------------------------------- */

// /* ------------------- */
// /*  CD/MD/RD commands  */
// /* ------------------- */

// /* --- MD --- */
// static void parse_mkdir(void) {
//   path_t  path;
//   uint8_t *name;

//   /* MD requires a colon */
//   if (!ustrchr(command_buffer, ':')) {
//     set_error(ERROR_SYNTAX_NONAME);
//     return;
//   }
//   if (parse_path(command_buffer+2, &path, &name, 0))
//     return;
//   mkdir(&path,name);
// }

// /* --- CD --- */
// static void parse_chdir(void) {
//   path_t  path;
//   uint8_t *name;
//   cbmdirent_t dent;

//   if (parse_path(command_buffer+2, &path, &name, 1))
//     return;

//   if (ustrlen(name) != 0) {
//     /* Path component after the : */
//     if (name[0] == '_') {
//       /* Going up a level */
//       ustrcpy(dent.name, name);
//       if (chdir(&path,&dent))
//         return;
//     } else {
//       /* A directory name - try to match it */
//       if (first_match(&path, name, FLAG_HIDDEN, &dent))
//         return;

//       if (chdir(&path, &dent))
//         return;
//     }
//   } else {
//     if (!ustrchr(command_buffer, '/')) {
//       set_error(ERROR_FILE_NOT_FOUND_39);
//       return;
//     }
//   }

//   update_current_dir(&path);

//   if (globalflags & AUTOSWAP_ACTIVE)
//     set_changelist(NULL, NULLSTRING);
// }

// /* --- RD --- */
// static void parse_rmdir(void) {
//   uint8_t *str;
//   uint8_t res;
//   uint8_t part;
//   path_t  path;
//   cbmdirent_t dent,chkdent;
//   dh_t dh;

//   /* No deletion across subdirectories */
//   if (ustrchr(command_buffer, '/')) {
//     set_error(ERROR_SYNTAX_NONAME);
//     return;
//   }

//   /* Parse partition number */
//   str = command_buffer+2;
//   part = parse_partition(&str);
//   if (*str != ':') {
//     set_error(ERROR_SYNTAX_NONAME);
//   } else {
//     path.part = part;
//     path.dir  = partition[part].current_dir;

//     if (first_match(&path, str+1, TYPE_DIR, &dent) != 0)
//       return;

//     /* Check if there is anything in that directory */
//     if (chdir(&path, &dent))
//       return;

//     if (opendir(&dh, &path))
//       return;

//     if (readdir(&dh, &chkdent) != -1) {
//       if (current_error == 0)
//         set_error(ERROR_FILE_EXISTS);
//       return;
//     }

//     path.dir = partition[part].current_dir;

//     res = file_delete(&path, &dent);
//     if (res != 255)
//       set_error_ts(ERROR_SCRATCHED,res,0);
//   }
// }

// /* --- CD/MD/RD subparser --- */
// static void parse_dircommand(void) {
//   clean_cmdbuffer();

//   switch (command_buffer[0]) {
//   case 'M':
//     parse_mkdir();
//     break;

//   case 'C':
//     parse_chdir();
//     break;

//   case 'R':
//     parse_rmdir();
//     break;

//   default:
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     break;
//   }
// }

// /* ------------ */
// /*  B commands  */
// /* ------------ */
// static void parse_block(void) {
//   uint8_t  *str;
//   buffer_t *buf;
//   uint8_t  params[4];
//   int8_t   pcount;

//   clean_cmdbuffer();

//   str = ustrchr(command_buffer, '-');
//   if (!str) {
//     set_error(ERROR_SYNTAX_UNABLE);
//     return;
//   }

//   memset(params,0,sizeof(params));
//   pcount = parse_blockparam(params);
//   if (pcount < 0)
//     return;

//   str++;
//   switch (*str) {
//   case 'R':
//   case 'W':
//     /* Block-Read  - CD56 */
//     /* Block-Write - CD73 */
//     buf = find_buffer(params[0]);
//     if (!buf) {
//       set_error(ERROR_NO_CHANNEL);
//       return;
//     }

//     /* Use current partition for 0 */
//     if (params[1] == 0)
//       params[1] = current_part;

//     if (*str == 'R') {
//       read_sector(buf,params[1],params[2],params[3]);
//       if (command_buffer[0] == 'B') {
//         buf->position = 1;
//         buf->lastused = buf->data[0];
//       } else {
//         buf->position = 0;
//         buf->lastused = 255;
//       }
//     } else {
//       if (command_buffer[0] == 'B')
//         buf->data[0] = buf->position-1; // FIXME: Untested, verify!
//       write_sector(buf,params[1],params[2],params[3]);
//     }
//     break;

//   case 'P':
//     /* Buffer-Position - CDBD */
//     buf = find_buffer(params[0]);
//     if (!buf) {
//       set_error(ERROR_NO_CHANNEL);
//       return;
//     }
//     if (buf->pvt.buffer.size != 1) {
//       /* Extended position for large buffers */
//       uint8_t count = params[2];

//       /* Walk the chain, wrap whenever necessary */
//       buf->secondary = BUFFER_SEC_CHAIN - params[0];
//       buf = buf->pvt.buffer.first;
//       while (count--) {
//         if (buf->pvt.buffer.next != NULL)
//           buf = buf->pvt.buffer.next;
//         else
//           buf = buf->pvt.buffer.first;
//       }
//       buf->secondary = params[0];
//       buf->mustflush = 0;
//     }
//     buf->position = params[1];
//     break;

//   default:
//     set_error(ERROR_SYNTAX_UNABLE);
//     return;
//   }
// }

// /* ---------- */
// /*  C - Copy  */
// /* ---------- */
// static void parse_copy(void) {
//   path_t srcpath,dstpath;
//   uint8_t *srcname,*dstname,*tmp;
//   uint8_t savedtype;
//   int8_t res;
//   buffer_t *srcbuf,*dstbuf;
//   cbmdirent_t dent;

//   clean_cmdbuffer();

//   /* Find the = */
//   srcname = ustrchr(command_buffer,'=');
//   if (srcname == NULL) {
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }
//   *srcname++ = 0;

//   /* Parse the destination name */
//   if (parse_path(command_buffer+1, &dstpath, &dstname, 0))
//     return;

//   if (ustrlen(dstname) == 0) {
//     set_error(ERROR_SYNTAX_NONAME);
//     return;
//   }

//   /* Check for invalid characters in the destination name */
//   if (check_invalid_name(dstname)) {
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }

//   /* Check if the destination file exists */
//   res = first_match(&dstpath, dstname, FLAG_HIDDEN, &dent);
//   if (res == 0) {
//     set_error(ERROR_FILE_EXISTS);
//     return;
//   }

//   if (res > 0)
//     return;

//   set_error(ERROR_OK);

//   srcbuf = alloc_buffer();
//   dstbuf = alloc_buffer();
//   if (srcbuf == NULL || dstbuf == NULL)
//     return;

//   savedtype = 0;
//   srcname = ustr1tok(srcname,',',&tmp);
//   while (srcname != NULL) {
//     /* Parse the source path */
//     if (parse_path(srcname, &srcpath, &srcname, 0))
//       goto cleanup;

//     /* Open the current source file */
//     res = first_match(&srcpath, srcname, FLAG_HIDDEN, &dent);
//     if (res != 0)
//       goto cleanup;

//     /* Note: A 1541 can't copy REL files. We try to do better. */
//     if ((dent.typeflags & TYPE_MASK) == TYPE_REL) {
//       if (savedtype != 0 && savedtype != TYPE_REL) {
//         set_error(ERROR_FILE_TYPE_MISMATCH);
//         goto cleanup;
//       }
//       open_rel(&srcpath, &dent, srcbuf, 0, 1);
//     } else {
//       if (savedtype != 0 && savedtype == TYPE_REL) {
//         set_error(ERROR_FILE_TYPE_MISMATCH);
//         goto cleanup;
//       }
//       open_read(&srcpath, &dent, srcbuf);
//     }

//     if (current_error != 0)
//       goto cleanup;

//     /* Open the destination file (first source only) */
//     if (savedtype == 0) {
//       savedtype = dent.typeflags & TYPE_MASK;
//       memset(&dent, 0, sizeof(dent));
//       ustrncpy(dent.name, dstname, CBM_NAME_LENGTH);
//       if (savedtype == TYPE_REL)
//         open_rel(&dstpath, &dent, dstbuf, srcbuf->recordlen, 1);
//       else
//         open_write(&dstpath, &dent, savedtype, dstbuf, 0);
//     }

//     while (1) {
//       uint8_t tocopy;

//       if (savedtype == TYPE_REL)
//         tocopy = srcbuf->recordlen;
//       else
//         tocopy = 256-dstbuf->position;

//       if (tocopy > (srcbuf->lastused - srcbuf->position+1))
//         tocopy = srcbuf->lastused - srcbuf->position + 1;

//       if (tocopy > 256-dstbuf->position)
//         tocopy = 256-dstbuf->position;

//       memcpy(dstbuf->data + dstbuf->position,
//              srcbuf->data + srcbuf->position,
//              tocopy);
//       mark_buffer_dirty(dstbuf);
//       srcbuf->position += tocopy-1;  /* add 1 less, simplifies the test later */
//       dstbuf->position += tocopy;
//       dstbuf->lastused  = dstbuf->position-1;

//       /* End if we just copied the last data block */
//       if (srcbuf->sendeoi && srcbuf->position == srcbuf->lastused)
//         break;

//       /* Refill the buffers if required */
//       if (srcbuf->recordlen || srcbuf->position++ == srcbuf->lastused)
//         if (srcbuf->refill(srcbuf))
//           goto cleanup;

//       if (dstbuf->recordlen || dstbuf->position == 0)
//         if (dstbuf->refill(dstbuf))
//           goto cleanup;
//     }

//     /* Close current source file */
//     /* Free and reallocate the buffer. This is required because most of the  */
//     /* file_open code assumes that it will get a "pristine" buffer with      */
//     /* 0 is most of the fields. Allocation cannot fail at this point because */
//     /* there is at least one free buffer.                                    */
//     cleanup_and_free_buffer(srcbuf);
//     srcbuf = alloc_buffer();

//     /* Next file */
//     srcname = ustr1tok(NULL,',',&tmp);
//   }

//   cleanup:
//   /* Close the buffers */
//   srcbuf->cleanup(srcbuf);
//   cleanup_and_free_buffer(dstbuf);
// }

// /* ----------------------- */
// /*  CP - Change Partition  */
// /* ----------------------- */
// static void parse_changepart(void) {
//   uint8_t *str;
//   uint8_t part;

//   if (command_buffer[1] == 'P') {
//     clean_cmdbuffer();
//     str  = command_buffer + 2;
//     part = parse_partition(&str);
//   } else {
//     /* Shift-P - binary version */
//     part = command_buffer[2] - 1;
//   }

//   if(part>=max_part) {
//     set_error_ts(ERROR_PARTITION_ILLEGAL,part+1,0);
//     return;
//   }

//   current_part = part;
//   if (globalflags & AUTOSWAP_ACTIVE)
//     set_changelist(NULL, NULLSTRING);

//   display_current_part(current_part);

//   set_error_ts(ERROR_PARTITION_SELECTED, part+1, 0);
// }

// /* ------------ */
// /*  D commands  */
// /* ------------ */
// static void parse_direct(void) {
//   buffer_t *buf;
//   uint8_t drive;
//   uint32_t sector;
//   DRESULT res;

//   /* This also guards against attempts to use the old Duplicate command. */
//   /* Its syntax is D1=0, so buffer[2] would never have a value that is   */
//   /* a valid secondary address.                                          */
//   buf = find_buffer(command_buffer[2]);
//   if (!buf) {
//     set_error(ERROR_NO_CHANNEL);
//     return;
//   }

//   if (buf->pvt.buffer.size > 1) {
//     uint8_t oldsec = buf->secondary;
//     buf->secondary = BUFFER_SEC_CHAIN - oldsec;
//     buf = buf->pvt.buffer.first;
//     buf->secondary = oldsec;
//   }

//   buf->position = 0;
//   buf->lastused = 255;

//   drive = command_buffer[3];
//   sector = *((uint32_t *)(command_buffer+4));

//   switch (command_buffer[1]) {
//   case 'I':
//     /* Get information */
//     memset(buf->data,0,256);
//     if (disk_getinfo(drive, command_buffer[4], buf->data) != RES_OK) {
//       set_error(ERROR_DRIVE_NOT_READY);
//       return;
//     }
//     break;

//   case 'R':
//     /* Read sector */
//     if (buf->pvt.buffer.size < 2) { // FIXME: Assumes 512-byte sectors
//       set_error(ERROR_BUFFER_TOO_SMALL);
//       return;
//     }
//     res = disk_read(drive, buf->data, sector, 1);
//     switch (res) {
//     case RES_OK:
//       return;

//     case RES_ERROR:
//       set_error(ERROR_READ_NOHEADER); // Any random READ ERROR
//       return;

//     case RES_PARERR:
//     case RES_NOTRDY:
//     default:
//       set_error_ts(ERROR_DRIVE_NOT_READY,res,0);
//       return;
//     }

//   case 'W':
//     /* Write sector */
//     if (buf->pvt.buffer.size < 2) { // FIXME: Assumes 512-byte sectors
//       set_error(ERROR_BUFFER_TOO_SMALL);
//       return;
//     }
//     res = disk_write(drive, buf->data, sector, 1);
//     switch(res) {
//     case RES_OK:
//       return;

//     case RES_WRPRT:
//       set_error(ERROR_WRITE_PROTECT);
//       return;

//     case RES_ERROR:
//       set_error(ERROR_WRITE_VERIFY);
//       return;

//     case RES_NOTRDY:
//     case RES_PARERR:
//     default:
//       set_error_ts(ERROR_DRIVE_NOT_READY,res,0);
//       return;
//     }

//   default:
//     set_error(ERROR_SYNTAX_UNABLE);
//     break;
//   }
// }

// /* -------------------------- */
// /*  G-P - Get Partition info  */
// /* -------------------------- */
// static void parse_getpartition(void) {
//   uint8_t *ptr;
//   uint8_t part;

//   if (command_length < 3) /* FIXME: should this set an error? */
//     return;

//   if (command_buffer[1] != '-' || command_buffer[2] != 'P') {
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }

//   if (command_length == 3 || command_buffer[3] == 0xff)
//     part = current_part + 1;
//   else
//     part = command_buffer[3];

//   buffers[CONFIG_BUFFER_COUNT].position = 0;
//   buffers[CONFIG_BUFFER_COUNT].lastused = 30;

//   memset(error_buffer,0,30);
//   error_buffer[30] = 13;
//   ptr = error_buffer;

//   if (part > max_part) {
//     /* Nonexisting partition - return empty answer */
//     ptr[30] = 13;
//     return;
//   }

//   if (part == 0) {
//     /* System partition - return static info */
//     memcpy_P(ptr, system_partition_info, sizeof(system_partition_info));
//     return;
//   }

//   part -= 1;

//   /* Create partition info */
//   if (partition[part].fop == &d64ops) {
//     /* Use type of mounted image as partition type */
//     *ptr++ = partition[part].imagetype & D64_TYPE_MASK;
//   } else {
//     /* Use native for anything else */
//     *ptr++ = 1;
//   }
//   *ptr++ = 0xe2; // 1.6MB disk - "reserved" for HD

//   *ptr++ = part+1;

//   /* Read partition label */
//   memset(ptr, 0xa0, 16);
//   if (disk_label(part, entrybuf)) {
//     return;
//   }

//   uint8_t *inptr  = entrybuf;
//   uint8_t *outptr = ptr;
//   while (*inptr)
//     *outptr++ = *inptr++;

//   ptr += 16;
//   *ptr++ = (partition[part].fatfs.fatbase >> 16) & 0xff;
//   *ptr++ = (partition[part].fatfs.fatbase >>  8) & 0xff;
//   *ptr++ = (partition[part].fatfs.fatbase      ) & 0xff;
//   ptr += 5; // reserved bytes

//   uint32_t size = (partition[part].fatfs.max_clust - 1) * partition[part].fatfs.csize;
//   *ptr++ = (size >> 16) & 0xff;
//   *ptr++ = (size >>  8) & 0xff;
//   *ptr   = size & 0xff;
// }

// /* ---------------- */
// /*  I - Initialize  */
// /* ---------------- */
// static void parse_initialize(void) {
//   if (disk_state != DISK_OK)
//     set_error_ts(ERROR_READ_NOSYNC,18,0);
//   else
//     free_multiple_buffers(FMB_USER_CLEAN);
// }

// /* ------------ */
// /*  M commands  */
// /* ------------ */

// /* --- M-E --- */
// static void handle_memexec(void) {
//   uint16_t address;

//   if (command_length < 5)
//     return;

//   if (detected_loader == FL_NONE) {
//     uart_puts_P(PSTR("M-E at "));
//     uart_puthex(command_buffer[4]);
//     uart_puthex(command_buffer[3]);
//     uart_puts_P(PSTR(", CRC "));
//     uart_puthex(datacrc >> 8);
//     uart_puthex(datacrc & 0xff);
//     uart_putcrlf();
//   }

//   if (detected_loader == FL_NONE)
//     detected_loader = previous_loader;

// #ifdef CONFIG_CAPTURE_LOADERS
//   if (detected_loader == FL_NONE && datacrc != 0xffff) {
//     dump_command();
//     dump_buffer_state();
//     save_capbuffer();
//   }
// #endif

//   /* Try to find a handler for loader */
//   const struct fastloader_handler_s *ptr = fl_handler_table;
//   uint8_t loader,parameter;
//   fastloader_handler_t handler;

//   address = command_buffer[3] + (command_buffer[4]<<8);
//   while ( (loader = pgm_read_byte(&ptr->loadertype)) != FL_NONE ) {
//     if (detected_loader == loader &&
//         address == pgm_read_word(&ptr->address)) {
//       /* Found it */
//       handler   = (fastloader_handler_t)pgm_read_word(&ptr->handler);
//       parameter = pgm_read_byte(&ptr->parameter);

//       /* Call */
//       handler(parameter);

//       break;
//     }
//     ptr++;
//   }

//   datacrc = 0xffff;
//   previous_loader = detected_loader;
//   detected_loader = FL_NONE;
// }

// /* --- M-R --- */
// static void handle_memread(void) {
//   FRESULT res;
//   uint16_t address, check;
//   magic_value_t *p;

//   if (command_length < 6)
//     return;

//   address = command_buffer[3] + (command_buffer[4]<<8);

//   if (address >= 0x8000 && rom_filename[0] != 0) {
//     /* Try to use the rom file as data source - look in the current dir first */
//     partition[current_part].fatfs.curr_dir = partition[current_part].current_dir.fat;
//     res = f_open(&partition[current_part].fatfs, &romfile, rom_filename, FA_READ | FA_OPEN_EXISTING);

//     if (res != FR_OK) {
//       /* Not successful, try root dir */
//       partition[current_part].fatfs.curr_dir = 0;
//       res = f_open(&partition[current_part].fatfs, &romfile, rom_filename, FA_READ | FA_OPEN_EXISTING);

//       if (res != FR_OK) {
//         /* Not successful, try root of drive 0 */
//         partition[0].fatfs.curr_dir = 0;
//         res = f_open(&partition[0].fatfs, &romfile, rom_filename, FA_READ | FA_OPEN_EXISTING);

//         if (res != FR_OK)
//           /* No file available - use internal table */
//           goto use_internal;
//       }
//     }

//     /* One of the f_open calls was successful */
//     address -= 0x8000;
//     if (romfile.fsize < 32*1024U)
//       /* Allow 16K 1541 roms */
//       address &= 0x3fff;

//     if ((romfile.fsize & 0x3fff) != 0)
//       /* Skip header bytes */
//       address += romfile.fsize & 0x3fff;

//     res = f_lseek(&romfile, address);
//     if (res != FR_OK)
//       goto use_internal;

//     /* Clamp maximum read length to buffer size */
//     uint8_t bytes = command_buffer[5];
//     if (bytes > sizeof(error_buffer))
//       bytes = sizeof(error_buffer);

//     UINT bytesread;
//     res = f_read(&romfile, error_buffer, bytes, &bytesread);
//     if (res != FR_OK || bytesread != bytes)
//       goto use_internal;

//     /* Note: f_close isn't neccessary in FatFs for read-only files */

//   } else {
//   use_internal:
//     /* Check some special addresses used for drive detection. */
//     p = (magic_value_t*) c1541_magics;
//     while ( (check = pgm_read_word(&p->address)) ) {
//       if (check == address) {
//         error_buffer[0] = pgm_read_byte(p->val);
//         error_buffer[1] = pgm_read_byte(p->val + 1);
//         break;
//       }
//       p++;
//     }
//   }

//   /* possibly the host wants to read more bytes than error_buffer size */
//   /* we ignore this knowing that we return nonsense in this case       */
//   buffers[CONFIG_BUFFER_COUNT].data     = error_buffer;
//   buffers[CONFIG_BUFFER_COUNT].position = 0;
//   buffers[CONFIG_BUFFER_COUNT].lastused = command_buffer[5]-1;
// }

// /* --- M-W --- */
// static void handle_memwrite(void) {
//   uint16_t address;
//   uint8_t  i;

//   if (command_length < 6)
//     return;

//   address = command_buffer[3] + (command_buffer[4]<<8);
//   //length  = command_buffer[5];

//   if (address == 119) {
//     /* Change device address, 1541 style */
//     device_address = command_buffer[6] & 0x1f;
//     display_address(device_address);
//     return;
//   }

//   if (address == 0x1c06 || address == 0x1c07) {
//     /* Ignore attempts to increase the VIA timer frequency */
//     return;
//   }

//   previous_loader = FL_NONE;

//   for (i=0;i<command_buffer[5];i++) {
//     datacrc = crc16_update(datacrc, command_buffer[i+6]);

// #ifdef CONFIG_LOADER_GIJOE
//     /* Identical code, but lots of different upload variations */
//     if (datacrc == 0x38a2 && command_buffer[i+6] == 0x60)
//       detected_loader = FL_GI_JOE;
// #endif
//   }

//   /* Figure out the fastloader based on the current CRC */
//   const struct fastloader_crc_s *crcptr = fl_crc_table;
//   uint8_t loader;

//   while ( (loader = pgm_read_byte(&crcptr->loadertype)) != FL_NONE ) {
//     if (datacrc == pgm_read_word(&crcptr->crc))
//       break;

//     crcptr++;
//   }

//   /* Set RX/TX function pointers */
//   if (loader != FL_NONE) {
//     uint8_t index;

//     detected_loader = loader;
//     index = pgm_read_word(&crcptr->rxtx);

// #ifdef CONFIG_LOADER_GEOS
//     if (index != 0) {
//       geos_get_byte  = (fastloader_rx_t)pgm_read_word(&(fl_rxtx_table[index].rxfunc));
//       geos_send_byte = (fastloader_tx_t)pgm_read_word(&(fl_rxtx_table[index].txfunc));
//     }
// #endif
//   }

// #ifdef CONFIG_LOADER_GEOS
//   /* Capture decryption key */

//   if (detected_loader == FL_GEOS_S1_KEY) {
//     /* Copy encryption key */
//     buffer_t *buf = find_buffer(BUFFER_SYS_GEOSKEY);

//     if (buf->position < 256-32) {
//       /* Middle blocks - copy completely */
//       memcpy(buf->data + buf->position, command_buffer + 6, 32);
//       buf->position += 32;
//     } else {
//       /* Last interesting block, also the last block of the loader */
//       memcpy(buf->data + buf->position, command_buffer + 6, 254-buf->position);

//       /* Same ID for both, the address can be used to distinguish them */
//       detected_loader = FL_GEOS_S1;
//       geos_send_byte = geos_send_byte_1mhz;
//     }
//   }

//   if (datacrc == 0xb979 || datacrc == 0x2469) {
//     /* GEOS stage 1 encryption key starts in this block */
//     buffer_t *buf = alloc_system_buffer();

//     if (buf == NULL)
//       return;

//     stick_buffer(buf);
//     buf->secondary = BUFFER_SYS_GEOSKEY;
//     if (datacrc == 0xb979) {
//       /* GEOS 64 1.3/2.0 */
//       buf->position = 32-10;
//       memcpy(buf->data, command_buffer + 6 + 10, 32-10);
//     } else { // datacrc == 0x2469
//       /* GEOS 128 2.0 */
//       buf->position = 32-15;
//       memcpy(buf->data, command_buffer + 6 + 15, 32-15);
//     }

//     detected_loader = FL_GEOS_S1_KEY;
//   }
// #endif

// #ifdef CONFIG_CAPTURE_LOADERS
//   dump_command();
// #endif

//   if (detected_loader == FL_NONE) {
//     uart_puts_P(PSTR("M-W CRC result: "));
//     uart_puthex(datacrc >> 8);
//     uart_puthex(datacrc & 0xff);
//     uart_putcrlf();
//   }
// }

// /* --- M subparser --- */
// static void parse_memory(void) {
//   if (command_buffer[2] == 'W')
//     handle_memwrite();
//   else if (command_buffer[2] == 'E')
//     handle_memexec();
//   else if (command_buffer[2] == 'R')
//     handle_memread();
//   else
//     set_error(ERROR_SYNTAX_UNKNOWN);
// }

// /* --------- */
// /*  N - New  */
// /* --------- */
// static void parse_new(void) {
//   uint8_t *name, *id;
//   uint8_t part;

//   clean_cmdbuffer();

//   name = command_buffer+1;
//   part = parse_partition(&name);
//   name = ustrchr(command_buffer, ':');
//   if (name++ == NULL) {
//     set_error(ERROR_SYNTAX_NONAME);
//     return;
//   }

//   id = ustrchr(name, ',');
//   if (id != NULL) {
//     *id = 0;
//     id++;
//   }

//   format(part, name, id);
// }

// /* -------------- */
// /*  P - Position  */
// /* -------------- */
// static void parse_position(void) {
//   buffer_t *buf;

//   command_length = original_length;
//   clean_cmdbuffer();

//   if(command_length < 2 || (buf = find_buffer(command_buffer[1] & 0x0f)) == NULL) {
//     set_error(ERROR_NO_CHANNEL);
//     return;
//   }

//   if (buf->seek == NULL) {
//     set_error(ERROR_SYNTAX_UNABLE);
//     return;
//   }

//   if (buf->recordlen) {
//     /* REL file */
//     uint16_t record;
//     uint8_t  pos;
//     record = 1;
//     pos = 1;

//     if (command_length > 1)
//       record = command_buffer[2];
//     if (command_length > 2)
//       record |= command_buffer[3] << 8;
//     if (command_length > 3)
//       pos = command_buffer[4];

//     if (pos > buf->recordlen) {
//       set_error(ERROR_RECORD_OVERFLOW);
//       return;
//     }

//     if (record)
//       record--;

//     if (pos)
//       pos--;

//     buf->seek(buf, record * (uint32_t)buf->recordlen, pos);
//   } else {
//     /* Non-REL seek uses a straight little-endian offset */
//     union {
//       uint32_t l;
//       uint8_t  c[4];
//     } offset;

//     // smaller than memcpy
//     /* WARNING: Endian-dependant */
//     offset.c[0] = command_buffer[2];
//     offset.c[1] = command_buffer[3];
//     offset.c[2] = command_buffer[4];
//     offset.c[3] = command_buffer[5];

//     buf->seek(buf, offset.l, 0);
//   }
// }

// /* ------------ */
// /*  R - Rename  */
// /* ------------ */
// static void parse_rename(void) {
//   path_t oldpath,newpath;
//   uint8_t *oldname,*newname;
//   cbmdirent_t dent;
//   int8_t res;

//   clean_cmdbuffer();

//   /* Find the boundary between the names */
//   oldname = ustrchr(command_buffer,'=');
//   if (oldname == NULL) {
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }
//   *oldname++ = 0;

//   /* Parse both names */
//   if (parse_path(command_buffer+1, &newpath, &newname, 0))
//     return;

//   if (parse_path(oldname, &oldpath, &oldname, 0))
//     return;

//   /* Rename can't move files across directories */
//   if (memcmp(&oldpath.dir, &newpath.dir, sizeof(newpath.dir))) {
//     set_error(ERROR_FILE_NOT_FOUND);
//     return;
//   }

//   /* Check for invalid characters in the new name */
//   if (check_invalid_name(newname)) {
//     /* This isn't correct for all cases, but for most. */
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }

//   /* Don't allow an empty new name */
//   /* The 1541 renames the file to "=" in this case, but I consider that a bug. */
//   if (ustrlen(newname) == 0) {
//     set_error(ERROR_SYNTAX_NONAME);
//     return;
//   }

//   /* Check if the new name already exists */
//   res = first_match(&newpath, newname, FLAG_HIDDEN, &dent);
//   if (res == 0) {
//     set_error(ERROR_FILE_EXISTS);
//     return;
//   }

//   if (res > 0)
//     /* first_match generated an error other than File Not Found, abort */
//     return;

//   /* Clear the FNF */
//   set_error(ERROR_OK);

//   /* Check if the old name exists */
//   if (first_match(&oldpath, oldname, FLAG_HIDDEN, &dent))
//     return;

//   rename(&oldpath, &dent, newname);
// }

// /* ------------- */
// /*  S - Scratch  */
// /* ------------- */
// static void parse_scratch(void) {
//   cbmdirent_t dent;
//   int8_t  res;
//   uint8_t count,cnt;
//   uint8_t *filename,*tmp,*name;
//   path_t  path;

//   clean_cmdbuffer();

//   filename = ustr1tok(command_buffer+1,',',&tmp);

//   set_dirty_led(1);
//   count = 0;
//   /* Loop over all file names */
//   while (filename != NULL) {
//     parse_path(filename, &path, &name, 0);

//     if (opendir(&matchdh, &path))
//       return;

//     while (1) {
//       res = next_match(&matchdh, name, NULL, NULL, FLAG_HIDDEN, &dent);
//       if (res < 0)
//         break;
//       if (res > 0)
//         return;

//       /* Skip directories */
//       if ((dent.typeflags & TYPE_MASK) == TYPE_DIR)
//         continue;
//       cnt = file_delete(&path, &dent);
//       if (cnt != 255)
//         count += cnt;
//       else
//         return;
//     }

//     filename = ustr1tok(NULL,',',&tmp);
//   }

//   set_error_ts(ERROR_SCRATCHED,count,0);
// }

// #ifdef HAVE_RTC
// /* ------------------ */
// /*  T - Time commands */
// /* ------------------ */

// /* --- T-R --- */
// static void parse_timeread(void) {
//   struct tm time;
//   uint8_t *ptr = error_buffer;
//   uint8_t hour;

//   if (rtc_state != RTC_OK) {
//     set_error(ERROR_SYNTAX_UNABLE);
//     return;
//   }

//   read_rtc(&time);
//   hour = time.tm_hour % 12;
//   if (hour == 0) hour = 12;

//   switch (command_buffer[3]) {
//   case 'A': /* ASCII format */
//     buffers[CONFIG_BUFFER_COUNT].lastused = 25;
//     memcpy_P(error_buffer+4, asciitime_skel, sizeof(asciitime_skel));
//     memcpy_P(error_buffer, downames + 4*time.tm_wday, 4);
//     appendnumber(error_buffer+5, time.tm_mon+1);
//     appendnumber(error_buffer+8, time.tm_mday);
//     appendnumber(error_buffer+11, time.tm_year % 100);
//     appendnumber(error_buffer+14, hour);
//     appendnumber(error_buffer+17, time.tm_min);
//     appendnumber(error_buffer+20, time.tm_sec);
//     if (time.tm_hour < 12)
//       error_buffer[23] = 'A';
//     else
//       error_buffer[23] = 'P';
//     break;

//   case 'B': /* BCD format */
//     buffers[CONFIG_BUFFER_COUNT].lastused = 8;
//     *ptr++ = time.tm_wday;
//     *ptr++ = int2bcd(time.tm_year % 100);
//     *ptr++ = int2bcd(time.tm_mon+1);
//     *ptr++ = int2bcd(time.tm_mday);
//     *ptr++ = int2bcd(hour);
//     *ptr++ = int2bcd(time.tm_min);
//     *ptr++ = int2bcd(time.tm_sec);
//     *ptr++ = (time.tm_hour >= 12);
//     *ptr   = 13;
//     break;

//   case 'D': /* Decimal format */
//     buffers[CONFIG_BUFFER_COUNT].lastused = 8;
//     *ptr++ = time.tm_wday;
//     *ptr++ = time.tm_year;
//     *ptr++ = time.tm_mon+1;
//     *ptr++ = time.tm_mday;
//     *ptr++ = hour;
//     *ptr++ = time.tm_min;
//     *ptr++ = time.tm_sec;
//     *ptr++ = (time.tm_hour >= 12);
//     *ptr   = 13;
//     break;

//   default: /* Unknown format */
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     break;
//   }
// }

// /* --- T-W --- */
// static void parse_timewrite(void) {
//   struct tm time;
//   uint8_t i, *ptr;

//   switch (command_buffer[3]) {
//   case 'A': /* ASCII format */
//     if (command_length < 27) { // Allow dropping the AM/PM marker for 24h format
//       set_error(ERROR_SYNTAX_UNABLE);
//       return;
//     }
//     for (i=0;i<7;i++) {
//       if (memcmp_P(command_buffer+4, downames + 4*i, 2) == 0) // only need to compare 2
//         break;
//     }
//     if (i == 7) {
//       set_error(ERROR_SYNTAX_UNABLE);
//       return;
//     }
//     time.tm_wday = i;
//     ptr = command_buffer + 9;
//     time.tm_mon  = parse_number(&ptr)-1;
//     ptr++;
//     time.tm_mday = parse_number(&ptr);
//     ptr++;
//     time.tm_year = parse_number(&ptr);
//     ptr++;
//     time.tm_hour = parse_number(&ptr);
//     ptr++;
//     time.tm_min  = parse_number(&ptr);
//     ptr++;
//     time.tm_sec  = parse_number(&ptr);
//     if (command_buffer[28] == 'M') {
//       /* Adjust for AM/PM only if AM/PM is actually supplied */
//       if (time.tm_hour == 12)
//         time.tm_hour = 0;
//       if (command_buffer[27] == 'P')
//         time.tm_hour += 12;
//     }
//     break;

//   case 'B': /* BCD format */
//     if (command_length < 12) {
//       set_error(ERROR_SYNTAX_UNABLE);
//       return;
//     }
//     time.tm_wday = command_buffer[4];
//     time.tm_year = bcd2int(command_buffer[5]);
//     time.tm_mon  = bcd2int(command_buffer[6])-1;
//     time.tm_mday = bcd2int(command_buffer[7]);
//     time.tm_hour = bcd2int(command_buffer[8]);
//     /* Hour range is 1-12, change 12:xx to 0:xx for easier conversion */
//     if (time.tm_hour == 12)
//       time.tm_hour = 0;
//     time.tm_min  = bcd2int(command_buffer[9]);
//     time.tm_sec  = bcd2int(command_buffer[10]);
//     if (command_buffer[11])
//       time.tm_hour += 12;
//     break;

//   case 'D': /* Decimal format */
//     if (command_length < 12) {
//       set_error(ERROR_SYNTAX_UNABLE);
//       return;
//     }
//     time.tm_wday = command_buffer[4];
//     time.tm_year = command_buffer[5];
//     time.tm_mon  = command_buffer[6]-1;
//     time.tm_mday = command_buffer[7];
//     time.tm_hour = command_buffer[8];
//     /* Hour range is 1-12, change 12:xx to 0:xx for easier conversion */
//     if (time.tm_hour == 12)
//       time.tm_hour = 0;
//     time.tm_min  = command_buffer[9];
//     time.tm_sec  = command_buffer[10];
//     if (command_buffer[11])
//       time.tm_hour += 12;
//     break;

//   default: /* Unknown format */
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     return;
//   }

//   /* Y2K fix for legacy apps */
//   if (time.tm_year < 80)
//     time.tm_year += 100;

//   /* The CMD drives don't check for validity, we do - partially */
//   if (time.tm_mday ==  0 || time.tm_mday >  31 ||
//       time.tm_mon  >  11 ||
//       time.tm_wday >   6 ||
//       time.tm_hour >  23 ||
//       time.tm_min  >  59 ||
//       time.tm_sec  >  59) {
//     set_error(ERROR_SYNTAX_UNABLE);
//     return;
//   }

//   set_rtc(&time);
// }

// /* --- T subparser --- */
// static void parse_time(void) {
//   if (rtc_state == RTC_NOT_FOUND)
//     set_error(ERROR_SYNTAX_UNKNOWN);
//   else {
//     if (command_buffer[2] == 'R') {
//       parse_timeread();
//     } else if (command_buffer[2] == 'W') {
//       parse_timewrite();
//     } else
//       set_error(ERROR_SYNTAX_UNKNOWN);
//   }
// }
// #endif /* HAVE_RTC */

// /* ------------ */
// /*  U commands  */
// /* ------------ */
// static void parse_user(void) {
//   switch (command_buffer[1]) {
//   case 'A':
//   case '1':
//     /* Tiny little hack: Rewrite as (B)-R and call that                */
//     /* This will always work because there is either a : in the string */
//     /* or the drive will start parsing at buf[3].                      */
//     command_buffer[0] = '-';
//     command_buffer[1] = 'R';
//     parse_block();
//     break;

//   case 'B':
//   case '2':
//     /* Tiny little hack: see above case for rationale */
//     command_buffer[0] = '-';
//     command_buffer[1] = 'W';
//     parse_block();
//     break;

//   case 'I':
//   case '9':
//     if (command_length == 2) {
//       /* Soft-reset - just return the dos version */
//       set_error(ERROR_DOSVERSION);
//       return;
//     }
//     switch (command_buffer[2]) {
//     case '+':
//       globalflags &= (uint8_t)~VC20MODE;
//       break;

//     case '-':
//       globalflags |= VC20MODE;
//       break;

//     default:
//       /* Soft-reset - just return the dos version */
//       set_error(ERROR_DOSVERSION);
//       break;
//     }
//     break;

//   case 'J':
//   case ':':
//     /* Reset - technically hard-reset */
//     /* Faked because Ultima 5 sends UJ. */
//     free_multiple_buffers(FMB_USER);
//     set_error(ERROR_DOSVERSION);
//     break;

//   case 202: /* Shift-J */
//     /* The real hard reset command */
//     system_reset();
//     break;

//   case '0':
//     /* U0 - only device address changes for now */
//     if ((command_buffer[2] & 0x1f) == 0x1e &&
//         command_buffer[3] >= 4 &&
//         command_buffer[3] <= 30) {
//       device_address = command_buffer[3];
//       display_address(device_address);
//       break;
//     }
//     /* Fall through */

//   default:
//     set_error(ERROR_SYNTAX_UNKNOWN);
//     break;
//   }
// }

// /* ------------ */
// /*  X commands  */
// /* ------------ */
// static void parse_xcommand(void) {
//   uint8_t num;
//   uint8_t *str;
//   path_t path;

//   clean_cmdbuffer();

//   switch (command_buffer[1]) {
//   case 'B':
//     /* Free-block count faking */
//     num = parse_bool();
//     if (num != 255) {
//       if (num)
//         globalflags |= FAT32_FREEBLOCKS;
//       else
//         globalflags &= (uint8_t)~FAT32_FREEBLOCKS;

//       set_error_ts(ERROR_STATUS,device_address,0);
//     }
//     break;

//   case 'E':
//     /* Change file extension mode */
//     str = command_buffer+2;
//     if (*str == '+') {
//       globalflags |= EXTENSION_HIDING;
//     } else if (*str == '-') {
//       globalflags &= (uint8_t)~EXTENSION_HIDING;
//     } else {
//       num = parse_number(&str);
//       if (num > 4) {
//         set_error(ERROR_SYNTAX_UNKNOWN);
//       } else {
//         file_extension_mode = num;
//         if (num >= 3)
//           globalflags |= EXTENSION_HIDING;
//       }
//     }
//     set_error_ts(ERROR_STATUS,device_address,0);
//     break;

//   case 'D':
//     /* drive config */
// #ifdef NEED_DISKMUX
//     str = command_buffer+2;
//     if(*str == '?') {
//       set_error_ts(ERROR_STATUS,device_address,1);
//       break;
//     }
//     num = parse_number(&str);
//     if(num < 8) {
//       while(*str == ' ')
//         str++;
//       if(*str == '=') {
//         uint8_t val, i;
//         str++;
//         val = parse_number(&str);
//         if(val <= 0x0f) {
//           for(i = 0;(val != 0x0f) && i < 8; i++) {
//             if(i != num && map_drive(num) == val) {
//               /* Trying to set the same drive mapping in two places. */
//               set_error(ERROR_SYNTAX_UNKNOWN);
//               break;
//             }
//           }
//           switch(val >> DRIVE_BITS) {
//           case DISK_TYPE_NONE:
// # ifdef HAVE_SD
//           case DISK_TYPE_SD:
// # endif
// # ifdef HAVE_ATA
//           case DISK_TYPE_ATA:
// # endif
//             if(map_drive(num) != val) {
//               set_map_drive(num,val);
//               /* sanity check.  If the user has truly turned off all drives, turn the
//                * defaults back on
//                */
//               if(drive_config == 0xffffffff)
//                 set_drive_config(get_default_driveconfig());
//               fatops_init(0);
//             }
//             break;
//           default:
//             set_error(ERROR_SYNTAX_UNKNOWN);
//             break;
//           }
//           break;
//         }
//       }
//     } else
//       set_error(ERROR_SYNTAX_UNKNOWN);
// #else
//     // return error for units without MUX support
//     set_error(ERROR_SYNTAX_UNKNOWN);
// #endif
//     break;

//   case 'I':
//     /* image-as-directory mode */
//     str = command_buffer + 2;
//     num = parse_number(&str);
//     if (num <= 2) {
//       image_as_dir = num;
//     } else {
//       set_error(ERROR_SYNTAX_UNKNOWN);
//     }
//     break;

//   case 'W':
//     /* Write configuration */
//     write_configuration();
//     set_error_ts(ERROR_STATUS,device_address,0);
//     break;

//   case 'R':
//     /* Set Rom-file */
//     if (command_buffer[2] == ':') {
//       if (command_length > ROM_NAME_LENGTH+3) {
//         set_error(ERROR_SYNTAX_TOOLONG);
//       } else {
//         ustrcpy(rom_filename, command_buffer+3);
//       }
//     } else {
//       /* Clear rom name */
//       rom_filename[0] = 0;
//     }
//     break;

//   case 'S':
//     /* Swaplist */
//     if (parse_path(command_buffer+2, &path, &str, 0))
//       return;

//     set_changelist(&path, str);
//     break;

//   case '*':
//     /* Post-* matching */
//     num = parse_bool();
//     if (num != 255) {
//       if (num)
//         globalflags |= POSTMATCH;
//       else
//         globalflags &= (uint8_t)~POSTMATCH;

//       set_error_ts(ERROR_STATUS,device_address,0);
//     }
//     break;

// #ifdef CONFIG_STACK_TRACKING
//   case '?':
//     /* Output the largest stack size seen */
//     //FIXME: AVR-only code
//     set_error_ts(ERROR_LONGVERSION,(RAMEND-minstack)>>8,(RAMEND-minstack)&0xff);
//     break;
// #else
//   case '?':
//     /* Output the long version string */
//     set_error(ERROR_LONGVERSION);
//     break;
// #endif

// #ifdef CONFIG_PARALLEL_DOLPHIN
//   case 'Q': // fast load
//     load_dolphin();
//     break;

//   case 'Z': // fast save
//     save_dolphin();
//     break;
// #endif

//   default:
//     /* Unknown command, just show the status */
//     set_error_ts(ERROR_STATUS,device_address,0);
//     break;
//   }
// }

/* ------------------------------------------------------------------------- */
/*  Main command parser function                                             */
/* ------------------------------------------------------------------------- */

//void CBMDOS_2_5::execute(std::string command)
//{
//     /* Set default message: Everything ok */
//     set_error(ERROR_OK);

//     /* Abort if the command is too long */
//     if (command_length == CONFIG_COMMAND_BUFFER_SIZE)
//     {
//         set_error(ERROR_SYNTAX_TOOLONG);
//         return;
//     }

// #ifdef CONFIG_COMMAND_CHANNEL_DUMP
//     /* Debugging aid: Dump the whole command via serial */
//     if (detected_loader == FL_NONE)
//     {
//         /* Dump only if no loader was detected because it may ruin the timing */
//         uart_trace(command_buffer, 0, command_length);
//     }
// #endif

//     // /* Remove one CR at end of command */
//     // original_length = command_length;
//     // if (command_length > 0 && command_buffer[command_length - 1] == 0x0d)
//     //     command_length--;

//     /* Abort if there is no command */
//     if (command.size() == 0)
//     {
//         set_error(ERROR_SYNTAX_UNABLE);
//         return;
//     }

//     /* Send command to display */
//     display_doscommand(command_length, command_buffer);

//     /* MD/CD/RD clash with other commands, so they're checked first */
//     if (command_buffer[0] != 'X' && command_buffer[1] == 'D')
//     {
//         parse_dircommand();
//         return;
//     }

//     switch (command_buffer[0])
//     {
//     case 'B':
//         /* Block-Something */
//         parse_block();
//         break;

//     case 'C':
//         /* Copy or Change Partition */
//         if (command_buffer[1] == 'P' || command_buffer[1] == 0xd0)
//             parse_changepart();
//         else
//             /* Copy a file */
//             parse_copy();
//         break;

//     case 'D':
//         /* Direct sector access (was duplicate in CBM drives) */
//         parse_direct();
//         break;

//     case 'G':
//         /* Get-Partition */
//         parse_getpartition();
//         break;

//     case 'I':
//         /* Initialize */
//         parse_initialize();
//         break;

//     case 'M':
//         /* Memory-something */
//         parse_memory();
//         break;

//     case 'N':
//         /* New */
//         parse_new();
//         break;

//     case 'P':
//         /* Position */
//         parse_position();
//         break;

//     case 'R':
//         /* Rename */
//         parse_rename();
//         break;

//     case 'S':
//         if (command_length == 3 && command_buffer[1] == '-')
//         {
//             /* Swap drive number */
//             set_error(ERROR_SYNTAX_UNABLE);
//             break;
//         }
//         /* Scratch */
//         parse_scratch();
//         break;

// #ifdef HAVE_RTC
//     case 'T':
//         parse_time();
//         break;
// #endif

//     case 'U':
//         parse_user();
//         break;

//     case 'X':
//         parse_xcommand();
//         break;

//     default:
//         set_error(ERROR_SYNTAX_UNKNOWN);
//         break;
//     }
// }