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


   parser.c: Common file name parsers

*/

#include <stdio.h>

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "dirent.h"
#include "display.h"
#include "eefs-ops.h"
#include "errormsg.h"
#include "fatops.h"
#include "flags.h"
#include "ustring.h"
#include "parser.h"

partition_t partition[CONFIG_MAX_PARTITIONS];
uint8_t current_part;
uint8_t max_part;
uint8_t dir_changed;

/* Updates current_dir in the partition array and sends */
/* the new dir to the display.                          */
void update_current_dir(path_t *path){
  partition[path->part].current_dir = path->dir;
  dir_changed = 1;

  if (display_found && path->part == current_part) {
    uint8_t dirname[CBM_NAME_LENGTH+1];
    uint8_t *ptr = dirname + CBM_NAME_LENGTH;

    dir_label(path, dirname);
    *ptr-- = 0;

    while (*ptr == ' ') *ptr-- = 0;
    display_current_directory(path->part, dirname);
  }
}

/**
 * check_invalid_name - check for invalid characters in a file name
 * @name: pointer to the file name
 *
 * This function checks if the passed name contains any characters
 * that are not valid in a CBM file name. Returns 1 if invalid
 * characters are found, 0 if not.
 */
uint8_t check_invalid_name(uint8_t *name) {
  while (*name) {
    if (*name == '=' || *name == '"' ||
        *name == '*' || *name == '?' ||
        *name == ',')
      return 1;
    name++;
  }
  return 0;
}

/* Convert a PETSCII character to lower-case */
static uint8_t tolower_pet(uint8_t c) {
  if (c >= 0x61 && c <= 0x7a)
    c -= 0x20;
  else if (c >= 0xc1 && c <= 0xda)
    c -= 0x80;
  return c;
}

/**
 * parse_partition - parse a partition number from a file name
 * @buf     : pointer to pointer to filename
 *
 * This function parses the partition number from the file name
 * specified in buf and advances buf to point at the first character
 * that isn't part of the number. Returns a 0-based partition number.
 */
uint8_t parse_partition(uint8_t **buf) {
  uint8_t part=0;

#ifdef CONFIG_HAVE_EEPROMFS
  /* special case: recognize "!:" as alias for the eefs partition */
  if (eefs_partition != 255 && (*buf)[0] == '!' && (*buf)[1] == ':') {
    (*buf) += 2;
    return eefs_partition;
  }
#endif

  while (isdigit(**buf) || **buf == ' ' || **buf == '@') {
    if(isdigit(**buf))
      part=part*10+(**buf-'0');
    (*buf)++;
  }
  if (part == 0)
    return current_part;
  else
    return part-1;
}


/**
 * match_name - Match a pattern against a file name
 * @matchstr  : pattern to be matched
 * @dent      : pointer to the directory entry to be matched against
 * @ignorecase: ignore the case of the file names
 *
 * This function tests if matchstr matches name in dent.
 * Returns 1 for a match, 0 otherwise.
 */
uint8_t match_name(uint8_t *matchstr, cbmdirent_t *dent, uint8_t ignorecase) {
  uint8_t *filename = dent->name;
  uint8_t *starpos;
  uint8_t m,f;
  uint8_t chars_remain = 16;

#if 0
  /* Shortcut for chaining fastloaders ("!*file") */
  if (*filename == *matchstr && matchstr[1] == '*')
    return 1;
#endif

  while (chars_remain && *filename) {
    if (ignorecase) {
      m = tolower_pet(*matchstr);
      f = tolower_pet(*filename);
    } else {
      m = *matchstr;
      f = *filename;
    }
    switch (m) {
    case '?':
      filename++;
      matchstr++;
      chars_remain--;
      break;

    case '*':
      if (globalflags & POSTMATCH) {
        starpos = matchstr;
        matchstr += ustrlen(matchstr)-1;
        filename += ustrlen(filename)-1;
        while (matchstr != starpos) {
          if (ignorecase) {
            m = tolower_pet(*matchstr);
            f = tolower_pet(*filename);
          } else {
            m = *matchstr;
            f = *filename;
          }
          if (m != f && m != '?')
            return 0;
          filename--;
          matchstr--;
        }
      }
      return 1;

    default:
      if (m != f)
        return 0;
      matchstr++;
      filename++;
      chars_remain--;
      break;
    }
  }
  if (*matchstr && *matchstr != '*' && chars_remain != 0)
    return 0;
  else
    return 1;
}

