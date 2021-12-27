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

#include "compress.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cpio.h>
#include <syslog.h>

#include <zlib.h>
#include <assert.h>

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

typedef struct {
  CpioHeader cpio_header_;
  char *filename_;
  unsigned char *file_data_
} CpioRecord;

/// \param n Number of bytes that should be read from the buffer
static char *NumberToAsciiHexString(unsigned char *buffer, size_t n) {
  unsigned char *output = malloc(n * 2 + 1);
  for (int i = 0; i < n; i++) {
    snprintf(output + i * 2, 3, "%02X", *(buffer + (n - 1 - i)));
  }

  printf("CC");

  return output;
}

char *CpioHeaderToHexString(CpioHeader cpio_header/*, char *filename, uint8_t file_data*/) {
  unsigned char *src = (unsigned char *) &cpio_header;
  int n = 6;
  char *archive = malloc(221);
  char *dest = archive;
  for (int i = 0; i < 14; i++) {
    char *hex = NumberToAsciiHexString(src, n);
    memcpy(dest, hex, n * 2);
    free(hex);
    src += 6;
    dest += 12;
    n = 8;
    if (i > 0) {
      src += 2;
      dest += 4;
    }
  }
  archive[220] = '\0';
  return archive;
}

unsigned char *WriteRecordToFile(const CpioHeader cpio_header,
                                 const char *const filename,
                                 const unsigned char *const file_data,
                                 const size_t file_data_size) {
  const char *cpio_header_hex = CpioHeaderToHexString(cpio_header);

  unsigned char front_padding = 4 - (strlen(cpio_header_hex) + strlen(filename)) % 4;
  if (front_padding == 4)
    front_padding = 0;

  unsigned char rear_padding = 4 - file_data_size % 4;

  if (rear_padding == 4)
    rear_padding = 0;

  size_t record_size = strlen(cpio_header_hex) + strlen(filename) +
      front_padding + file_data_size + rear_padding;
  const unsigned char *output = malloc(record_size);

  if (output != NULL) {
    memset(output, '\0', record_size);

    strcpy(output, cpio_header_hex);

    unsigned char *dest = strcpy(output + strlen(cpio_header_hex), filename) + strlen(filename) + front_padding;

    memcpy(dest, file_data, file_data_size);

    free(cpio_header_hex);

    return output;
  } else {
    perror("Error occurred when preparing record:");
    return NULL;
  }
}