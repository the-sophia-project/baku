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

#include "compress.h"

#define __USE_XOPEN_EXTENDED

#include <ftw.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "common.h"

#define FD_LIMIT 16

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

void AppendRecordToFile(const CpioHeader cpio_header,
                        const char *filename,
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

    if (S_ISDIR(file_status.st_mode)) {
      return;
    }

    long filesize = file_status.st_size;

    unsigned char rear_padding = CalculateRearPadding(filesize);

    unsigned char
        *file_data_with_padding = malloc(filesize + rear_padding);

    if (file_data_with_padding == NULL) {

      perror("Error when using malloc");
    } else {
      fread(file_data_with_padding, 1, filesize, source);
      memset(file_data_with_padding + filesize, '\0', rear_padding);

      fwrite(file_data_with_padding,
             1,
             filesize + rear_padding,
             destination);
    }

    free(file_data_with_padding);
  }
}

FILE *archive = NULL;

CpioHeader HeaderFromFileStatus(const char *file_path, const char *relative_path) {
  struct stat buf;

  CpioHeader cpio_header = {0};

  if (lstat(file_path, &buf)) {
    fprintf(stderr, "Error when processing %s \n", file_path);
    perror("Cannot get file status info");
    return cpio_header;
  }

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

  size_t namesize = strlen(relative_path) + 1;
  CopyFileStatusToHeaderField(&cpio_header.c_namesize, &namesize);
  memset(&cpio_header.c_checksum, '0', sizeof(cpio_header.c_checksum));

  return cpio_header;
}

int ProcessFile(const char *pathname, const struct stat *, int, struct FTW *ftw) {
  char *filename = strchr(pathname, '/') + 1;

  if (filename != (char *) 1) {
    CpioHeader file_header = HeaderFromFileStatus(pathname, filename);

    //If file_header wasn't changed by HeaderFromFileStatus, omit this file in archive
    if (archive == NULL || !memcmp((unsigned char *) &file_header.c_magic, "\0", 1)) {
      return 1;
    }

    FILE *source = fopen(pathname, "r");

    AppendRecordToFile(file_header, filename, source, archive);
    fclose(source);
  }

  return 0;
}

int *CompressDirectoryContent(const char *archive_path,
                              const char *source_dir) {

  archive = fopen(archive_path, "w+");

  nftw(source_dir, &ProcessFile, FD_LIMIT, FTW_PHYS);

  CpioHeader file_header = {0};
  memset(&file_header, '0', sizeof(file_header));
  memcpy(&file_header.c_magic, MAGIC, sizeof(file_header.c_magic));
  memcpy(&file_header.c_nlink, "00000001", sizeof(file_header.c_nlink));
  memcpy(&file_header.c_namesize, "0000000B", sizeof(file_header.c_namesize));
  AppendRecordToFile(file_header, "TRAILER!!!", NULL, archive);

  fclose(archive);
  archive = NULL;

  return 1;
}