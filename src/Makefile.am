SUBDIRS = cpio
bin_PROGRAMS = baku
baku_SOURCES = cpio/compress.c cpio/decompress.c #test.c
AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = -std=c17 -Wpedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-truncation #-fsanitize=address -static-libasan
#CFLAGS = -g -O0