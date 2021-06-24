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


   wrapops.h: Lots of ugly defines to allow switchable file operations

   The structure-of-pgmspace-function-pointers access scheme used here
   was inspired by code from MMC2IEC by Lars Pontoppidan.

   typeof-usage suggested by T. Diedrich.
*/

#ifndef WRAPOPS_H
#define WRAPOPS_H

#include "buffers.h"
#include "fileops.h"
#include "ff.h"
#include "progmem.h"

/**
 * struct fileops_t - function pointers to file operations
 * @open_read   : open a file for reading
 * @open_write  : open a file for writing/appending
 * @open_rel    : open a relative file
 * @file_delete : delete a file
 * @disk_label  : read disk label (up to 17 bytes, 0-terminated)
 * @dir_label   : read dir label  (16 characters, space-padded, unterminated)
 * @disk_id     : read disk id
 * @disk_free   : read free space of disk
 * @read_sector : read a sector from the disk
 * @write_sector: write a sector to the disk
 * @format      : format the disk
 * @opendir     : open a directory
 * @readdir     : read an entry from a directory
 * @mkdir       : create a directory
 * @chdir       : change current directory
 * @rename      : rename a file
 *
 * This structure holds function pointers for the various
 * abstracted operations on the supported file systems/images.
 * Instances of this structure must always be allocated in flash
 * and no field may be set to NULL.
 */
typedef struct fileops_s {
  void     (*open_read)(path_t *path, cbmdirent_t *name, buffer_t *buf);
  void     (*open_write)(path_t *path, cbmdirent_t *name, uint8_t type, buffer_t *buf, uint8_t append);
  void     (*open_rel)(path_t *path, cbmdirent_t *name, buffer_t *buf, uint8_t recordlen, uint8_t mode);
  uint8_t  (*file_delete)(path_t *path, cbmdirent_t *name);
  uint8_t  (*disk_label)(uint8_t part, uint8_t *label);
  uint8_t  (*dir_label)(path_t *path, uint8_t *label);
  uint8_t  (*disk_id)(path_t *path, uint8_t *id);
  uint16_t (*disk_free)(uint8_t part);
  void     (*read_sector)(buffer_t *buf, uint8_t part, uint8_t track, uint8_t sector);
  void     (*write_sector)(buffer_t *buf, uint8_t part, uint8_t track, uint8_t sector);
  void     (*format)(uint8_t drv, uint8_t *name, uint8_t *id);
  uint8_t  (*opendir)(dh_t *dh, path_t *path);
  int8_t   (*readdir)(dh_t *dh, cbmdirent_t *dent);
  void     (*mkdir)(path_t *path, uint8_t *dirname);
  uint8_t  (*chdir)(path_t *path, cbmdirent_t *dent);
  void     (*rename)(path_t *path, cbmdirent_t *oldname, uint8_t *newname);
} fileops_t;

/* Helper-Define to avoid lots of typedefs */
#ifdef __AVR__
#  define pgmcall(x) ((typeof(x))pgm_read_word(&(x)))
#else
/* No-op macro - avoids issues with different function pointer sizes */
#  define pgmcall(x) x
#endif

/* Wrappers to make the indirect calls look like normal functions */
#define open_read(path,name,buf) ((pgmcall(partition[(path)->part].fop->open_read))(path,name,buf))
#define open_write(path,name,type,buf,app) ((pgmcall(partition[(path)->part].fop->open_write))(path,name,type,buf,app))
#define open_rel(path,name,buf,len,mode) ((pgmcall(partition[(path)->part].fop->open_rel))(path,name,buf,len,mode))
#define file_delete(path,name) ((pgmcall(partition[(path)->part].fop->file_delete))(path,name))
#define disk_label(part,label) ((pgmcall(partition[part].fop->disk_label))(part,label))
#define dir_label(path,label) ((pgmcall(partition[(path)->part].fop->dir_label))(path,label))
#define disk_id(path,id) ((pgmcall(partition[(path)->part].fop->disk_id))(path,id))
#define disk_free(drv) ((pgmcall(partition[drv].fop->disk_free))(drv))
#define read_sector(buf,drv,t,s) ((pgmcall(partition[(drv)].fop->read_sector))(buf,drv,t,s))
#define write_sector(buf,drv,t,s) ((pgmcall(partition[(drv)].fop->write_sector))(buf,drv,t,s))
#define format(drv,name,id) ((pgmcall(partition[(drv)].fop->format))(drv,name,id))
#define opendir(dh,path) ((pgmcall(partition[(path)->part].fop->opendir))(dh,path))
#define readdir(dh,dent) ((pgmcall(partition[(dh)->part].fop->readdir))(dh,dent))
#define mkdir(path,dir) ((pgmcall(partition[(path)->part].fop->mkdir))(path,dir))
#define chdir(path,dent) ((pgmcall(partition[(path)->part].fop->chdir))(path,dent))
#define rename(path,old,new) ((pgmcall(partition[(path)->part].fop->rename))(path,old,new))

#endif
