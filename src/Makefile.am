SUBDIRS = cpio

bin_PROGRAMS = baku
baku_SOURCES = main.c install/install.c
baku_LDADD = cpio/libcpio.la

AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = -std=c17 -Wpedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 -Wformat-truncation # -Werror -fsanitize=address -static-libasan
#CFLAGS = -g -O0 #CFLAGS should only be used when debugging takes place