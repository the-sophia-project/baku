/*
 * Copyright © 2021 Michał Fudali
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BAKU_SRC_CPIO_COMMON_H_
#define BAKU_SRC_CPIO_COMMON_H_

//MAGIC defined in POSIX header has bad value for us.
#ifdef MAGIC
#undef MAGIC
#endif
#define MAGIC "070701"

typedef struct {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_checksum[8];
} CpioHeader;

#define FIELDS_IN_HEADER 14

static inline unsigned char CalculateFrontPadding(CpioHeader cpio_header, size_t filename_size) {
  unsigned char padding_length = 4 - (sizeof(cpio_header) + filename_size) % 4;
  return padding_length == 4 ? 0 : padding_length;
}

static inline unsigned char CalculateRearPadding(size_t filesize) {
  unsigned char padding_length = 4 - filesize % 4;
  return padding_length == 4 ? 0 : padding_length;
}

#endif //BAKU_SRC_CPIO_COMMON_H_
