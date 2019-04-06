/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  scall1b.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  19.12.97 - Divided SCALL1 for Hi-Tech Compiler.	     HFB
 *  14.05.01 - Corrected a bug in readi()                    HP
 */
/*LINTLIBRARY*/

#include <unix.h>
#include <config.h>
#include <extern.h>


readi (ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 toread;
    register blkno_t pblk;
    register char *bp;
    int      dev;
    int      ispipe;
    char     *bread();
    char     *zerobuf();
    blkno_t  bmap();
    unsigned psize(),cdread();
    int      min();

    dev = ino->c_dev;
    ispipe = 0;
    switch (getmode (ino))
    {

    case F_DIR:
    case F_REG:

        /* See if end of file will limit read */

        if (ino->c_node.i_size.o_blkno < udata.u_offset.o_blkno) {
            udata.u_count = 0;
        } else if ((ino->c_node.i_size.o_blkno == udata.u_offset.o_blkno) &&
                   (ino->c_node.i_size.o_offset < udata.u_offset.o_offset)) {
            udata.u_count = 0;
        } else {
            udata.u_count = ino->c_node.i_size.o_blkno - udata.u_offset.o_blkno >= 64 ?
                udata.u_count : min(udata.u_count,
                  512 * (ino->c_node.i_size.o_blkno - udata.u_offset.o_blkno) +
                        (ino->c_node.i_size.o_offset - udata.u_offset.o_offset));
        }
        toread = udata.u_count;
        goto loop;

    case F_PIPE:
        ispipe = 1;
        while (psize (ino) == 0)
        {
            if (ino->c_refs == 1)      /* No writers */
                break;
            /* Sleep if empty pipe */
            psleep (ino);
        }
        toread = udata.u_count = min (udata.u_count, psize (ino));
        goto loop;

    case F_BDEV:
        toread = udata.u_count;
        dev = *(ino->c_node.i_addr);

    loop:
        while (toread)
        {
            if ((pblk = bmap (ino, udata.u_offset.o_blkno, 1)) != NULLBLK)
                bp = bread (dev, pblk, 0);
            else
                bp = zerobuf();

            amount = min (toread, 512 - udata.u_offset.o_offset);

              /* if System is in context or destination is in Common memory
               * (F000-FFFF is always in context), simply copy the data.
               * Otherwise perform Interbank move (uput) to User bank.
               */
            if (udata.u_sysio || (udata.u_base >= (char *)0xf000))
                bcopy (bp+udata.u_offset.o_offset, udata.u_base, amount);
            else
                uput (bp+udata.u_offset.o_offset, udata.u_base, amount);

            brelse (bp);

            udata.u_base += amount;
            addoff (&udata.u_offset, amount);
            if (ispipe && udata.u_offset.o_blkno >= 18)
                udata.u_offset.o_blkno = 0;
            toread -= amount;
            if (ispipe)
            {
                addoff (&(ino->c_node.i_size), -amount);
                wakeup (ino);
            }
        }
        break;

    case F_CDEV:
        udata.u_count = cdread (ino->c_node.i_addr[0]);

        if (udata.u_count != -1)
            addoff (&udata.u_offset, udata.u_count);
        break;

    default:
        udata.u_error = ENODEV;
    }
}



/* Writei (and readi) need more i/o error handling */

writei (ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 towrite;
    register char *bp;
    int      ispipe;
    blkno_t  pblk;
    int      dev;
    char     *bread();
    blkno_t  bmap();
    unsigned cdwrite(),psize();
    int      min();

    dev = ino->c_dev;

    switch (getmode (ino))
    {

    case F_BDEV:
        dev = *(ino->c_node.i_addr);
    case F_DIR:
    case F_REG:
        ispipe = 0;
        towrite = udata.u_count;
        goto loop;

    case F_PIPE:
        ispipe = 1;
        while ((towrite = udata.u_count) > (16*512) - psize (ino))
        {
            if (ino->c_refs == 1)        /* No readers */
            {
                udata.u_count = -1;
                udata.u_error = EPIPE;
                ssig (udata.u_ptab, SIGPIPE);
                return;
            }
            /* Sleep if empty pipe */
            psleep (ino);
        }
        /* Sleep if empty pipe */
        goto loop;

    loop:

        while (towrite)
        {
            amount = min (towrite, 512 - udata.u_offset.o_offset);

            if ((pblk = bmap (ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
                break;                     /* No space to make more blocks */

            /* If we are writing an entire block, we don't care
             * about its previous contents
             */
            bp = bread (dev, pblk, (amount == 512));

              /* if System is in context or destination is in Common memory
               * (F000-FFFF is always in context), simply copy the data.
               * Otherwise perform Interbank move (uput) to User bank.
               */
            if (udata.u_sysio || (udata.u_base >= (char *) 0xf000))
                bcopy (udata.u_base, bp+udata.u_offset.o_offset, amount);
            else
                uget (udata.u_base, bp+udata.u_offset.o_offset, amount);

            bawrite (bp);

            udata.u_base += amount;
            addoff (&udata.u_offset, amount);
            if (ispipe)
            {
                if (udata.u_offset.o_blkno >= 18)
                    udata.u_offset.o_blkno = 0;
                addoff (&(ino->c_node.i_size), amount);
                /* Wake up any readers */
                wakeup (ino);
            }
            towrite -= amount;
        }

        /* Update size if file grew */
        ifnot (ispipe)
        {
            if (udata.u_offset.o_blkno > ino->c_node.i_size.o_blkno ||
               (udata.u_offset.o_blkno == ino->c_node.i_size.o_blkno &&
                    udata.u_offset.o_offset > ino->c_node.i_size.o_offset))
            {
                ino->c_node.i_size.o_blkno = udata.u_offset.o_blkno;
                ino->c_node.i_size.o_offset = udata.u_offset.o_offset;
                ino->c_dirty = 1;
            }
        }
        break;

    case F_CDEV:
        udata.u_count = cdwrite (ino->c_node.i_addr[0]);

        if (udata.u_count != -1)
            addoff (&udata.u_offset, udata.u_count);
        break;

    default:
        udata.u_error = ENODEV;
    }
}
