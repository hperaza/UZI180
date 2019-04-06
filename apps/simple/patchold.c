/***************************************************************
 * Utility to patch old UZI180 executables so they would run   *
 * with kernel 1.5.1 or later.                                 *
 ***************************************************************/

#include <stdio.h>
#include <string.h>
#include <unix.h>


long lseek(uchar, long, uchar);
long _ltell();
int  open(char *, int);
int  read(int, unsigned char *, int);
int  write(int, unsigned char *, int);


#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2

int main(int argc, char *argv[]) {
  int fd;
  unsigned char bfr[6];


  if (argc != 2) {
    fprintf(stderr, "usage: %s filename\n", argv[0]);
    return 1;
  }

  fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "%s: can't open file %s\n", argv[0], argv[1]);
    return 1;
  }

  /* check for UZI180 signature */

  lseek(fd, 3L, SEEK_SET);
  if (_ltell(fd) != 3L) {
    fprintf(stderr, "%s: seek error\n", argv[0]);
    return -1;
  }

  if (read(fd, bfr, 6) != 6) {
    fprintf(stderr, "%s: read error from %s\n", argv[0], argv[1]);
    return 1;
  }

  if (strncmp((char *) bfr, "UZI180", 6) != 0) {
    fprintf(stderr, "%s: bad executable signature on %s\n", argv[0], argv[1]);
    return 1;
  }

  /* check for known values and patch if neccessary */

  lseek(fd, 0x28L, SEEK_SET);
  if (_ltell(fd) != 0x28L) {
    fprintf(stderr, "%s: seek error\n", argv[0]);
    return -1;
  }

  if (read(fd, bfr, 4) != 4) {
    fprintf(stderr, "%s: read error from %s\n", argv[0], argv[1]);
    return 1;
  }

  if ((bfr[0] == 0xE1) && (bfr[1] == 0xD1) &&
      (bfr[2] == 0xE5) && (bfr[3] == 0xD5)) {
    bfr[0] = bfr[1] = bfr[2] = bfr[3] = 0;
  } else if ((bfr[0] == 0) && (bfr[1] == 0) &&
             (bfr[2] == 0) && (bfr[3] == 0)) {
    fprintf(stderr, "%s: file %s is already patched\n", argv[0], argv[1]);
    return 0;
  } else if ((bfr[0] == 0xCD) && (bfr[3] == 0xE5)) {
    fprintf(stderr, "%s: file %s does not need patching\n", argv[0], argv[1]);
    return 0;
  } else {
    fprintf(stderr, "%s: failed to find byte pattern on %s, cannot patch\n",
                    argv[0], argv[1]);
    return 1;
  }

  lseek(fd, 0x28L, SEEK_SET);
  if (_ltell(fd) != 0x28L) {
    fprintf(stderr, "%s: seek error\n", argv[0]);
    return -1;
  }

  if (write(fd, bfr, 4) != 4) {
    fprintf(stderr, "%s: file write error\n", argv[0]);
    return -1;
  }

  fprintf(stdout, "%s: file %s successfully patched\n", argv[0], argv[1]);

  close(fd);

  return 0;
}
