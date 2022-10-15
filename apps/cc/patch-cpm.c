#include <stdio.h>

unsigned char b1, b2, b3, pgm[65536];

int main(int argc, char *argv[]) {
  FILE *f;
  int  cnt, cntw;

  if (argc != 3) {
    fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
    return 1;
  }

  f = fopen(argv[1], "r");
  if (f == NULL) {
    fprintf(stderr, "can't open file input %s\n", argv[1]);
    return 1;
  }
  cnt = fread(pgm, 1, 65536, f);
  fclose(f);
  printf("%d (0x%x) bytes read\n", cnt, cnt);

  b1 = pgm[0];
  b2 = pgm[1];
  b3 = pgm[2];

  if (b1 == 0xc3) {
    printf("no patch needed\n");
    return 0;
  }

  pgm[0] = 0xc3;
  pgm[1] = (cnt + 0x100) & 0xff;
  pgm[2] = ((cnt + 0x100) >> 8) & 0xff;

  pgm[cnt]   = 0x3e;
  pgm[cnt+1] = b1;
  pgm[cnt+2] = 0x32;
  pgm[cnt+3] = 0x00;
  pgm[cnt+4] = 0x01;

  pgm[cnt+5] = 0x3e;
  pgm[cnt+6] = b2;
  pgm[cnt+7] = 0x32;
  pgm[cnt+8] = 0x01;
  pgm[cnt+9] = 0x01;

  pgm[cnt+10] = 0x3e;
  pgm[cnt+11] = b3;
  pgm[cnt+12] = 0x32;
  pgm[cnt+13] = 0x02;
  pgm[cnt+14] = 0x01;
  
  pgm[cnt+15] = 0xc3;
  pgm[cnt+16] = 0x00;
  pgm[cnt+17] = 0x01;

  cnt += 128;

  f = fopen(argv[2], "w");
  if (f == NULL) {
    fprintf(stderr, "can't open file output %s\n", argv[2]);
    return 1;
  }
  cntw = fwrite(pgm, 1, cnt, f);
  if (cntw != cnt) {
    fprintf(stderr, "error writing output file\n");
  }
  fclose(f);
  printf("%d (0x%x) bytes written\n", cntw, cntw);

  return 0;
}
