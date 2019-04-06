/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  scall1a.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  19.12.97 - Split SCALL1.C for Hi-Tech Compiler.	     HFB
 */

/*LINTLIBRARY*/
#include <unix.h>
#include <config.h>
#include <extern.h>

/*******************************************
open (name, flag)                 Function 1
char *name;
register int16 flag;
********************************************/
#define name (char *)udata.u_argn
#define flag (int16)udata.u_argn1

_open()
{
    int16    uindex;
    register int16 oftindex;
    register inoptr ino;
    register int16 perm;
    inoptr   n_open();
    int      uf_alloc(), oft_alloc(), getperm(), isdevice(), d_open();
    unsigned getmode();

    if (flag < 0 || flag > 2)
    {
        udata.u_error = EINVAL;
        return (-1);
    }
    if ((uindex = uf_alloc()) == -1)
        return (-1);

    if ((oftindex = oft_alloc()) == -1)
        goto nooft;

    ifnot (ino = n_open (name, NULLINOPTR))
        goto cantopen;

    of_tab[oftindex].o_inode = ino;

    perm = getperm (ino);
    if (((flag == O_RDONLY || flag == O_RDWR) && !(perm & OTH_RD)) ||
        ((flag == O_WRONLY || flag == O_RDWR) && !(perm & OTH_WR)))
    {
        udata.u_error = EPERM;
        goto cantopen;
    }

    if (getmode (ino) == F_DIR &&
        (flag == O_WRONLY || flag == O_RDWR))
    {
        udata.u_error = EISDIR;
        goto cantopen;
    }

    if (isdevice (ino) && d_open ((int)ino->c_node.i_addr[0]) != 0)
    {
        udata.u_error = ENXIO;
        goto cantopen;
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_access = flag;

/*
 *         Sleep process if no writer or reader
 */
    if (getmode (ino) == F_PIPE && of_tab[oftindex].o_refs == 1 )
        psleep (ino);

    return (uindex);

cantopen:
    oft_deref (oftindex);      /* This will call i_deref() */
nooft:
    udata.u_files[uindex] = -1;
    return (-1);
}

#undef name
#undef flag



/*******************************************
close (uindex)                    Function 2
int16 uindex;
********************************************/
#define uindex (int16)udata.u_argn

_close()
{
        int doclose();

    return (doclose (uindex));
}

#undef uindex


doclose (uindex)
int16 uindex;
{
    register int16 oftindex;
    inoptr ino;
    inoptr getinode();
    int    isdevice();

    ifnot (ino = getinode (uindex))
        return (-1);
    oftindex = udata.u_files[uindex];

    if (isdevice (ino))
/*        && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1) */
        d_close ((int)(ino->c_node.i_addr[0]));

    udata.u_files[uindex] = -1;
    oft_deref (oftindex);

    return (0);
}



/*******************************************
creat (name, mode)                Function 3
char  *name;
int16 mode;
********************************************/
#define name (char *)udata.u_argn
#define mode (int16)udata.u_argn1

_creat()
{
    register inoptr ino;
    register int16 uindex;
    register int16 oftindex;
    inoptr   parent;
    register int16 j;
    inoptr   n_open();
    inoptr   newfile();
    int      uf_alloc(), oft_alloc(), getperm();
    unsigned getmode();

    char fname[15];

    parent = NULLINODE;

    if ((uindex = uf_alloc()) == -1)
        return (-1);
    if ((oftindex = oft_alloc()) == -1)
        return (-1);

    if (ino = n_open (name, &parent))  /* The file exists */
    {
        i_deref(parent);

        if (getmode (ino) == F_DIR)
        {
            i_deref (ino);
            udata.u_error = EISDIR;
            goto nogood;
        }
        ifnot (getperm (ino) & OTH_WR)
        {
            i_deref (ino);
            udata.u_error = EACCES;
            goto nogood;
        }
        if (getmode (ino) == F_REG)
        {
            /* Truncate the file to zero length */
            f_trunc (ino);
            /* Reset any oft pointers */
            for (j=0; j < OFTSIZE; ++j)
                if (of_tab[j].o_inode == ino)
                    of_tab[j].o_ptr.o_blkno = of_tab[j].o_ptr.o_offset = 0;
        }
    }
    else
    {
        /* Get trailing part of name into fname */
        filename (name, fname);
        if (parent && *fname && (ino = newfile (parent, fname)))
                 /* Parent was derefed in newfile */
        {
            ino->c_node.i_mode = (F_REG | (mode & MODE_MASK & ~udata.u_mask));
            setftime (ino, A_TIME|M_TIME|C_TIME);
            /* The rest of the inode is initialized in newfile() */
            wr_inode (ino);
        }
        else
        {
            /* Doesn't exist and can't make it */
            goto nogood;
        }
    }
    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_inode = ino;
    of_tab[oftindex].o_access = O_WRONLY;

    return (uindex);

nogood:
    oft_deref (oftindex);
    return (-1);
}

#undef name
#undef mode



/*******************************************
pipe (fildes)                    Function 40		?
int fildes[];
********************************************/
#define fildes (int *)udata.u_argn

_pipe()
{
    register int16 u1, u2, oft1, oft2;
    register inoptr ino;
    inoptr   i_open();
    int      uf_alloc(), oft_alloc();

/* bug fix SN */
    if ((u1 = uf_alloc()) == -1)
        goto nogood;
    if ((oft1 = oft_alloc()) == -1)
        goto nogood;
    udata.u_files[u1] = oft1;

    if ((u2 = uf_alloc()) == -1)
        goto nogood2;
    if ((oft2 = oft_alloc()) == -1)
        goto nogood2;

    ifnot (ino = i_open (ROOTDEV, 0))
    {
        oft_deref (oft2);
        goto nogood2;
    }

    udata.u_files[u2] = oft2;

    of_tab[oft1].o_ptr.o_offset = 0;
    of_tab[oft1].o_ptr.o_blkno = 0;
    of_tab[oft1].o_inode = ino;
    of_tab[oft1].o_access = O_RDONLY;

    of_tab[oft2].o_ptr.o_offset = 0;
    of_tab[oft2].o_ptr.o_blkno = 0;
    of_tab[oft2].o_inode = ino;
    of_tab[oft2].o_access = O_WRONLY;

    ++ino->c_refs;
    ino->c_node.i_mode = F_PIPE | 0777; /* No permissions necessary on pipes */
    ino->c_node.i_nlink = 0;            /* a pipe is not in any directory */

#if 0
    *fildes = u1;			/* 280 is: uputw(u1, fildes)   */
    *(fildes+1) = u2;			/* 280 is: uputw(u2, fildes+1) */
#else
    uputw(u1, fildes);
    uputw(u2, fildes+1);
#endif
    return (0);

nogood2:
    oft_deref (oft1);
    udata.u_files[u1] = -1;
nogood:
    return (-1);
}

#undef fildes



/*******************************************
link (name1, name2)               Function 5
char *name1;
char *name2;
********************************************/
#define name1 (char *)udata.u_argn
#define name2 (char *)udata.u_argn1

_link()
{
    register inoptr ino;
    register inoptr ino2;
    inoptr   parent2;
    inoptr   n_open();
    int      ch_link(), super();
    unsigned getperm();
    char     fname[15];

    ifnot (ino = n_open (name1, NULLINOPTR))
	return (-1);

/*    if (getmode(ino) == F_DIR && !super()) */
    if (!(getperm (ino) & OTH_WR) && !super())
    {
        udata.u_error = EPERM;
        goto nogood;
    }

      /* Make sure file2 doesn't exist, and get its parent */
    if (ino2 = n_open (name2, &parent2))
    {
        i_deref (ino2);
        i_deref (parent2);
        udata.u_error = EEXIST;
        goto nogood;
    }

    ifnot (parent2)
        goto nogood;

    if (ino->c_dev != parent2->c_dev)
    {
        i_deref (parent2);
        udata.u_error = EXDEV;
        goto nogood;
    }

    filename (name2, fname);
    ifnot (*fname && ch_link (parent2, "", fname, ino))
    {
/* BUG SN */
        i_deref (parent2);
/* !!! */
        goto nogood;
    }

      /* Update the link count. */
    ++ino->c_node.i_nlink;
    wr_inode (ino);
    setftime (ino, C_TIME);

    i_deref (parent2);
    i_deref (ino);
    return (0);

nogood:
    i_deref (ino);
    return (-1);
}

#undef name1
#undef name2



/*******************************************
unlink (path)                     Function 6
char *path;
********************************************/
#define path (char *)udata.u_argn

_unlink()
{
    register inoptr ino;
    inoptr   pino;
    inoptr   n_open();
    int      super(), ch_link();
    unsigned getperm();
    char     fname[15];

    ino = n_open (path, &pino);

/* BUG !!
 *  resulting in too many i-refs to pino for non existing files.
 * Fixed SN */
    ifnot (pino && ino)
    {
/* FIX ... */
        if (pino)       /* parent exist */
            i_deref (pino);
/* ... FIX end */
        udata.u_error = ENOENT;
        return (-1);
    }

/*    if (getmode (ino) == F_DIR && !super()) */

      /* Owner of file can overwrite R/O mode
       * ROOT has no restrictions
       */
    if (ino->c_node.i_uid != udata.u_euid  &&
        !(getperm (ino) & OTH_WR)  &&  !super())
    {
        udata.u_error = EPERM;
        goto nogood;
    }

      /* Remove the directory entry */
    filename (path, fname);
    ifnot (*fname && ch_link (pino, fname, "", NULLINODE))
        goto nogood;

      /* Decrease the link count of the inode */
    ifnot (ino->c_node.i_nlink--)
    {
        ino->c_node.i_nlink += 2;
        warning("_unlink: bad nlink");
    }
    setftime (ino, C_TIME);
    i_deref (pino);
    i_deref (ino);
    return (0);

nogood:
    i_deref (pino);
    i_deref (ino);
    return (-1);
}

#undef path



/*******************************************
read (d, buf, nbytes)             Function 7
int16  d;
char   *buf;
uint16 nbytes;
********************************************/
#define d (int16)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn2

_read()
{
    register inoptr ino;
    inoptr  rwsetup();

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup (1)) == NULLINODE)
        return (-1);          /* bomb out if error */

    readi (ino);
    updoff();

    return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes


