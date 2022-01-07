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

#ifndef BAKU_SRC_FILE_FORMAT_H_
#define BAKU_SRC_FILE_FORMAT_H_

struct rpmlead {
  unsigned char magic[4];
  unsigned char major, minor;
  short type;
  short archnum;
  char name[66];
  short osnum;
  short signature_type;
  char reserved[16];
};

struct rpmheader {
  unsigned char magic[4];
  unsigned char reserved[4];
  int nindex;
  int hsize;
};

struct rpmhdrindex {
  int tag;
  int type;
  int offset;
  int count;
};

#endif //BAKU_SRC_FILE_FORMAT_H_
