/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  scall1c.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *   19.12.97 - Split SCALL1.C for Hi-Tech compiler, modified for
 *		UZI180, added explanatory documentation.     HFB
 *   08.10.99 - Corrected a bug in _access()                 HP
 */
/*LINTLIBRARY*/

#include <unix.h>
#include <config.h>
#include <extern.h>


int
min (a, b)
int a, b;
{
    return ( a < b ? a : b);
}


psize (ino)
inoptr ino;
{
    return (512*ino->c_node.i_size.o_blkno+ino->c_node.i_size.o_offset);
}


addoff (ofptr, amount)
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


updoff()
{
    register off_t *offp;

    /* Update current file pointer */
    offp = &of_tab[udata.u_files[udata.u_argn]].o_ptr;
    offp->o_blkno = udata.u_offset.o_blkno;
    offp->o_offset = udata.u_offset.o_offset;
}


/*******************************************
seek (file, offset, flag)         Function 9		?
int16 file;
uint16 offset;
int16 flag;
********************************************/
#define file   (int16)udata.u_argn
#define offset (uint16)udata.u_argn1
#define flag   (int16)udata.u_argn2

_seek()
{
    register inoptr ino;
    register int16 oftno;
    register uint16 retval;
    inoptr   getinode();
    unsigned getmode();

    if ((ino = getinode (file)) == NULLINODE)
        return (-1);

    if (getmode (ino) == F_PIPE)
    {
        udata.u_error = ESPIPE;
        return (-1);
    }

    oftno = udata.u_files[file];

     /* REMEMBER:
      * flag  0-2 return only offset within current block (512 bytes)
      *       3-5 return only current block number
      */
    if (flag <= 2)
        retval = of_tab[oftno].o_ptr.o_offset;
    else
        retval = of_tab[oftno].o_ptr.o_blkno;

    switch (flag)
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
        return (-1);
    }
    while ((unsigned)of_tab[oftno].o_ptr.o_offset >= 512)
    {
        of_tab[oftno].o_ptr.o_offset -= 512;
        ++of_tab[oftno].o_ptr.o_blkno;
    }

    return ((int16)retval);
}

#undef file
#undef offset
#undef flag


/*******************************************
chdir (dir)                      Function 10
char *dir;
********************************************/
#define dir (char *)udata.u_argn

_chdir()
{
    register inoptr newcwd;
    inoptr   n_open();
    unsigned getmode();

    ifnot (newcwd = n_open (dir, NULLINOPTR))
	return (-1);

    if (getmode (newcwd) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	i_deref (newcwd);
	return (-1);
    }
    i_deref (udata.u_cwd);
    udata.u_cwd = newcwd;
    return (0);
}

#undef dir


/*******************************************
mknod (name, mode, dev)           Function 4
char  *name;
int16 mode;
int16 dev;
********************************************/
#define name (char *)udata.u_argn
#define mode (int16)udata.u_argn1
#define dev  (int16)udata.u_argn2

_mknod()
{
    register inoptr ino;
    inoptr parent;
    inoptr n_open();
    inoptr newfile();
    int    isdevice();
    char   fname[15];

    udata.u_error = 0;

/*    ifnot (super()) */

    if (ino = n_open (name, &parent))
    {
	udata.u_error = EEXIST;
	goto nogood;
    }

    ifnot (parent)
    {
	udata.u_error = ENOENT;
        return (-1);
    }

    filename (name, fname);
    ifnot (*fname && (ino = newfile (parent, fname)))
/* BUG SN
 *        goto nogood2;
 *!!!!*/
        goto nogood3;       /* parent inode is derefed in newfile. SN */

    /* Initialize mode and dev */
    ino->c_node.i_mode = mode & ~udata.u_mask;
    ino->c_node.i_addr[0] = isdevice (ino) ? dev : 0;
    setftime (ino, A_TIME|M_TIME|C_TIME);
    wr_inode (ino);

    i_deref (ino);
    return (0);

nogood:
    i_deref (ino);
    i_deref (parent);
nogood3:
    return (-1);
}

#undef name
#undef mode
#undef dev


/*******************************************
sync()
********************************************/

_sync()
{
    register j;
    register inoptr ino;
    register char *buf;
    char     *bread();

    /* Write out modified inodes */

    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if (ino->c_refs > 0 && ino->c_dirty != 0)
	{
	    wr_inode (ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread (j, 1, 1);
	    bcopy ((char *)&fs_tab[j], buf, 512);
	    bfree (buf, 2);
	}
    }
    bufsync();          /* Clear buffer pool */
}


/*******************************************
access (path, mode)              Function 12		?
char  *path;
int16 mode;
********************************************/
#define path (char *)udata.u_argn
#define mode (int16)udata.u_argn1

