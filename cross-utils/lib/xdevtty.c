/**************************************************
UZI (Unix Z80 Implementation) Kernel:  xdevtty.c
***************************************************/
/* Revisions:
 *  1.4.98 - Adapted to UZI180.				HFB
 * 20.9.99 - Linux version for working with uzi images -- HP
 */


#include "unix.h"
#include <config.h>
extern struct u_data udata;

#define LINESIZ 132

char line[LINESIZ];

#define off_t
#include <stdio.h>


tty_read(minor, rawflag)
int16 minor;
int16 rawflag;
{
    int nread;

/*
    line[0] = udata.u_count;
    line[1] = 0;
    bdos(10, line);     / * Read console buffer * /
    bdos(2, '\n');
    nread = line[1];
    line[nread+2] = '\n';
    bcopy(line+2, udata.u_base, nread+1);
    return(nread+1);
*/
    fgets(line, udata.u_count, stdin);
    return strlen(line);
}


tty_write(minor, rawflag)
int16 minor;
int16 rawflag;
{
    while (udata.u_count-- != 0)
    {
/*
        if (*udata.u_base == '\n')
            bdos(2, '\r');
        bdos(2, *udata.u_base);
*/
        fputc(*udata.u_base, stdout);
        ++udata.u_base;
    }
}


_putc(c)
int c;
{
/*
    bdos(2, c);
*/
    fputc(c, stdout);
}


tty_open(minor)
int minor;
{
    return(0);
}


tty_close(minor)
int minor;
{
    return(0);
}


tty_ioctl(minor)
int minor;
{
    return(-1);
}
