/*
 * Copyright © 2022 Michał Fudali
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

#include <cpio.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
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

#define PATH_SIZE 255

typedef struct {
  CpioHeader header_;
  char filename_[PATH_SIZE + 1];
  unsigned char front_padding_;
  unsigned char *file_data_;
  unsigned char rear_padding_;
} Record;

typedef struct {
  ino_t inode_;
  const char path_[PATH_SIZE + 1];
} Inode;

void ExtractArchive(const char *dest, const char *archive) {

  FILE *src = fopen(archive, "r");

  Record record;

  //FIXME: This is illegitimate array size.
  Inode database[1024];
  memset(database, '\0', sizeof(database));

  while (1) {
    memset(&record, '\0', sizeof(record));
    fread(&record.header_, 1, strlen(MAGIC), src);

    if (memcmp(&record.header_, MAGIC, strlen(MAGIC))) {
      fprintf(stderr, "This is not an cpio SVR4 archive.");
      break;
    } else {
      for (int i = 0; i < FIELDS_IN_HEADER - 1; i++) {
        fread((unsigned char *) &record.header_ + 6 + (i * 8), 1, 8, src);
      }

      long long namesize = lstrtoll(&record.header_.c_namesize, NULL, 16, sizeof(record.header_.c_namesize));
      fread(&record.filename_, 1, namesize, src);

      if (!strcmp(record.filename_, "TRAILER!!!"))
        break;

      size_t n = strlen(dest) + strlen(record.filename_) + 3;
      char *dest_path = malloc(n);
      snprintf(dest_path, n, "%s/%s", dest, record.filename_);

      long long file_mode = lstrtoll(&record.header_.c_mode, NULL, 16, sizeof(record.header_.c_mode));
      long long inode = lstrtoll(&record.header_.c_ino, NULL, 16, sizeof(record.header_.c_ino));
      long long nlink = lstrtoll(&record.header_.c_nlink, NULL, 16, sizeof(record.header_.c_nlink));

      bool in_database = false;
      if (nlink > 1) {
        for (int i = 0; i < sizeof(database) / sizeof(Inode); i++) {
          if (!database[i].inode_) {
            database[i].inode_ = inode;
            strncpy(database[i].path_, dest_path, sizeof(database[i].path_));
          } else if (database[i].inode_ == inode) {
            in_database = true;
            link(database[i].path_, dest_path);
            break;
          }
        }
      }

      unsigned char front_padding = CalculateFrontPadding(record.header_, strlen(record.filename_) + 1);
      fseek(src, front_padding, SEEK_CUR);

      long long filesize = 0;

      if (!S_ISDIR(file_mode)) {
        filesize = lstrtoll(&record.header_.c_filesize, NULL, 16, 8);
      }

      if (!in_database) {
        char access_mode[2] = "w";
        if (S_ISDIR(file_mode)) {
          mkdir(dest_path, file_mode);
          strcpy(access_mode, "r");
        }

        FILE *file = fopen(dest_path, access_mode);

        free(dest_path);

        if (file == NULL) {
          perror("Error occurred when opening file for writing");
          break;
        }

        if (fchmod(file->_fileno, file_mode)) {
          perror("Error occurred when giving file appropriate permissions");
        }

        //FIXME: According to the LSB, values specified in RPMTAGs should take precedence when installing the package
        long long uid = lstrtoll(&record.header_.c_uid, NULL, 16, sizeof(record.header_.c_uid));
        long long gid = lstrtoll(&record.header_.c_gid, NULL, 16, sizeof(record.header_.c_gid));
        if (fchown(file->_fileno, uid, gid)) {
          perror("Error occurred when changing file owner and group.");
        }

        if (!S_ISDIR(file_mode)) {
          unsigned char *file_data = malloc(filesize);
          fread(file_data, 1, filesize, src);
          fwrite(file_data, 1, filesize, file);

          free(file_data);

          unsigned char rear_padding = CalculateRearPadding(filesize);
          fseek(src, rear_padding, SEEK_CUR);
        }
        fclose(file);
      } else {
        fseek(src, filesize, SEEK_CUR);
        free(dest_path);
      }
    }
  }

  fclose(src);
}