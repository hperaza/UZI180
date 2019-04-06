/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devflop.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 */

#include <unix.h>
#include <config.h>
#include <extern.h>

extern ei();

extern int fdInit (int);      /* Floppy Init Routine.  0 if drive exists */
extern int fdread0(), fdwrite0();
extern unsigned char ftrack, fsector, ferror;
extern char *fbuf;
extern struct drvTbl {
    unsigned char logged;
    unsigned char cbyte0;
    unsigned char cbyte1;
    unsigned char gap3;
    unsigned char spt;
    unsigned char sector1;
    unsigned char format;
    unsigned char spinup;
    unsigned char curtrk;
} dTbl[4];

int secsiz [4] = {128, 256, 512, 1024};		/* Table of Sector Sizes */


fd_read (minor, rawflag)
int16 minor;
int rawflag;
{
    int fd();
    return (fd (1, minor, rawflag));
}

fd_write (minor, rawflag)
int16 minor;
int rawflag;
{
    int fd();
    return (fd (0, minor, rawflag));
}


fd (rwflag, minor, rawflag)
int rwflag;
int minor;
int rawflag;
{
    register unsigned nblocks;
    register unsigned firstblk;

    if (rawflag)
    {
        if (rawflag == 2)
        {
            nblocks = swapcnt >> 9;
            fbuf = swapbase;
            firstblk = swapblk;
        }
        else
        {
            nblocks = udata.u_count >> 9;
            fbuf = udata.u_base;
            firstblk = udata.u_offset.o_blkno;
        }       
    }
    else
    {
        nblocks = 1;
        fbuf = udata.u_buf->bf_data;
        firstblk = udata.u_buf->bf_blk;
    }

    ftrack = firstblk / dTbl[minor].spt;
    fsector = firstblk % dTbl[minor].spt;	/* Base 0 Sect # */
    ferror = 0;

    for (;;)
    {
        if (rwflag)
            fdread0(minor);
        else
            fdwrite0(minor);

        ifnot (--nblocks)
            break;

        if (++fsector > dTbl[minor].spt)
        {
            fsector = 0;
            ++ftrack;
        }
        fbuf += secsiz[(unsigned int) (dTbl[minor].format & 0x03)];
    }

    if (ferror)
    {
        kprintf ("fd_%s: error %d track %d sector %d\n",
                    rwflag ? "read" : "write", ferror, ftrack, fsector);
        panic ("");
    }
    return (nblocks);
}


fd_open (minor)
int minor;
{
    if (fdInit (minor))
    {
        udata.u_error = ENXIO;
        return (-1);
    }
    return (0);
}


fd_close (minor)
int minor;
{
    dTbl[minor].logged = 0;	/* Mark Drive as logged out */
    return (0);
}


fd_ioctl (minor)
int minor;
{
    return (-1);
}
