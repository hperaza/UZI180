/*******************************************************
 * UZI (Unix Z80 Implementation) Utilities:  insboot.c *
 * Installs a boot sector on floppy UZI disks.         *
 * Copyright (C) 2001, Hector Peraza.                  *
 *******************************************************/

#include <stdio.h>
#include <string.h>
#include <unix.h>


long lseek(uchar, long, uchar);

void usage(char *progname)
{
    fprintf(stderr, "usage: %s devname [-l loader] kernel [cmdline]\n", progname);
    exit(1);
}


main(argc, argv)
int argc;
char *argv[];
{
    int _open(), _read(), _close(), dread(), dwrite();
    int dev, fd;
    struct stat statbuf;
    dinode *ino;
    int _stat();
    int i, j, datofs, inode, block, offset;
    char *devname, *kernel, *loader, bdev, chksum;
    blkno_t *bp, map[96];  /* 48K max for kernel */
    static unsigned char buf[512];


    if (argc < 3) usage(argv[0]);

    devname = argv[1];
    loader = "loader";

    for (i = 2; i < argc; ++i) {
	if (strcmp(argv[i], "-l") == 0) {
	    if (++i == argc) usage(argv[0]);
	    loader = argv[i];
	    break;
	}
    }
    
    if (++i == argc) usage(argv[0]);
    kernel = argv[i];

    bdev = (++i < argc) ? argv[i][0] : '\0';

    dev = _open(devname, O_RDWR);
    if (dev < 0) {
        fprintf(stderr, "%s: can't open %s\n", argv[0], devname);
        exit(1);
    }

    if (_stat(kernel, &statbuf) != 0) {
      printf("%s: can't stat %s\n", argv[0], kernel);
      exit(1);
    }
    
    inode = statbuf.st_ino;
    block = inode / 8 + 2;
    offset = (inode % 8) * 64;
    
    dread(dev, block, buf);
    ino = (dinode *) &buf[offset];

    /* 1st indirection blocks */
    for (j = i = 0; i < 18; ++i) {
      if ((map[j++] = ino->i_addr[i]) == 0) break;
    }

    if ((block = ino->i_addr[18]) != 0) {
      /* 2nd indirection blocks */
      dread(dev, block, buf);
      bp = (blkno_t *) buf;
      while (*bp) {
        if (j == 96) {
          fprintf(stderr, "%s: kernel image too big\n", argv[0]);
          exit(1);
        }
        if ((map[j++] = *bp++) == 0) break;
      }
    } else {
      map[j++] = 0;
    }

    fd = _open(loader, O_RDONLY);
    if (fd < 0) {
        printf("%s: can't open %s\n", argv[0], loader);
        exit(1);
    }
    _read(fd, buf, 512);
    _close(fd);
    
    if (buf[0] != 0xC3) {
        printf("%s: bad loader signature\n", argv[0]);
        exit(1);
    }
    
    datofs = (buf[3] & 0xFF) + (buf[4] << 8);
    
    buf[datofs] = bdev;

    bp = (blkno_t *) &buf[datofs+1];
    for (i = 0; i < j; ++i) *bp++ = map[i];
    *bp = (blkno_t) 0;
    
    for (i = 0, chksum = 0; i < 511; ++i) chksum += buf[i];
    buf[511] = -chksum;

    dwrite(dev, 0, buf);
    printf("%s: boot sector installed\n", argv[0]);
    
    _close(dev);

    exit(0);
}


dread(dev, blk, buf)
int dev;
uint16 blk;
char *buf;
{
    int _read();
        
    lseek(dev, blk * 512L, 0);
    _read(dev, buf, 512);
}


dwrite(dev, blk, buf)
int dev;
uint16 blk;
char *buf;
{
    int _write();
    
    lseek(dev, blk * 512L, 0);
    _write(dev, buf, 512);
}
