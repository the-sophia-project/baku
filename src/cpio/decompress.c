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

#include "decompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "common.h"

/// Limited strtoll
/// \param n Number of characters to take account of
long long *lstrtoll(char *nptr, char **endptr, int base, unsigned long long n) {
  char *buffer = malloc(n + 1);
  buffer[n] = '\0';
  memcpy(buffer, nptr, n);
  long long output = strtoll(buffer, endptr, base);
  free(buffer);
  return output;
}

typedef struct {
  CpioHeader header_;
  char filename_[256];
  unsigned char front_padding_;
  unsigned char *file_data_;
  unsigned char rear_padding_;
} Record;

void DecompressArchive(const char *dest_dir, const char *archive_path) {

  FILE *src = fopen(archive_path, "r");
  struct stat file_status;
  fstat(src->_fileno, &file_status);

  Record record;

  while (1) {
    memset(&record, '\0', sizeof(record));
    fread(&record.header_, 1, strlen(MAGIC), src);

    if (memcmp(&record.header_, MAGIC, strlen(MAGIC))) {
      fprintf(stderr, "This is not an cpio SVR4 src.");
      break;
    } else {
      for (int i = 0; i < FIELDS_IN_HEADER - 1; i++) {
        fread((unsigned char *) &record.header_ + 6 + (i * 8), 1, 8, src);
      }

      int i = 0;
      do {
        fread((unsigned char *) &record.filename_ + i, 1, 1, src);
        i++;
      } while (record.filename_[i - 1] != 0);

      if (!strcmp(record.filename_, "TRAILER!!!"))
        break;

      size_t n = strlen(dest_dir) + strlen(archive_path) + 2;
      char *dest_file_path = malloc(n);
      snprintf(dest_file_path, n, "%s/%s", dest_dir, record.filename_);

      FILE *dest = fopen(dest_file_path, "w");
      free(dest_file_path);

      unsigned char front_padding = CalculateFrontPadding(record.header_, strlen(record.filename_) + 1);
      fseek(src, front_padding, SEEK_CUR);

      long long filesize = lstrtoll(&record.header_.c_filesize, NULL, 16, 8);

      unsigned char *file_data = malloc(filesize);
      fread(file_data, 1, filesize, src);
      fwrite(file_data, 1, filesize, dest);

      free(file_data);
      fclose(dest);

      unsigned char rear_padding = CalculateRearPadding(filesize);
      fseek(src, rear_padding, SEEK_CUR);
    }
  }
}
