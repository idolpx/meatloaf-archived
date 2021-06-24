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


   dirent.h: Various data structures for directory browsing

*/

#ifndef DIRENT_H
#define DIRENT_H

//#include "eeprom-fs.h"
//#include "ff.h"

#define CBM_NAME_LENGTH 16

#define TYPE_LENGTH 3
#define TYPE_MASK 7
#define EXT_TYPE_MASK 15

/* Standard file types */
#define TYPE_DEL 0
#define TYPE_SEQ 1
#define TYPE_PRG 2
#define TYPE_USR 3
#define TYPE_REL 4
#define TYPE_CBM 5
#define TYPE_DIR 6

/* Internal file types used for the partition directory */
#define TYPE_NAT 8

/* Internal file type used to force files without header on FAT (for M2I) */
#define TYPE_RAW 15

/// Hidden is an unused bit on CBM
#define FLAG_HIDDEN (1<<5)
#define FLAG_RO     (1<<6)
#define FLAG_SPLAT  (1<<7)

/* forward declaration to avoid an include loop */
struct buffer_s;

/**
 * @date      : 1900-based year
 * @month     : month, 1..12
 * @day       : day,   1..(28-31)
 * @hour      : hour   0..23
 * @minute    : minute 0..59
 * @second    : second 0..59 (FIXME: leap seconds ;) )
 *
 * This struct holds a file timestamp. It must have the "packed" attribute
 * with the fields in descending order of magnitude because it is compared
 * using memcmp.
 */
typedef struct date {
  uint8_t  year;
  uint8_t  month;
  uint8_t  day;
  uint8_t  hour;
  uint8_t  minute;
  uint8_t  second;
}  __attribute__((packed)) date_t;

/**
 * struct dir_t - struct of directory references for various ops
 * @fat: cluster number of the directory start for FAT
 * @dxx: track/sector of the first directory sector for Dxx
 */
typedef struct {
  uint32_t fat;
  struct {
    uint8_t track;
    uint8_t sector;
  } dxx;
} dir_t;

/**
 * struct path_t - struct to reference a directory
 * @part: partition number (0-based)
 * @dir : directory reference
 *
 * This structure is passed around to various functions as
 * a reference to a specific partition and directory.
 * File operation type is implicitly given by partitions[part].fop
 * until multi-type references on a single partition are implemented.
 */
typedef struct {
  uint8_t  part;
  dir_t    dir;
} path_t;

/**
 * struct d64dh - D64 directory handle
 * @track : track of the current directory sector
 * @sector: sector of the current directory sector
 * @entry : number of the current directory entry in its sector
 *
 * This structure addresses an entry in a D64 directory by its track,
 * sector and entry (8 entries per sector).
 */
struct d64dh {
  uint8_t track;
  uint8_t sector;
  uint8_t entry;
};

/* Ops type selector for cbmdirent_t */
typedef enum {
  OPSTYPE_UNDEFINED = 0,
  OPSTYPE_FAT,
  OPSTYPE_FAT_X00,  /* X00 files can never be disk images */
                    /* and should match case-sensitive    */
  OPSTYPE_M2I,
  OPSTYPE_DXX,
  OPSTYPE_EEFS
} opstype_t;

/**
 * struct cbmdirent_t - directory entry for CBM names
 * @name      : 0-padded commodore file name
 * @typeflags : OR of file type and flags
 * @blocksize : Size in blocks of 254 bytes
 * @remainder : (filesize MOD 254) or 0xff if unknown
 * @date      : Last modified date
 * @opstype   : selects which part of the pvt union is valid
 * @pvt       : fileops-specific private data
 * @pvt.fat.cluster : Start cluster of the entry
 * @pvt.fat.realname: Actual 8.3 name of the file (preferred if present)
 * @pvt.dxx.dh      : Dxx directory handle for the dir entry of this file
 * @pvt.m2i.offset  : Offset in the M2I file
 *
 * This structure holds a CBM filename, its type and its size. The typeflags
 * are almost compatible to the file type byte in a D64 image, but the splat
 * bit is inverted. The name is padded with 0-bytes and will always be
 * zero-terminated. If it was read from a D64 file, it may contain valid
 * characters beyond the first 0-byte which should be displayed in the
 * directory, but ignored for name matching purposes.
 * realname is either an empty string or the actual ASCII 8.3 file name
 * of the file if the data in the name field was modified compared
 * to the on-disk name (e.g. extension hiding or x00).
 * year/month/day/hour/minute/second specify the time stamp of the file.
 * It is recommended to set it to 1982-08-31 00:00:00 if unknown because
 * this is currently the value used by FatFs for new files.
 */
