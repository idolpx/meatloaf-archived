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


   buffers.c: Internal buffer management
*/

#include <stdint.h>
#include <string.h>
#include "config.h"
#include "dirent.h"
#include "errormsg.h"
#include "ff.h"
#include "led.h"
#include "buffers.h"

dh_t    matchdh;
uint8_t ops_scratch[33];

/// One additional buffer structure for channel 15
buffer_t buffers[CONFIG_BUFFER_COUNT+1];

/// The actual data buffers
static uint8_t bufferdata[CONFIG_BUFFER_COUNT*256];

/// Number of active data buffers + 16 * number of dirty buffers
uint8_t active_buffers;

/**
 * callback_dummy - dummy function for the buffer callbacks
 * @buf: pointer to a buffer
 *
 * This is the default function used for all buffer callbacks so
 * the caller doesn't need to check if the callbacks are valid.
 * It always returns 0 for success.
 */
uint8_t callback_dummy(buffer_t *buf) {
  return 0;
}

/**
 * buffers_init - initializes the buffer data structures
 *
 * This function initialized all the buffer-related data structures.
 */
void buffers_init(void) {
  uint8_t i;

  memset(buffers,0,sizeof(buffers));
  for (i=0;i<CONFIG_BUFFER_COUNT;i++)
    buffers[i].data = bufferdata + 256*i;

  buffers[ERRORBUFFER_IDX].data      = error_buffer;
  buffers[ERRORBUFFER_IDX].secondary = 15;
  buffers[ERRORBUFFER_IDX].allocated = 1;
  buffers[ERRORBUFFER_IDX].read      = 1;
  buffers[ERRORBUFFER_IDX].write     = 1;
  buffers[ERRORBUFFER_IDX].sendeoi   = 1;
  buffers[ERRORBUFFER_IDX].refill    = set_ok_message;
  buffers[ERRORBUFFER_IDX].cleanup   = callback_dummy;
}

/**
 * alloc_specific_buffer - allocate a specific buffer for system use
 *
 * This function allocates the specified buffer and marks it as used.
 * Returns without doing anything if the buffer is already allocated.
 */
static void alloc_specific_buffer(uint8_t bufnum) {
  if (!buffers[bufnum].allocated) {
    /* Clear everything except the data pointer */
    memset(sizeof(uint8_t *)+(char *)&(buffers[bufnum]),0,sizeof(buffer_t)-sizeof(uint8_t *));
    buffers[bufnum].allocated = 1;
    buffers[bufnum].secondary = BUFFER_SEC_SYSTEM;
    buffers[bufnum].refill    = callback_dummy;
    buffers[bufnum].cleanup   = callback_dummy;
  }
}

/**
 * alloc_system_buffer - allocate a buffer for system use
 *
 * This function allocates a buffer and marks it as used. Returns a
 * pointer to the buffer structure or NULL of no buffer is free.
 */
buffer_t *alloc_system_buffer(void) {
  uint8_t i;

  for (i=0;i<CONFIG_BUFFER_COUNT;i++) {
    if (!buffers[i].allocated) {
      alloc_specific_buffer(i);
      return &buffers[i];
    }
  }

  set_error(ERROR_NO_CHANNEL);
  return NULL;
}

/**
 * alloc_buffer - allocates a buffer
 *
 * This function allocates a buffer and marks it as used. It will also
 * turn on the busy LED to notify the user. Returns a pointer to the
 * buffer structure or NULL if no buffer is free.
 */
buffer_t *alloc_buffer(void) {
  buffer_t *buf = alloc_system_buffer();
  if (buf != NULL) {
    buf->secondary = 0;
    active_buffers++;
    set_busy_led(1);
  }
  return buf;
}

/**
 * alloc_linked_buffers - allocates linked buffers
 * @count    : Number of buffers to allocate
 *
 * This function allocates count buffers, marks them as used and
 * links them. It will also turn on the busy LED to notify the user.
 * Returns a pointer to the first buffer structure or NULL if
 * not enough buffers are free. The data segments of the allocated
 * buffers are guaranteed to be continuous.
 */
