/***************************************************************
   UZI (Unix Z80 Implementation)  flopimg.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/

/* Revisions:
 * 20.9.99 - Linux version -- HP
 */

/* #define USE_STDIO */

#include <stdio.h>

#ifdef USE_STDIO
#include "unix.h"
#include "config.h"
#include "extern.h"
#else
#undef O_RDWR
#undef O_RDONLY
#undef O_WRONLY
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif



#define IBMPC3  0xAE   /* HD,  DD, DS, 3.5",   512-byte Sctrs (1.44 MB) */
#define UZIHD3  0xAF   /* HD,  DD, DS, 3.5",  1024-byte Sctrs (1.76 MB) */
#define IBMPC5  0xAA   /* HD,  DD, DS, 5.25",  512-byte Sctrs (1.2 MB)  */
#define UZIHD5  0xAB   /* HD,  DD, DS, 5.25", 1024-byte Sctrs (1.44 MB) */
#define DSQD3   0x2F   /* MFM, DD, DS, 3.5",  1024-byte Sctrs (800 KB)  */
#define DSDD3   0x2E   /* MFM, DD, DS, 3.5",   512-byte Sctrs (800 KB)  */
#define DSQD5   0x2B   /* MFM, DD, DS, 5.25", 1024-byte Sctrs (800 KB)  */
#define DSDD5   0x2A   /* MFM, DD, DS, 5.25",  512-byte Sctrs (800 KB)  */
        
unsigned char dTbl[] = {   /* Drive Param Table.  1 Entry Per Drive. */
      0x00, 0xCF, 0x02, 27,  18,  1, IBMPC3, 16, 0,
/*      |     |     |    |    |   |     |     |  +- Current Track Number
        |     |     |    |    |   |     |     +---- Spinup (1/20-secs)
        |     |     |    |    |   |     +--- Format Byte (See above)
        |     |     |    |    |   +--------- First Sector Number
        |     |     |    |    +---------- Physical Sectors-Per-Track
        |     |     |    +--------------- Gap3 (Size 512=27, 1024=13)
        |     |     +-------------------- Hd Load in 4mS steps (0=inf)
        |     +-------------------------- Step Rate (B7-4), HUT (3-0)
        +-------------------------------- Drive Logged (FF), Unlogged (0)
*/
      0x00, 0xCF, 0x01, 27, 18, 1, IBMPC3, 10, 0,
      0x00, 0xCF, 0x01, 27, 18, 1, IBMPC3, 10, 0,
      0x00, 0xCF, 0x01, 27, 18, 1, IBMPC3, 10, 0
};

unsigned char ftrack, fsector, _ferror = 0;
unsigned char *fbuf;

FILE *uziImage = NULL;
int fd_image;

char _imageName[1024] = "uzi180.disk";
char imageBuffer[2000000];

/* #define USE_STDIO */

void _setfdImageName(char *name) {
  strcpy(_imageName, name);
}

int fdInit(int minor) {
#ifdef USE_STDIO
  uziImage = fopen(_imageName, "r");
  if (uziImage == NULL) return 1;
  fread(imageBuffer, 160 * 18, 512, uziImage);
  fclose(uziImage);
  return 0;
#else
  fd_image = open(_imageName, O_CREAT | O_RDWR, 0644);
  if (fd_image <= 0) {
    fprintf(stderr, "Could not open %s\n", _imageName);
    exit(1);
  }
  return 0;
#endif
}

fdread0() {
#ifdef USE_STDIO
  memcpy(fbuf, &imageBuffer[(ftrack * 18 + fsector) * 512], 512);
  _ferror = 0;
  return 0;
#else
  lseek(fd_image, (ftrack * 18 + fsector) * 512, SEEK_SET);
  read(fd_image, fbuf, 512);
  return 0;
#endif
}

fdwrite0() {
#ifdef USE_STDIO
  memcpy(&imageBuffer[(ftrack * 18 + fsector) * 512], fbuf, 512);
  uziImage = fopen(_imageName, "w");
  if (uziImage == NULL) return 1;
  fwrite(imageBuffer, 160 * 18, 512, uziImage);
  fclose(uziImage);
  _ferror = 0;
  return 0;
#else
  lseek(fd_image, (ftrack * 18 + fsector) * 512, SEEK_SET);
  write(fd_image, fbuf, 512);
  return 0;
#endif
}

scsiop() {
  panic("scsiop() not implemented\n");
  return 1;
}