/*******************************************
write (d, buf, nbytes)            Function 8
int16  d;
char   *buf;
uint16 nbytes;
********************************************/
#define d (int16)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn2

_write()
{
    register inoptr ino;
    inoptr rwsetup();

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup (0)) == NULLINODE)
        return (-1);          /* bomb out if error */

    writei (ino);
    updoff();

    return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes



inoptr
rwsetup (rwflag)
int  rwflag;
{
    register inoptr ino;
    register struct oft *oftp;
    inoptr getinode();

    udata.u_sysio = 0;                         /* I/O to user data space */
    udata.u_base  = (char *)udata.u_argn1;     /* buf */
    udata.u_count = (uint16)udata.u_argn2;     /* nbytes */

    if ((ino = getinode (udata.u_argn)) == NULLINODE)
        return (NULLINODE);

    oftp = of_tab + udata.u_files[udata.u_argn];
    if (oftp->o_access == (rwflag ? O_WRONLY : O_RDONLY))
    {
        udata.u_error = EBADF;
        return (NULLINODE);
    }
    setftime (ino, rwflag ? A_TIME : (A_TIME | M_TIME | C_TIME));

    /* Initialize u_offset from file pointer */
    udata.u_offset.o_blkno = oftp->o_ptr.o_blkno;
    udata.u_offset.o_offset = oftp->o_ptr.o_offset;

    return (ino);
}