/**
 * next_match - get next matching directory entry
 * @dh        : directory handle
 * @matchstr  : pattern to be matched
 * @start     : start date
 * @end       : end date
 * @type      : required file type (0 for any)
 * @dent      : pointer to a directory entry for returning the match
 *
 * This function looks for the next directory entry matching matchstr and
 * type (if != 0) and returns it in dent. Return values of the function are
 * -1 if no match could be found, 1 if an error occured or 0 if a match was
 * found.
 */
int8_t next_match(dh_t *dh, uint8_t *matchstr, date_t *start, date_t *end, uint8_t type, cbmdirent_t *dent) {
  int8_t res;

  while (1) {
    res = readdir(dh, dent);
    if (res == 0) {
      /* Skip if the type doesn't match */
      if ((type & TYPE_MASK) &&
          (dent->typeflags & TYPE_MASK) != (type & TYPE_MASK))
        continue;

      /* Skip hidden files */
      if ((dent->typeflags & FLAG_HIDDEN) &&
          !(type & FLAG_HIDDEN))
        continue;

      /* Skip if the name doesn't match */
      if (matchstr) {
        if (dent->opstype == OPSTYPE_FAT) {
          /* FAT: Ignore case */
          if (!match_name(matchstr, dent, 1))
            continue;
        } else {
          /* Honor case */
          if (!match_name(matchstr, dent, 0))
            continue;
        }
      }

      /* skip if earlier than start date */
      if (start &&
          memcmp(&dent->date, start, sizeof(date_t)) < 0)
        continue;

      /* skip if later than end date */
      if (end &&
          memcmp(&dent->date, end, sizeof(date_t)) > 0)
        continue;
    }

    return res;
  }
}

/**
 * first_match - get the first matching directory entry
 * @path    : pointer to a path object
 * @matchstr: pattern to be matched
 * @type    : required file type (0 for any)
 * @dent    : pointer to a directory entry for returning the match
 *
 * This function looks for the first directory entry matching matchstr and
 * type (if != 0) in path and returns it in dent. Uses matchdh for matching
 * and returns the same values as next_match. This function is just a
 * convenience wrapper around opendir+next_match, it is not required to call
 * it before using next_match.
 */
int8_t first_match(path_t *path, uint8_t *matchstr, uint8_t type, cbmdirent_t *dent) {
  int8_t res;

  if (opendir(&matchdh, path))
    return 1;

  res = next_match(&matchdh, matchstr, NULL, NULL, type, dent);
  if (res < 0)
    set_error(ERROR_FILE_NOT_FOUND);
  return res;
}

/**
 * parse_path - parse CMD style directory specification
 * @in    : input buffer
 * @path  : pointer to path object
 * @name  : pointer to pointer to filename (may be NULL)
 * @for_cd: parse path from CD command
 *
 * This function parses a CMD style directory specification in the input
 * buffer. If successful, the path object will be set up for accessing
 * the path named in the input buffer. Returns 0 if successful or 1 if an
 * error occured.
 * If @for_cd is true, no colon is required in the input string and parsing
 * is ended successfully if the last component in the path is an image file.
 */
