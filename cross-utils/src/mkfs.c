/**************************************************
UZI (Unix Z80 Implementation) Utilities:  mkfs.c
***************************************************/
/* This utility creates an UZI file system for a defined block device.
 * the format is:
 *                  mkfs device isize fsize
 * where: device is the logical block on which to install a filesystem
 *        isize is the number of 512-byte blocks (less two reserved for
 *            system use) to use for storing 64-byte i-nodes
 *        fsize is the total number of 512-byte blocks to assign to the
 *            filesystem.  Space available for storage of data is therefore
 *            fsize-isize blocks.
 * This utility uses some modified files from the UZI kernel, and some
 * older, simplified files.  It is compiled and linked with CP/M libraries
 * and executed as a CP/M program.
 *
 * Revisions:
 *   Based on UZI280 version (minor changes from original UZI.  HFB
 */

#include <stdio.h>
#include <unix.h>
#include <config.h>		/* 1.4.98 - HFB */
#include <extern.h>

int dev;

/*extern char zerobuf(); */
direct dirbuf[32] = { ROOTINODE, ".", ROOTINODE, ".." };
struct dinode inode[8];


main(argc, argv)
int argc;
char *argv[];
{
    uint16 fsize, isize;
    int atoi(), yes(), d_open();

    if (argc != 4) {
	printf("Usage: mkfs device isize fsize\n");
	exit(-1);
    }
    dev = atoi(argv[1]);
    isize = (uint16) atoi(argv[2]);
    fsize = (uint16) atoi(argv[3]);

    if (dev == 0 && argv[1][0] != '0') {
	printf("Invalid device\n");
	exit(-1);
    }
    if (dev < 0 || dev >= NDEVS) {
	printf("Invalid device\n");
	exit(-1);
    }
    if (fsize < 3 || isize < 2 || isize >= fsize) {
	printf("Bad parameter values\n");
	exit(-1);
    }
    printf("Making filesystem on device %d with isize %u fsize %u. Confirm? ",
	   dev, isize, fsize);
    if (!yes())
	exit(-1);

    bufinit();
    if (d_open(dev)) {
	printf("Can't open device\n");
	exit(-1);
    }
    mkfs(fsize, isize);
    bufsync();
    exit(0);
}


mkfs(fsize, isize)
uint16 fsize, isize;
{
    uint16 j;
    char *zeros;
    char *zerobuf();

    /* Zero out the blocks */
    printf("Zeroizing i-blocks...\n");
    zeros = zerobuf();		/* Allocate a block, zero filled */
#ifdef CPM
    for (j = 0; j < isize; ++j)	/* Don't waste time in CP/M */
	dwrite(j, zeros);
#else
    for (j = 0; j < fsize; ++j)
	dwrite(j, zeros);
#endif
    brelse(zeros);		/* Free the zeroized buffer */

    /* Initialize the super-block */
    fs_tab[dev].s_mounted = SMOUNTED;	/* Magic number */
    fs_tab[dev].s_isize = isize;
    fs_tab[dev].s_fsize = fsize;
    fs_tab[dev].s_nfree = 1;
    fs_tab[dev].s_free[0] = 0;
    fs_tab[dev].s_tfree = 0;
    fs_tab[dev].s_ninode = 0;
    fs_tab[dev].s_tinode = (8 * (isize - 2)) - 2;

    /* Free each block, building the free list */

    printf("Building free list...\n");
    for (j = fsize - 1; j > isize; --j) {
	if (fs_tab[dev].s_nfree == 50) {
	    dwrite(j, (char *) &fs_tab[dev].s_nfree);
	    fs_tab[dev].s_nfree = 0;
	}
	++fs_tab[dev].s_tfree;
	fs_tab[dev].s_free[(fs_tab[dev].s_nfree)++] = j;
    }

    /* The inodes are already zeroed out */
    /* create the root dir */

    inode[ROOTINODE].i_mode = F_DIR | (0777 & MODE_MASK);
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size.o_blkno = 0;
    inode[ROOTINODE].i_size.o_offset = 32;
    inode[ROOTINODE].i_addr[0] = isize;

    /* Reserve reserved inode */
    inode[0].i_nlink = 1;
    inode[0].i_mode = ~0;

    printf("Writing initial inode and superblock...\n");

    bufsync();
    dwrite(2, (char *) inode);
    dwrite(isize, (char *) dirbuf);

    bufsync();
    /* Write out super block */
    dwrite(1, (char *) &fs_tab[dev]);

/*==    bufsync();==*/
    printf("Done.\n");
}


dwrite(blk, addr)
uint16 blk;
char *addr;
{
    char *buf;
    char *bread();

    buf = bread(dev, blk, 1);
    bcopy(addr, buf, 512);
    bfree(buf, 1);
}


int yes()
{
    char line[20];
    /* int  fgets(); - HP */

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}
