/**************************************************
UZI (Unix Z80 Implementation) Utilities:  xfs1a.c
***************************************************/
/* Revisions:
 *  1.4.98 - Split xfs.c into parts for compilation with Hi-Tech C.   HFB
 */

/*LINTLIBRARY*/
#include "unix.h"
#include <config.h>
#include "extern.h"

extern char *bread();


_seek(file,offset,flag)
int16 file;
uint16 offset;
int16 flag;
{
    register inoptr ino;
    register uint16 retval;
    register int    oftno;
    inoptr   getinode();

    udata.u_error = 0;
    if ((ino = getinode(file)) == NULLINODE)
	return(-1);

    oftno = udata.u_files[file];

    if (flag <= 2)
	retval = of_tab[oftno].o_ptr.o_offset;
    else
	retval = of_tab[oftno].o_ptr.o_blkno;

    switch(flag)
    {
    case 0:
	of_tab[oftno].o_ptr.o_blkno = 0;
	of_tab[oftno].o_ptr.o_offset = offset;
	break;
    case 1:
	of_tab[oftno].o_ptr.o_offset += offset;
	break;
    case 2:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno;
	of_tab[oftno].o_ptr.o_offset = ino->c_node.i_size.o_offset + offset;
	break;
    case 3:
	of_tab[oftno].o_ptr.o_blkno = offset;
	break;
    case 4:
	of_tab[oftno].o_ptr.o_blkno += offset;
	break;
    case 5:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno + offset;
	break;
    default:
	udata.u_error = EINVAL;
	return(-1);
    }

    while ((unsigned)of_tab[oftno].o_ptr.o_offset >= 512)
    {
	of_tab[oftno].o_ptr.o_offset -= 512;
	++of_tab[oftno].o_ptr.o_blkno;
    }
    return((int16)retval);
}



readi(ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 toread;
    register blkno_t pblk;
    register char *bp;
    int dev;
    int ispipe;
    char *bread();
    char *zerobuf();
    blkno_t bmap();
    int     cdread(), min();

    dev = ino->c_dev;
    ispipe = 0;
    switch (getmode(ino))
    {

    case F_DIR:
    case F_REG:

	/* See of end of file will limit read */
	toread = udata.u_count =
	    ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno >= 64 ?
		udata.u_count :
		min(udata.u_count,
		 512*(ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno) +
		 (ino->c_node.i_size.o_offset-udata.u_offset.o_offset));
	goto loop;

    case F_BDEV:
	toread = udata.u_count;
        dev = *(ino->c_node.i_addr);

    loop:
	while (toread)
	{
	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 1)) != NULLBLK)
		bp = bread(dev, pblk, 0);
	    else
		bp = zerobuf();

	    bcopy(bp+udata.u_offset.o_offset, udata.u_base,
		    (amount = min(toread, 512 - udata.u_offset.o_offset)));
	    brelse(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if (ispipe && udata.u_offset.o_blkno >= 18)
		udata.u_offset.o_blkno = 0;
	    toread -= amount;
	}
	break;

    case F_CDEV:
	udata.u_count = cdread(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
        break;

    default:
	udata.u_error = ENODEV;
    }
}



/* Writei (and readi) need more i/o error handling */

writei(ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 towrite;
    register char *bp;
    int ispipe;
    blkno_t pblk;
/*--    int created;--*/	/* Set by bmap if newly allocated block used */
    int dev;
/*--    char *zerobuf();--*/
    char *bread();
    blkno_t bmap();
    int     cdwrite(), min();

    dev = ino->c_dev;

    switch (getmode(ino))
    {

    case F_BDEV:
        dev = *(ino->c_node.i_addr);
    case F_DIR:
    case F_REG:
  	ispipe = 0;
	towrite = udata.u_count;
	goto loop;

    loop:
	while (towrite)
	{
	    amount = min(towrite, 512 - udata.u_offset.o_offset);

	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
		break;    /* No space to make more blocks */

	    /* If we are writing an entire block, we don't care
	    about its previous contents */
	    bp = bread(dev, pblk, (amount == 512));

	    bcopy(udata.u_base, bp+udata.u_offset.o_offset, amount);
	    bawrite(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    towrite -= amount;
	}

	/* Update size if file grew */
	ifnot (ispipe)
	{
	    if ( udata.u_offset.o_blkno > ino->c_node.i_size.o_blkno ||
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
	udata.u_count = cdwrite(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }
}



addoff(ofptr, amount)     /* 1.4.98 - Simplified for Hi-Tech Compiler. HFB */
off_t *ofptr;
int amount;
{
        register int temp;

    if (amount >= 0)
    {
        temp = amount % 512;
        ofptr->o_offset += temp;
        if (ofptr->o_offset >= 512)
        {
            ofptr->o_offset -= 512;
            ++ofptr->o_blkno;
        }
        temp = amount / 512;
        ofptr->o_blkno += temp;
    }
    else
    {
        temp = (-amount) % 512;
        ofptr->o_offset -= temp;
        if (ofptr->o_offset < 0)
        {
            ofptr->o_offset += 512;
            --ofptr->o_blkno;
        }
        temp = (-amount) / 512;
        ofptr->o_blkno -= temp;
    }
}



updoff(d)
int d;
{
    register off_t *offp;

    /* Update current file pointer */
    offp = &of_tab[udata.u_files[d]].o_ptr;
    offp->o_blkno = udata.u_offset.o_blkno;
    offp->o_offset = udata.u_offset.o_offset;
}
