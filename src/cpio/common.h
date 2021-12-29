#ifndef BAKU_SRC_CPIO_COMMON_H_
#define BAKU_SRC_CPIO_COMMON_H_

//MAGIC defined in POSIX header has bad value for us.
#ifdef MAGIC
  #undef MAGIC
#endif
#define MAGIC "070701"

typedef struct {
  char c_magic[6];
  char c_ino[8];
  char c_mode[8];
  char c_uid[8];
  char c_gid[8];
  char c_nlink[8];
  char c_mtime[8];
  char c_filesize[8];
  char c_devmajor[8];
  char c_devminor[8];
  char c_rdevmajor[8];
  char c_rdevminor[8];
  char c_namesize[8];
  char c_checksum[8];
} CpioHeader;

#endif //BAKU_SRC_CPIO_COMMON_H_
