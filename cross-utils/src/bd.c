/**************************************************
UZI (Unix Z80 Implementation) Utilities:  bd.c
***************************************************/
/*
 *  Block Dump : To Examine Hard Disk.
 *
 *  Usage:  bd dev blkno
 */

#include "unix.h"
#include <config.h>		/* 1.4.98 - HFB */
#include "extern.h"

#define off_t
#include <stdio.h>

char buf[512];

main(argc, argv)
int argc;
char *argv[];
{
    int i, j;
    unsigned blkno;
    int dev;
    char junk, ch;
    int atoi(), isdigit();	/* 1.4.98 - HFB */

    if (argc != 3) {
	fprintf(stderr, "Usage: bd device blkno\n");
	exit(1);
    }
    _setfdImageName(argv[1]);
    dev = 1;
                
    xfs_init(dev);

    blkno = atoi(argv[2]);
    bufinit();
    d_open(dev);

    dread(dev, blkno, buf);

    for (i = 0; i < 512 / 16; ++i) {
#if 0
	printf("%4x  ", 16 * i);
#else
	printf("%04x  ", 16 * i);
#endif
	for (j = 0; j < 16; ++j) {
#if 0
	    if ((buf[16 * i + j] & 0x00ff) < 16)
		printf("0%1x ", buf[16 * i + j] & 0x00ff);
	    else
		printf("%2x ", buf[16 * i + j] & 0x00ff);
#else
	    printf("%02x ", buf[16 * i + j] & 0xff);
#endif
	}
	printf("    ");
	for (j = 0; j < 16; ++j) {
	    ch = (buf[16 * i + j] & 0x00ff);
	    if ((ch >= ' ') && (ch < 0x7f))
		putchar(ch);
	    else
		printf(".");
	}
	printf("\n");
	if (((i + 1) % 16 == 0) && (i < 31)) {
	    printf("[return for more]");
	    getchar();		/* (junk) -- HP */
	    printf("\n");
	}
    }
    exit(0);
}



dread(dev, blk, addr)
int dev;
uint16 blk;
char *addr;
{
    char *buf;
    char *bread();

    buf = bread(dev, blk, 0);
    bcopy(buf, addr, 512);
    bfree(buf, 0);
}
