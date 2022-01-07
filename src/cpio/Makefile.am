lib_LTLIBRARIES = libcpio.la
libcpio_la_SOURCES = cpio.h compress.c decompress.c
include_HEADERS = compress.h decompress.h

AM_CFLAGS = -std=c17 -Wpedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-truncation # -Werror -fsanitize=address -static-libasan
#CFLAGS = -g -O0 #CFLAGS should only be used when debugging takes place