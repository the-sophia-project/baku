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
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <sys/sysmacros.h>
#include <libgen.h>

#include "common.h"

static char *CopyFileStatusToHeaderField(unsigned char *dest, unsigned char *source) {
  unsigned char already_written = 0;
  dest += 8;
  while (already_written < 8) {
    dest -= 2;
    unsigned char byte_in_hex_string[3];
    snprintf(byte_in_hex_string, 3, "%02X", *(source));
    already_written += 2;
    memcpy(dest, byte_in_hex_string, 2);
    source++;
  }
}

#define COPY_TO_HEADER(dest_name, src_name, field_suffix) \
  CopyFileStatusToHeaderField(&dest_name.c_##field_suffix, &src_name.st_##field_suffix)

CpioHeader HeaderFromFileStatus(const char *const file_path) {
  struct stat buf;

  const size_t filename_size = strlen(basename(file_path));

  CpioHeader cpio_header = {0};

  if (lstat(file_path, &buf)) {
    perror("Cannot get file status info");
  } else {
    strncpy(&cpio_header.c_magic, MAGIC, sizeof(cpio_header.c_magic + 1));

    COPY_TO_HEADER(cpio_header, buf, ino);
    COPY_TO_HEADER(cpio_header, buf, mode);
    COPY_TO_HEADER(cpio_header, buf, uid);
    COPY_TO_HEADER(cpio_header, buf, gid);
    COPY_TO_HEADER(cpio_header, buf, nlink);
    COPY_TO_HEADER(cpio_header, buf, mtime);
    CopyFileStatusToHeaderField(&cpio_header.c_filesize, &buf.st_size);

    unsigned long devmajor = major(buf.st_dev);
    unsigned long devminor = minor(buf.st_dev);

    unsigned long rdevmajor = major(buf.st_rdev);
    unsigned long rdevminor = minor(buf.st_rdev);

    CopyFileStatusToHeaderField(&cpio_header.c_devmajor, &devmajor);
    CopyFileStatusToHeaderField(&cpio_header.c_devminor, &devminor);
    CopyFileStatusToHeaderField(&cpio_header.c_rdevmajor, &rdevmajor);
    CopyFileStatusToHeaderField(&cpio_header.c_rdevminor, &rdevminor);
    CopyFileStatusToHeaderField(&cpio_header.c_namesize, &filename_size);
    memset(&cpio_header.c_checksum, '0', sizeof(cpio_header.c_checksum));
  }

  return cpio_header;
}

void *AppendRecordToFile(const CpioHeader cpio_header,
                         const char *const filename,
                         const FILE *source,
                         FILE *destination) {

  fwrite(&cpio_header, 1, sizeof(cpio_header), destination);
  fwrite(filename, sizeof(filename[0]), strlen(filename) + 1, destination);

  unsigned char front_padding = CalculateFrontPadding(cpio_header, strlen(filename) + 1);
  if (front_padding) {
    unsigned char *padding = calloc(front_padding, sizeof(unsigned char));
    fwrite(padding, sizeof(unsigned char), front_padding, destination);
    free(padding);
  }

  if (strcmp(filename, "TRAILER!!!") != 0) {

    struct stat file_status = {0};
    fstat(source->_fileno, &file_status);
    long filesize = file_status.st_size;;

    unsigned char rear_padding = CalculateRearPadding(filesize);

    unsigned char *file_data = malloc(filesize);
    fread(file_data, 1, filesize, source);

    unsigned char
        *file_data_with_padding = malloc(filesize + rear_padding);

    if (file_data_with_padding == NULL) {

      perror("Error when using malloc");
    } else {
      memcpy(file_data_with_padding, file_data, filesize);
      memset(file_data_with_padding + filesize, '\0', rear_padding);

      fwrite(file_data_with_padding,
             sizeof(file_data_with_padding[0]),
             filesize + rear_padding,
             destination);
    }

    free(file_data);
    free(file_data_with_padding);
  }
}

void *CompressFiles(const char *const archive_path,
                    const char **const file_paths,
                    unsigned char files_count) {

  CpioHeader fileHeader = {0};
  FILE *archive = fopen(archive_path, "w+");
  for (unsigned char i = 0; i < files_count; i++) {
    fileHeader = HeaderFromFileStatus(file_paths);
    const unsigned char *const filename = basename(file_paths + i);
    FILE *source = fopen(file_paths + i, "r");
    AppendRecordToFile(fileHeader, filename, source, archive);
    fclose(source);
    fileHeader = (CpioHeader) {0};
  }

  memset(&fileHeader, '0', sizeof(fileHeader));
  memcpy(&fileHeader.c_magic, MAGIC, sizeof(fileHeader.c_magic));
  memcpy(&fileHeader.c_nlink, "00000001", sizeof(fileHeader.c_nlink));
  memcpy(&fileHeader.c_namesize, "0000000B", sizeof(fileHeader.c_namesize));
  AppendRecordToFile(fileHeader, "TRAILER!!!", NULL, archive);

  fclose(archive);
}