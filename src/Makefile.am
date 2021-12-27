SUBDIRS = cpio
bin_PROGRAMS = baku
baku_SOURCES = cpio/compress.c
AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = -std=c17 -Wpedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-truncation #-fsanitize=address -static-libasan -fno-omit-frame-pointer