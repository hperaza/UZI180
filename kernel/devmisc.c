/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devmisc.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 * 25.12.97 - lpout moved to machine-dependent module. HFB
 * 02.10.99 - added /dev/zero and /dev/proc, corrected mem_write HP
 */

#include <unix.h>
#include <config.h>
#include <extern.h>


/*------------ /dev/kmem ------------*/

mem_read (minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < 0x006C)
        return (1);

    if (udata.u_sysio)
        bcopy ((char *)addr, udata.u_base, udata.u_count);
    else
        uput ((char *)addr, udata.u_base, udata.u_count);

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

    if (udata.u_sysio)
        bcopy (udata.u_base, (char *)addr, udata.u_count);
    else
        uget (udata.u_base, (char *)addr, udata.u_count);

    return(udata.u_count);
}


mem_ioctl(minor, request, data)
int minor;
int request;
char *data;
{
    switch (request) {
    default:
         udata.u_error = EINVAL;
         return (-1);
    }
    return 0;
}


/*------------ /dev/proc ------------*/

proc_read (minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = (unsigned) &ptab;
    addr += 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < (unsigned) ptab || addr >= (unsigned) (ptab + PTABSIZE))
        return (1);

    if (udata.u_sysio)
        bcopy((char *)addr, udata.u_base, udata.u_count);
    else
        uput((char *)addr, udata.u_base, udata.u_count);

    return(udata.u_count);
}


proc_write (minor, rawflag)
int minor;
int rawflag;
{
    udata.u_error = EINVAL;
    return (-1);
}


#define PIO_TABSIZE	1
#define PIO_ENTRYSIZE	2

proc_ioctl(minor, request, data)
int minor;
int request;
char *data;
{
    switch (request) {
    case PIO_TABSIZE:
         uputw(maxproc, data);
         break;

    case PIO_ENTRYSIZE:
         uputw(sizeof(struct p_tab), data);
         break;

    default:
         udata.u_error = EINVAL;
         return (-1);
    }
    return 0;
}


/*------------ /dev/null ------------*/

null_read (minor, rawflag)
int minor;
int rawflag;
{
    udata.u_error = EINVAL;
    return (-1);
}


null_write (minor, rawflag)
int minor;
int rawflag;
{
    return (udata.u_count);
}



/*------------ /dev/zero ------------*/

zero_read (minor, rawflag)
int minor;
int rawflag;
{
    if (udata.u_sysio)
        bzero(udata.u_base, udata.u_count);
    else
        uzero(udata.u_base, udata.u_count);

    return (udata.u_count);
}

zero_write (minor, rawflag)
int minor;
int rawflag;
{
    udata.u_error = EINVAL;
    return (-1);
}



/*------------ /dev/lp ------------*/

static char lop = 0;

lpr_open()
{
    lop = 1;
    return (0);
}


lpr_close()
{
    if (lop)
    {
        lop  = 0;
/*7Jul98        lpout('\f');**/
    }
    return(0);
}


lpr_write (minor, rawflag)
int minor;
int rawflag;
{
    unsigned n;
    unsigned char  c;
    int  ugetc();

    n = udata.u_count;
    while (n--) {
        if (udata.u_sysio)
            c = *udata.u_base++;
        else
            c = ugetc (udata.u_base++);
        lpout (c);
    }
    return (udata.u_count);
}

/* NOTE: lpout moved to Machine-dependent Assembly Module - HFB */

#include "devmt.c"
