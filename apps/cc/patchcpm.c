/***************************************************************
 * Utility to patch CP/M executable files as required by UZI's *
 * CP/M emulator. Copyright (C) 2000-2022, Hector Peraza.      *
 ***************************************************************/

#include <stdio.h>
#include <unistd.h>


//long lseek(uchar, long, uchar);
//long _ltell();
//int  open(char *, int);
//int  read(int, unsigned char *, int);
//int  write(int, unsigned char *, int);


//#define SEEK_SET  0
//#define SEEK_CUR  1
//#define SEEK_END  2

int main(int argc, char *argv[]) {
  FILE *f;
  long fsize;
  unsigned char b1, b2, b3, bfr[14];


  if (argc != 2) {
    fprintf(stderr, "usage: %s filename\n", argv[0]);
    return 1;
  }

  f = fopen(argv[1], "rwb");
  if (!f) {
    fprintf(stderr, "%s: can't open file %s\n", argv[0], argv[1]);
    return 1;
  }

  if (fread(bfr, 1, 3, f) != 3) {
    fprintf(stderr, "%s: read error from %s\n", argv[0], argv[1]);
    return 1;
  }

  b1 = bfr[0];
  b2 = bfr[1];
  b3 = bfr[2];

  if (b1 == 0xc3) {  /* jp already there */
    printf("%s: no patch needed\n", argv[0]);
    fclose(f);
    return 0;
  }

  fseek(f, 0L, SEEK_END);
  fsize = ftell(f);

  bfr[0] = 0x21;    /* ld hl,100h */
  bfr[1] = 0x00;
  bfr[2] = 0x01;
  
  bfr[3] = 0x36;    /* ld (hl),b1 */
  bfr[4] = b1;
  bfr[5] = 0x23;    /* inc hl */

  bfr[6] = 0x36;    /* ld (hl),b2 */
  bfr[7] = b2;
  bfr[8] = 0x23;    /* inc hl */

  bfr[9] = 0x36;    /* ld (hl),b3 */
  bfr[10] = b3;

  bfr[11] = 0xc3;   /* jp 100h */
  bfr[12] = 0x00;
  bfr[13] = 0x01;

  if (fwrite(bfr, 1, 14, f) != 14) {
    fprintf(stderr, "%s: file write error\n", argv[0]);
    return -1;
  }

  fseek(f, 0L, SEEK_SET);
  if (ftell(f) != 0L) {
    fprintf(stderr, "%s: seek error\n", argv[0]);
    return -1;
  }

  bfr[0] = 0xc3;    /* jp instruction to our patching code */
  bfr[1] = (fsize + 0x100) & 0xff;
  bfr[2] = ((fsize + 0x100) >> 8) & 0xff;

  if (fwrite(bfr, 1, 3, f) != 3) {
    fprintf(stderr, "%s: file write error\n", argv[0]);
    return -1;
  }

  fclose(f);

  return 0;
}
