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

#include <stdio.h>
#include <string.h>

#include "install/install.h"

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("Baku package manager needs more arguments.\n");
  } else {
    if (!strcmp(argv[1], "-i")) {
      printf("Beginning installation of %s.\n", argv[2]);
    }
  }
}