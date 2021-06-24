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


   parser.h: Definitions for the common file name parsers

*/

#ifndef PARSER_H
#define PARSER_H

#include "dirent.h"
#include "ff.h"

extern partition_t partition[CONFIG_MAX_PARTITIONS];
extern uint8_t current_part;
extern uint8_t max_part;

/* Non-zero after any directory change */
/* Used for disk (image) change detection in some fastloaders */
/* Must be reset by its user */
extern uint8_t dir_changed;

/* Update current_dir in partition array */
void update_current_dir(path_t *path);

/* Parse a partition number */
uint8_t parse_partition(uint8_t **buf);

/* Performs CBM DOS pattern matching */
uint8_t match_name(uint8_t *matchstr, cbmdirent_t *dent, uint8_t ignorecase);

/* Returns the next matching dirent */
int8_t next_match(dh_t *dh, uint8_t *matchstr, date_t *start, date_t *end, uint8_t type, cbmdirent_t *dent);

/* Returns the first matching dirent */
int8_t first_match(path_t *path, uint8_t *matchstr, uint8_t type, cbmdirent_t *dent);

/* Parses CMD-style directory specifications */
uint8_t parse_path(uint8_t *in, path_t *path, uint8_t **name, uint8_t parse_always);

/* Check for invalid characters in a name */
uint8_t check_invalid_name(uint8_t *name);

/* Parse a decimal number at str and return a pointer to the following char */
uint16_t parse_number(uint8_t **str);

/* parse CMD-style dates */
uint8_t parse_date(date_t *date, uint8_t **str);

#endif
