/***************************************************************
   UZI (Unix Z80 Implementation) Utilities:  xdevmisc.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 * 25.12.97 - lpout moved to machine-dependent module.       HFB
 * 16.7.98 - combined w/devmt.c to stripped Util version.    HFB
 */

#include <unix.h>
#include <config.h>
#include <extern.h>

mem_read (minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < 0x006C)
        return (1);
    bcopy ((char *)addr, udata.u_base, udata.u_count);
    return(udata.u_count);
}


mem_write (minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < 0x006C)
        return (1);
    bcopy (udata.u_base, (char *)addr, udata.u_count);
    return(udata.u_count);
}


null_write (minor, rawflag)
int minor;
int rawflag;
{
    return (udata.u_count);
}



static char lop = 0;

lpr_open()
{
    lop = 1;
    return (0);
}


lpr_close()
{
    if (lop)  {
        lop  = 0;
    }
    return(0);
}


lpr_write (minor, rawflag)
int minor;
int rawflag;
{
    unsigned n;
    unsigned char  c;

    n = udata.u_count;
    while (n--) {
        c = *udata.u_base++;
        lpout (c);
    }
    return (udata.u_count);
}

/* NOTE: lpout moved to Machine-dependent Assembly Module - HFB */


mt_read (minor, rawflag)
int minor;
int rawflag;
{
    return (-1);
}


mt_write (minor, rawflag)
int minor;
int rawflag;
{
    return (-1);
}


mt_open()
{
    return (-1);
}


mt_close()
{
    return (0);
}
