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


   buffers.h: Data structures for the internal buffers
*/

#ifndef BUFFERS_H
#define BUFFERS_H

#include <stdint.h>
#include "dirent.h"

#define ERRORBUFFER_IDX   CONFIG_BUFFER_COUNT
#define BUFFER_SEC_SYSTEM 100

/* Special-purpose buffer numbers */
#define BUFFER_SYS_BAM      (BUFFER_SEC_SYSTEM+1)

// fastloader data capture
#define BUFFER_SYS_CAPTURE1 (BUFFER_SEC_SYSTEM+2)
#define BUFFER_SYS_CAPTURE2 (BUFFER_SEC_SYSTEM+3)
#define BUFFER_SYS_CAPTURE3 (BUFFER_SEC_SYSTEM+4)

/* chained buffers use (BUFFER_SEC_CHAIN-14)..BUFFER_SEC_CHAIN */
/* to distinguish secondary addresses */
#define BUFFER_SEC_CHAIN    (BUFFER_SEC_SYSTEM-1)

/* Flags for free_multiple_buffers */
#define FMB_CLEAN          (1<<0)
#define FMB_FREE_SYSTEM    (1<<1)
#define FMB_FREE_STICKY    (1<<2)
#define FMB_ALL            (FMB_FREE_STICKY|FMB_FREE_SYSTEM)
#define FMB_ALL_CLEAN      (FMB_FREE_STICKY|FMB_FREE_SYSTEM|FMB_CLEAN)
#define FMB_USER           (FMB_FREE_STICKY)
#define FMB_USER_CLEAN     (FMB_FREE_STICKY|FMB_CLEAN)
#define FMB_UNSTICKY       (FMB_FREE_SYSTEM)
#define FMB_UNSTICKY_CLEAN (FMB_FREE_SYSTEM|FMB_CLEAN)

typedef enum { DIR_FMT_CBM, DIR_FMT_CMD_SHORT, DIR_FMT_CMD_LONG } dirformat_t;

/**
 * struct buffer_s - buffer handling structire
 * @data     : Pointer to the data area of the buffer, MUST be the first field
 * @lastused : Index to the last used byted
 * @position : Index of the byte that will be read/written next
 * @seconday : Secondary address the buffer is associated with
 * @recordlen: Record length, if buffer points to a REL file
 * @allocated: Flags if the buffer is allocated or not
 * @mustflush: Flags if the buffer must be flushed before adding characters
 * @read     : Flags if the buffer was opened for reading
 * @write    : Flags if the buffer was opened for writing
 * @sendeoi  : Flags if the last byte should be sent with EOI
 * @sticky   : Flags if the buffer will survive garbage collection
 * @refill   : Callback to refill/write out the buffer, returns true on error
 * @cleanup  : Callback to clean up and save remaining data, returns true on error
 *
 * Most allocated buffers point into the same bufferdata array, but
 * the error channel uses the same structure to avoid special-casing it
 * everywhere.
 */
typedef struct buffer_s {
  /* The error channel uses the same data structure for convenience reasons, */
  /* so data must be a pointer. It also allows swapping the buffers around   */
  /* in case I ever add external ram (not XRAM) to the design (which will    */
  /* require locking =( ).                                                   */
  uint8_t *data;
  uint8_t lastused;
  uint8_t position;
  uint8_t secondary;
  uint8_t recordlen;
  uint32_t fptr;  // FIXME: Missing from doc comment
  int     allocated:1;
  int     mustflush:1;
  int     read:1;
  int     write:1;
  int     dirty:1;
  int     sendeoi:1;
  int     sticky:1;
  uint8_t (*seek) (struct buffer_s *buffer, uint32_t position, uint8_t index);
  uint8_t (*refill)(struct buffer_s *buffer);
  uint8_t (*cleanup)(struct buffer_s *buffer);

  /* private: */
  union {
    struct {
      dh_t dh;             /* Directory handle */
      uint8_t filetype;    /* File type */
      dirformat_t format;  /* Dir format */
      uint8_t *matchstr;   /* Pointer to filename pattern */
      date_t *match_start; /* Start matching date */
      date_t *match_end;   /* End matching date */
      uint8_t counter;     /* used for counting raw entries */
    } dir;
    struct {
      FIL fh;              /* File access via FAT */
      uint8_t headersize;  /* offset to start of file data */
    } fat;
    d64fh_t d64;           /* File access on D64  */
    eefs_fh_t eefh;        /* File handle for eepromfs */
    struct {
      uint8_t part;        /* partition number for $=P */
      uint8_t *matchstr;   /* Pointer to filename pattern */
    } pdir;
    struct {
      uint8_t part;        /* partition number where the BAM came from */
      uint8_t track;       /* BAM-track (if more than one) */
      uint8_t sector;      /* BAM-sector (if more than one) */
    } bam;
    struct {
      uint8_t part;           /* current partition at buffer creation time */
      uint8_t size;           /* Number of buffers in chain  */
      struct buffer_s *first; /* Pointer to the first buffer */
      struct buffer_s *next;  /* Pointer to the next buffer  */
    } buffer;
  } pvt;
} buffer_t;

extern dh_t matchdh;         /// Directory handle used in file matching
extern buffer_t buffers[];   /// Simplifies access to the error buffer length

extern uint8_t ops_scratch[33]; /// scratch space for use in fileops code

/* Initializes the buffer structures */
void buffers_init(void);

/* Dummy callback */
uint8_t callback_dummy(buffer_t *buf);

/* Allocates a buffer for internal use */
buffer_t *alloc_system_buffer(void);

/* Allocates a buffer - returns pointer to buffer or NULL if failure */
buffer_t *alloc_buffer(void);

/* Allocates linked buffers - returns pointer to first buffer or NULL if failure */
/* Buffers are guranteed to have continuous data segments. */
buffer_t *alloc_linked_buffers(uint8_t count);

/* Call the cleanup function and deallocate a buffer */
void cleanup_and_free_buffer(buffer_t *buffer);

/* Deallocates a buffer */
void free_buffer(buffer_t *buffer);

/* Deallocates multiple buffers */
uint8_t free_multiple_buffers(uint8_t flags);

/* Mark a buffer as sticky */
static void inline stick_buffer(buffer_t *buf) {
  buf->sticky = 1;
}

/* remove sticky mark */
static void inline unstick_buffer(buffer_t *buf) {
  buf->sticky = 0;
}

/* Finds the buffer corresponding to a secondary address */
/* Returns pointer to buffer on success or NULL on failure */
buffer_t *find_buffer(uint8_t secondary);

/* Number of currently allocated buffers + 16 * number of write buffers */
extern uint8_t active_buffers;

/* Check if any buffers are free */
#define check_free_buffers() ((active_buffers & 0x0f) < CONFIG_BUFFER_COUNT)

/* Return the number of dirty buffers */
#define get_dirty_buffer_count() (active_buffers >> 4)

/* Mark a buffer as write-buffer and sticky it */
// Note: inline function is smaller than external on AVR with gcc 4.8.2
static inline void mark_write_buffer(buffer_t *buf) {
  buf->write = 1;
  stick_buffer(buf);
}

/* Mark a buffer as dirty */
void mark_buffer_dirty(buffer_t *buf);

/* Mark a buffer as clean */
void mark_buffer_clean(buffer_t *buf);


#ifdef __AVR__
/* AVR-specific hack: Address 1 is r1 which is always zero in C code */
#  define NULLSTRING ((uint8_t *)1)
#else
#  define NULLSTRING ((uint8_t *)"")
#endif

#endif