uint8_t parse_path(uint8_t *in, path_t *path, uint8_t **name, uint8_t for_cd) {
  cbmdirent_t dent;
  uint8_t *end;
  uint8_t saved;
  uint8_t part;

  if (for_cd || ustrchr(in, ':')) {
    /* Skip partition number */
    part=parse_partition(&in);
    if(part>=max_part) {
      set_error(ERROR_DRIVE_NOT_READY);
      return 1;
    }

    path->part = part;
    path->dir  = partition[part].current_dir;

    if (*in != '/') {
      *name = ustrchr(in, ':');
      if (*name == NULL)
        *name = in;
      else
        *name += 1;
      return 0;
    }

    while (*in) {
      switch (*in++) {
      case '/':
        switch (*in) {
        case '/':
          /* Double slash -> root */
          dent.name[0] = 0;
          chdir(path, &dent);
          break;

        case 0:
          /* End of path found, no name */
          *name = in;
          return 0;

        case ':':
          /* End of path found */
          *name = in+1;
          return 0;

        default:
          /* Extract path component and match it */
          end = in;
          while (*end && *end != '/' && *end != ':') end++;
          saved = *end;
          *end = 0;
          if (first_match(path, in, FLAG_HIDDEN, &dent)) {
            /* first_match has set an error already */
            if (current_error == ERROR_FILE_NOT_FOUND)
              set_error(ERROR_FILE_NOT_FOUND_39);
            return 1;
          }

          if ((dent.typeflags & TYPE_MASK) != TYPE_DIR) {
            /* Not a directory */
            /* FIXME: Try to mount as image here so they can be accessed like a directory */
            if (for_cd && saved == 0 && check_imageext(in) != IMG_UNKNOWN) {
              /* no further path components, last one is an image file */
              *name = in;
              return 0;
            } else {
              set_error(ERROR_FILE_NOT_FOUND_39);
              return 1;
            }
          }

          /* Match found, move path */
          chdir(path, &dent);
          *end = saved;
          in = end;
          break;
        }
        break;

      case 0:
        /* End of path found, no name */
        *name = in-1;  // -1 to fix the ++ in switch
        return 0;

      case ':':
        /* End of path found */
        *name = in;
        return 0;
      }
    }
  } else {
    /* No :, use current dir/path */
    path->part = current_part;
    path->dir  = partition[current_part].current_dir;
  }

  *name = in;
  return 0;
}



/* Parse a decimal number at str and return a pointer to the following char */
uint16_t parse_number(uint8_t **str) {
  uint16_t res = 0;

  /* Skip leading spaces */
  while (**str == ' ') (*str)++;

  /* Parse decimal number */
  while (isdigit(**str)) {
    res *= 10;
    res += (*(*str)++) - '0';
  }

  return res;
}

/* Parse a CMD-style date and return a pointer to the following char */
uint8_t parse_date(date_t *date, uint8_t **str) {
  uint8_t ch;

  date->month = parse_number(str);
  if (date->month > 12 || *(*str)++ != '/') return 1;

  date->day = parse_number(str);
  if (date->day > 31 || *(*str)++ != '/') return 1;

  date->year = parse_number(str);
  /* Y2K */
  if (date->year < 80)
    date->year += 100;

  /* Shortcut: Just a date without time */
  if (!**str || *(*str) == ',') {
    date->hour = 0;
    date->minute = 0;
    date->second = 0;
    return 0;
  }
  if (*(*str)++ != ' ') return 1;

  date->hour = parse_number(str);
  ch = *(*str)++;
  if (date->hour > 23 || (ch != ':' && ch != '.')) return 1;

  date->minute = parse_number(str);
  ch = *(*str)++;
  if (date->minute > 59) return 1;

  switch (ch) {
  case ':':
  case '.':
    date->second = parse_number(str);
    if (date->second > 59 || *(*str)++ != ' ') return 1;
    break;

  case ' ':
    date->second = 0;
    break;

  case ',':
  case 0:
    /* No AM/PM */
    (*str)--;
    date->second = 0;
    return 0;

  default:
    return 1;
  }

  switch(*(*str)++) {
  case 'A':
    break;
  case 'P':
    date->hour+=12;
    break;
  default:
    return 1;
  }
  if (date->hour > 23)
    return 1;

  if (*(*str)++ != 'M') return 1; // JLB need to check to see if CMD is this anal
  return 0;
}
