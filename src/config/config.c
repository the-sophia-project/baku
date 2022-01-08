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

#include "config.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

#define SYSTEM_WIDE_CONFIG "/etc/baku/baku.conf"

bool DoesConfigExist() {
  FILE *config = fopen(SYSTEM_WIDE_CONFIG, "r");

  if (config == NULL) {
    if (errno == ENOENT) {
      return false;
    }
  }

  fclose(config);
  return true;
}

char *GetValueByKey(char *key) {
  FILE *config = fopen(SYSTEM_WIDE_CONFIG, "r");
  if (!config) {
    return NULL;
  }

  wchar_t *buffer = NULL;
  while (!feof(config)) {
    getline(&buffer, 0, config);
    printf("%s", buffer);
    buffer = NULL;
    break;
  }
}