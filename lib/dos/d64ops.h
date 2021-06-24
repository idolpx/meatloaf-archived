/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007-2017  Ingo Korb <ingo@akana.de>

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


   d64ops.h: Definitions for the D64 operations

*/

#ifndef D64OPS_H
#define D64OPS_H

#include "wrapops.h"

/* Offsets in a D64 directory entry, also needed for raw dirs */
#define DIR_OFS_FILE_TYPE       2
#define DIR_OFS_TRACK           3
#define DIR_OFS_SECTOR          4
#define DIR_OFS_FILE_NAME       5
#define DIR_OFS_YEAR            0x19
#define DIR_OFS_MONTH           0x1a
#define DIR_OFS_DAY             0x1b
#define DIR_OFS_HOUR            0x1c
#define DIR_OFS_MINUTE          0x1d
#define DIR_OFS_SIZE_LOW        0x1e
#define DIR_OFS_SIZE_HI         0x1f

/* Disk image types - values must match G-P partition type byte */
#define D64_TYPE_MASK 0x7f
#define D64_TYPE_NONE 0
#define D64_TYPE_DNP  1
#define D64_TYPE_D41  2
#define D64_TYPE_D71  3
#define D64_TYPE_D81  4
#define D64_HAS_ERRORINFO 128

extern const fileops_t d64ops;

uint8_t d64_mount(path_t *path, uint8_t *name);
void    d64_unmount(uint8_t part);

/* commit BAM buffer contents to storage medium */
uint8_t d64_bam_commit(void);

void d64_raw_directory(path_t *path, buffer_t *buf);
void d64_invalidate(void);

#endif