_access()
{
    register inoptr ino;
    register int16 euid;
    register int16 egid;
    register int16 retval;
    inoptr   n_open();
    char     ugetc();

    if ((mode & 07) && !ugetc(path))
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    /* Temporarily make eff. id real id. */
    euid = udata.u_euid;
    egid = udata.u_egid;
    udata.u_euid = udata.u_ptab->p_uid;
    udata.u_egid = udata.u_gid;

    ifnot (ino = n_open (path, NULLINOPTR))
    {
	retval = -1;
	goto nogood;
    }

    retval = 0;
    if (~getperm (ino) & (mode&07))
    {
	udata.u_error = EPERM;
	retval = -1;
    }

    i_deref (ino);
nogood:
    udata.u_euid = euid;
    udata.u_egid = egid;

    return (retval);
}

#undef path
#undef mode


/*******************************************
chmod (path, mode)               Function 13
char  *path;
int16 mode;
********************************************/
#define path (char *)udata.u_argn
#define mode (int16)udata.u_argn1

_chmod()
{
    inoptr ino;
    inoptr n_open();
    int    super();

    ifnot (ino = n_open (path, NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref (ino);
	udata.u_error = EPERM;
	return (-1);
    }

    ino->c_node.i_mode = (mode & MODE_MASK) | (ino->c_node.i_mode & F_MASK);
    setftime (ino, C_TIME);
    i_deref (ino);
    return (0);
}

#undef path
#undef mode


/*******************************************
chown (path, owner, group)       Function 14		?
char *path;
int  owner;
int  group;
********************************************/
#define path (char *)udata.u_argn
#define owner (int16)udata.u_argn1
#define group (int16)udata.u_argn2

_chown()
{
    register inoptr ino;
    inoptr n_open();
    int    super();

    ifnot (ino = n_open (path, NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref (ino);
	udata.u_error = EPERM;
	return (-1);
    }
    ino->c_node.i_uid = owner;
    ino->c_node.i_gid = group;
    setftime (ino, C_TIME);
    i_deref (ino);
    return (0);
}

#undef path
#undef owner
#undef group


/*******************************************
utime (file, buf)                Function 43
char *file;
char *buf;
********************************************/
#define file (char *)udata.u_argn
#define buf (char *)udata.u_argn1

_utime()
{
    register inoptr ino;
    inoptr  n_open();
    int     valadr(), super();

    ifnot (valadr (buf, 2*sizeof (time_t)))
        return (-1);
    ifnot (ino = n_open (file, NULLINOPTR))
        return (-1);
    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
        i_deref (ino);
        udata.u_error = EPERM;
        return (-1);
    }
     /* If Source is in Common Memory (F000-FFFF is always in context),
      * simply move the data.  Otherwise perform interbank "Get".
      */
    if (buf >= (char *)0xf000)
        bcopy (buf, &(ino->c_node.i_atime), 2*sizeof (time_t));
    else
        uget (buf, &(ino->c_node.i_atime), 2*sizeof (time_t));
    setftime (ino, C_TIME);
    i_deref (ino);
    return (0);
}

#undef file
#undef buf


/*******************************************
stat (path, buf)                 Function 15
char *path;
char *buf;
********************************************/
#define path (char *)udata.u_argn
#define buf (char *)udata.u_argn1

_stat()
{
    register inoptr ino;
    inoptr  n_open();
/*--    int     valadr();--*/

/*--    ifnot (valadr (buf, sizeof (struct stat)))
/*--        return (-1);--*/
    ifnot (ino = n_open (path, NULLINOPTR))
	return (-1);
    stcpy (ino, buf);
    i_deref (ino);
    return (0);
}

#undef path
#undef buf


/*******************************************
fstat (fd, buf)                  Function 16
int16 fd;
char *buf;
********************************************/
#define fd (int16)udata.u_argn
#define buf (char *)udata.u_argn1

_fstat()
{
    register inoptr ino;
    inoptr  getinode();
    int     valadr();

    ifnot (valadr (buf, sizeof (struct stat)))
	return (-1);

    if ((ino = getinode (fd)) == NULLINODE)
	return (-1);

    stcpy (ino, buf);
    return (0);
}

#undef fd
#undef buf


/* Utility for stat and fstat */
stcpy (ino, buf)
inoptr ino;
char *buf;
{
      /*   !!! violently system-dependent !!! */
      /* If Destination is in Common Memory (F000-FFFF is always in context),
       * simply move, otherwise perform Interbank "uput" to User Bank.
       */
    if (buf >= (char *) 0xf000) {
        bcopy ((char *)&(ino->c_dev), buf, 12);
        bcopy ((char *)&(ino->c_node.i_addr[0]), buf+12, 2);
        bcopy ((char *)&(ino->c_node.i_size), buf+14, 16);
    }
    else {
        uput ((char *)&(ino->c_dev), buf, 12);
        uput ((char *)&(ino->c_node.i_addr[0]), buf+12, 2);
        uput ((char *)&(ino->c_node.i_size), buf+14, 16);
    }
}


/*******************************************
dup (oldd)                       Function 17
int16 oldd;
********************************************/
#define oldd (uint16)udata.u_argn

_dup()
{
    register int newd;
    inoptr  getinode();
    int     uf_alloc();

    if (getinode (oldd) == NULLINODE)
	return (-1);

    if ((newd = uf_alloc()) == -1)
	return (-1);

    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return (newd);
}

#undef oldd


/*******************************************
dup2 (oldd, newd)                Function 36		?
int16 oldd;
int16 newd;
********************************************/
#define oldd (int16)udata.u_argn
#define newd (int16)udata.u_argn1

_dup2()
{
    inoptr getinode();

    if (getinode (oldd) == NULLINODE)
	return (-1);

    if (newd < 0 || newd >= UFTSIZE)
    {
	udata.u_error = EBADF;
	return (-1);
    }

    ifnot (udata.u_files[newd] & 0x80)
	doclose (newd);

    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return (0);
}

#undef oldd
#undef newd


/*******************************************
umask (mask)                     Function 21		?
int mask;
********************************************/
#define mask (int16)udata.u_argn

_umask()
{
    register int omask;

    omask = udata.u_mask;
    udata.u_mask = mask & 0777;
    return (omask);
}

#undef mask


/* Special system call returns super-block of given filesystem
 * for users to determine free space, etc.  Should be replaced
 * with a sync() followed by a read of block 1 of the device.
 */
/*******************************************
getfsys (dev, buf)               Function 22
int16  dev;
struct filesys *buf;
********************************************/
#define dev (int16)udata.u_argn
#define buf (struct filesys *)udata.u_argn1

_getfsys()
{
    if (dev < 0 || dev >= NDEVS || fs_tab[dev].s_mounted != SMOUNTED)
    {
       udata.u_error = ENXIO;
       return (-1);
    }
    if (buf >= (struct filesys *)0xf000) /* If in Common Memory, simply move */
        bcopy ((char *)&fs_tab[dev], (char *)buf, sizeof (struct filesys));
    else
        uput ((char *)&fs_tab[dev], (char *)buf, sizeof (struct filesys));
    return (0);
}

#undef dev
#undef buf


/*******************************************
ioctl (fd, request, data)        Function 29
int  fd;
int  request;
char *data;
********************************************/
#define fd (int)udata.u_argn
#define request (int)udata.u_argn1
#define data (char *)udata.u_argn2

_ioctl()
{
    register inoptr ino;
    register int dev;
    inoptr  getinode();
    int     isdevice(), getperm(), d_ioctl();

    if ((ino = getinode (fd)) == NULLINODE)
	return (-1);

    ifnot (isdevice (ino))
    {
	udata.u_error = ENOTTY;
	return (-1);
    }

    ifnot (getperm (ino) & OTH_WR)
    {
	udata.u_error = EPERM;
	return (-1);
    }

    dev = ino->c_node.i_addr[0];

    if (d_ioctl (dev, request, data))
	return (-1);
    return (0);
}

#undef fd
#undef request
#undef data


/* This implementation of mount ignores the rwflag */
/*******************************************
mount (spec, dir, rwflag)        Function 33
char *spec;
char *dir;
int  rwflag;
********************************************/
#define spec (char *)udata.u_argn
#define dir (char *)udata.u_argn1
#define rwflag (int)udata.u_argn2

_mount()
{
    register inoptr sino, dino;
    register int dev;
    inoptr   n_open();
    int      d_open(), fmount(), super();
    unsigned getmode();

    ifnot (super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open (spec, NULLINOPTR))
	return (-1);

    ifnot (dino = n_open (dir, NULLINOPTR))
    {
	i_deref (sino);
	return (-1);
    }

    if (getmode (sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    if (getmode (dino) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];

    if ( dev >= NDEVS || d_open (dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (fs_tab[dev].s_mounted || dino->c_refs != 1 || dino->c_num == ROOTINODE)
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    _sync();

    if (fmount (dev, dino))
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    i_deref (dino);
    i_deref (sino);
    return (0);

nogood:
    i_deref (dino);
    i_deref (sino);
    return (-1);
}

#undef spec
#undef dir
#undef rwflag


/*******************************************
umount (spec)                    Function 34
char *spec;
********************************************/
#define spec (char *)udata.u_argn

_umount()
{
    register inoptr sino;
    register int dev;
    register inoptr ptr;
    inoptr   n_open();
    int      super(), validdev();
    unsigned getmode();

    ifnot (super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open (spec, NULLINOPTR))
	return (-1);

    if (getmode (sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];
    ifnot (validdev (dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (!fs_tab[dev].s_mounted)
    {
	udata.u_error = EINVAL;
	goto nogood;
    }

    for (ptr = i_tab; ptr < i_tab+ITABSIZE; ++ptr)
	if (ptr->c_refs > 0 && ptr->c_dev == dev)
	{
	    udata.u_error = EBUSY;
	    goto nogood;
	}

    _sync();

    fs_tab[dev].s_mounted = 0;
    i_deref (fs_tab[dev].s_mntpt);
    i_deref (sino);
    return (0);

nogood:
    i_deref (sino);
    return (-1);
}

#undef spec
