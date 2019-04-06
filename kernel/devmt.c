/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devmisc.c
   Magnetic Tape, Unimplemented.  Currently a Null Device.
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 */


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
