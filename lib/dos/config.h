//
// sd2iec - SD/MMC to Commodore serial bus interface/controller
// Copyright (C) 2007-2014  Ingo Korb <ingo@akana.de>
//
//  Inspired by MMC2IEC by Lars Pontoppidan et al.
//
//  FAT filesystem access based on code from ChaN, see tff.c|h.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 of the License only.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  #define CONFIG: User-#define CONFIGurable options to simplify hardware changes and/or
//          reduce the code/ram requirements of the code.
//
//
// This file is included in the main sd2iec Makefile and also parsed
// into autoconf.h.

#define CONFIG_ARCH avr
#define CONFIG_MCU atmega1281
#define CONFIG_LINKER_RELAX y
#define CONFIG_MCU_FREQ 8000000
#define CONFIG_BOOTLOADER y
#define CONFIG_BOOT_DEVID 0x33304955
#define CONFIG_UART_DEBUG y
#define CONFIG_UART_BAUDRATE 38400
#define CONFIG_UART_BUF_SHIFT 6
#define CONFIG_DEADLOCK_ME_HARDER n
#define CONFIG_COMMAND_CHANNEL_DUMP y
#define CONFIG_LOADER_TURBODISK y
#define CONFIG_LOADER_FC3 y
#define CONFIG_LOADER_DREAMLOAD y
#define CONFIG_LOADER_ULOAD3 y
#define CONFIG_LOADER_GIJOE y
#define CONFIG_LOADER_EPYXCART y
#define CONFIG_LOADER_GEOS y
#define CONFIG_LOADER_WHEELS y
#define CONFIG_LOADER_NIPPON y
#define CONFIG_LOADER_AR6 y
#define CONFIG_LOADER_ELOAD1 y
#define CONFIG_HARDWARE_VARIANT 7
#define CONFIG_HARDWARE_NAME uIEC
#define CONFIG_TWINSD n
#define CONFIG_SD_AUTO_RETRIES 10
#define CONFIG_SD_DATACRC y
#define CONFIG_ERROR_BUFFER_SIZE 100
#define CONFIG_COMMAND_BUFFER_SIZE 254
#define CONFIG_BUFFER_COUNT 15
#define CONFIG_MAX_PARTITIONS 20
#define CONFIG_RTC_SOFTWARE y
#define CONFIG_HAVE_IEC y
#define CONFIG_M2I y
#define CONFIG_HAVE_EEPROMFS y
#define CONFIG_LOADER_MMZAK y
#define CONFIG_LOADER_N0SDOS y
#define CONFIG_LOADER_SAMSJOURNEY y
