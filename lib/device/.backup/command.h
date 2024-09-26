/*
 * vdrive-command.h - Virtual disk-drive implementation. Command interpreter.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef DEVICE_DISK_COMMAND_H
#define DEVICE_DISK_COMMAND_H

#include "cbmdos.h"

#include <cstdint>

class COMMAND
{
public:
struct vdrive_s;

void command_init(void);
int command_execute(struct vdrive_s *vdrive, const uint8_t *buf, unsigned int length);
int command_format(struct vdrive_s *vdrive, const char *disk_name);
int command_validate(struct vdrive_s *vdrive);
int command_set_error(struct vdrive_s *vdrive, int code, unsigned int track, unsigned int sector);
int command_memory_read(struct vdrive_s *vdrive, const uint8_t *buf, uint16_t addr, unsigned int length);
int command_memory_write(struct vdrive_s *vdrive, const uint8_t *buf, uint16_t addr, unsigned int length);
int command_memory_exec(struct vdrive_s *vdrive, const uint8_t *buf, uint16_t addr, unsigned int length);
int command_switch(struct vdrive_s *vdrive, int part);
void command_return(struct vdrive_s *vdrive, int origpart);
int command_switchtraverse(struct vdrive_s *vdrive, cbmdos_cmd_parse_plus_t *cmd);

uint8_t command_format_internal(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_format_worker(struct vdrive_s *vdrive, uint8_t *disk_name, uint8_t *disk_id);
uint8_t command_validate_internal(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_block(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_memory(vdrive_t *vdrive, uint8_t *buffer, unsigned int length);
uint8_t command_initialize(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_copy(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_chdir(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_mkdir(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_rmdir(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_chpart(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_chcmdpart(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_rename(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_renameheader(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_renamepart(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_scratch(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_position(vdrive_t *vdrive, uint8_t *buf, unsigned int length);
uint8_t command_u1a2b(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_lockunlock(vdrive_t *vdrive, cbmdos_cmd_parse_plus_t *cmd);
uint8_t command_time(vdrive_t *vdrive, uint8_t *cmd, int length);
uint8_t command_getpartinfo(vdrive_t *vdrive, const uint8_t *cmd, int length);
uint8_t command_deletepart(vdrive_t *vdrive, const uint8_t *cmd, int length);

};




#endif