typedef struct {
  uint8_t   name[CBM_NAME_LENGTH+1];
  uint8_t   typeflags;
  uint16_t  blocksize;
  uint8_t   remainder;
  date_t    date;
  opstype_t opstype;
  union {
    struct {
      uint32_t cluster;
      uint8_t  realname[8+3+1+1];
    } fat;
    struct {
      struct d64dh dh;
    } dxx;
    struct {
      uint16_t offset;
    } m2i;
  } pvt;
} cbmdirent_t;

/**
 * struct d64fh - D64 file handle
 * @dh    : d64dh pointing to the directory entry
 * @part  : partition
 * @track : current track
 * @sector: current sector
 * @blocks: number of sectors allocated before the current
 *
 * This structure holds the information required to write to a file
 * in a D64 image and update its directory entry upon close.
 */
typedef struct d64fh {
  struct d64dh dh;
  uint8_t part;
  uint8_t track;
  uint8_t sector;
  uint16_t blocks;
} d64fh_t;

/**
 * struct dh_t - union of all directory handles
 * @part: partition number for the handle
 * @fat : fat directory handle
 * @m2i : m2i directory handle (offset of entry in the file)
 * @d64 : d64 directory handle
 *
 * This is a union of directory handles for all supported file types
 * which is used as an opaque type to be passed between openddir and
 * readdir.
 */
typedef struct dh_s {
  uint8_t part;
  union {
    DIR          fat;
    uint16_t     m2i;
    struct d64dh d64;
    eefs_dir_t   eefs;
  } dir;
} dh_t;

/* This enum must match the struct param_s below! */
typedef enum { DIR_TRACK = 0, DIR_START_SECTOR,
               LAST_TRACK, LABEL_OFFSET, ID_OFFSET,
               FILE_INTERLEAVE, DIR_INTERLEAVE
               /* , FORMAT_FUNCTION */ } param_t;

/**
 * struct param_s - Dxx image file parameters
 * @dir_track       : track of the (current) directory
 * @dir_start_sector: first sector of the directory on dir_track
 * @last_track      : highest valid track number
 * @label_offset    : byte offset of the disk label from the beginning of dir_track/0
 * @id_offset       : as label_offset, for the disk ID
 * @file_interleave : interleave factor for file sectors
 * @dir_interleave  : interleave factor for directory sectors
 * @format_function : image-specific format function
 *
 * This structure holds those parameters that differ between various Dxx
 * disk images and which could be abstracted out easily.
 */
struct param_s {
  uint8_t dir_track;
  uint8_t dir_start_sector;
  uint8_t last_track;
  uint8_t label_offset;
  uint8_t id_offset;
  uint8_t file_interleave;
  uint8_t dir_interleave;
  // Insert new fields here, otherwise get_param will fail!
  void    (*format_function)(uint8_t part, struct buffer_s *buf, uint8_t *name, uint8_t *idbuf);
};

/**
 * struct partition_t - per-partition data
 * @fatfs      : FatFs per-drive/partition structure
 * @current_dir: current directory on FAT as seen by sd2iec
 * @fop        : pointer to the fileops structure for this partition
 * @imagehandle: file handle of a mounted image file on this partition
 * @imagetype  : disk image type mounted on this partition
 * @d64data    : extended information about a mounted Dxx image
 *
 * This data structure holds per-partition data.
 */
typedef struct partition_s {
  FATFS                  fatfs;
  dir_t                  current_dir;
  const struct fileops_s *fop;
  FIL                    imagehandle;
  uint8_t                imagetype;
  struct param_s         d64data;
} partition_t;

#endif