buffer_t *alloc_linked_buffers(uint8_t count) {
  uint8_t i,freebufs,start;

  freebufs = 0;
  start    = 0;
  for (i=0;i<CONFIG_BUFFER_COUNT;i++) {
    if (buffers[i].allocated) {
      /* Look for continuous buffers */
      /* Switching data segments is possible, but probably not required */
      freebufs = 0;
    } else {
      if (freebufs == 0)
        start = i;
      freebufs++;
      /* Found enough free space */
      if (freebufs == count)
        break;
    }
  }

  if (freebufs < count) {
    set_error(ERROR_NO_CHANNEL);
    return NULL;
  }

  /* Chain the buffers */
  for (i=0;i<count;i++) {
    alloc_specific_buffer(start+i);
    active_buffers++;
    buffers[start+i].secondary = 0;
    buffers[start+i].pvt.buffer.next  = &buffers[start+i+1];
    buffers[start+i].pvt.buffer.first = &buffers[start];
    buffers[start+i].pvt.buffer.size  = count;
  }

  set_busy_led(1);

  buffers[start+count-1].pvt.buffer.next = NULL;

  return &buffers[start];
}

/**
 * cleanup_and_free_buffer - cleanup and deallocate a buffer
 * @buffer: pointer to the buffer structure to cleanup and mark as free
 *
 * This function calls the cleanup function and deallocates the buffer.
 */
void cleanup_and_free_buffer(buffer_t *buffer) {
  buffer->cleanup(buffer);
  free_buffer(buffer);
}

/**
 * free_buffer - deallocate a buffer
 * @buffer: pointer to the buffer structure to mark as free
 *
 * This function deallocates the given buffer. If the pointer is NULL,
 * the buffer is already freed or the buffer is assigned to secondary
 * address 15 nothing will happen. This function will also update the
 * two LEDs according to the remaining number of open and writable
 * buffers.
 */
void free_buffer(buffer_t *buffer) {
  if (buffer == NULL) return;
  if (buffer->secondary == 15) return;
  if (!buffer->allocated) return;

  buffer->allocated = 0;

  if (buffer->dirty)
    active_buffers -= 16;
  if (buffer->secondary < BUFFER_SEC_SYSTEM)
    active_buffers--;

  update_leds();
}

/**
 * free_multiple_buffers - deallocates multiple buffers
 * @flags: Defines which buffers should be deallocated
 *
 * This function iterates over all buffers and frees those which match the
 * specification in flags (see FMB_* defines). If FMB_CLEAN is set it will
 * also call the cleanup function for those buffers which are about to be
 * freed. When FMB_CLEAN is set, the function returns 0 if all cleanup
 * functions returned 0 or 1 if at least one did not. When FMB_CLEAN is not
 * set, returns 0.
 */
uint8_t free_multiple_buffers(uint8_t flags) {
  uint8_t i,res;

  res = 0;

  for (i=0;i<CONFIG_BUFFER_COUNT;i++) {
    if (buffers[i].allocated) {
      if ((flags & FMB_FREE_SYSTEM) || buffers[i].secondary < BUFFER_SEC_SYSTEM) {
        if ((flags & FMB_FREE_STICKY) || !buffers[i].sticky) {
          if (flags & FMB_CLEAN) {
            res = res || buffers[i].cleanup(&buffers[i]);
          }
          free_buffer(&buffers[i]);
        }
      }
    }
  }

  return res;
}

/**
 * find_buffer - find the buffer corresponding to a secondary address
 * @secondary: secondary address to look for
 *
 * This function returns a pointer to the first buffer structure whose
 * secondary address is the same as the one given. Returns NULL if
 * no matching buffer was found.
 */
buffer_t *find_buffer(uint8_t secondary) {
  uint8_t i;

  for (i=0;i<CONFIG_BUFFER_COUNT+1;i++) {
    if (buffers[i].allocated && buffers[i].secondary == secondary)
      return &buffers[i];
  }
  return NULL;
}

/**
 * mark_buffer_dirty - mark a buffer as dirty
 * @buf: pointer to the buffer
 *
 * This function marks the given buffer as dirty, tracks
 * this in active_buffers and turns on the dirty LED.
 */
void mark_buffer_dirty(buffer_t *buf) {
  if (!buf->dirty) {
    buf->dirty = 1;
    active_buffers += 16;
    set_dirty_led(1);
  }
}

/**
 * mark_buffer_clean - mark a buffer as clean
 * @buf: pointer to the buffer
 *
 * This function marks the given buffer as clean, tracks
 * this in active_buffers and turns off the dirty LED if required.
 */
void mark_buffer_clean(buffer_t *buf) {
  if (buf->dirty) {
    buf->dirty = 0;
    active_buffers -= 16;
    if (get_dirty_buffer_count() == 0)
      set_dirty_led(0);
  }
}
