/**************************************************
UZI (Unix Z80 Implementation) Utilities:  xfs1a1.c
***************************************************/

/*LINTLIBRARY*/
#include "unix.h"
#include <config.h>
#include "extern.h"


_mknod(name,mode,dev)
char *name;
int16 mode;
int16 dev;
{
    register inoptr ino;
    inoptr parent;
    inoptr n_open();
    inoptr newfile();
    int    super(), isdevice();

    udata.u_error = 0;
    ifnot (super())
    {
	udata.u_error = EPERM;
	return(-1);
    }

    if (ino = n_open(name,&parent))
    {
	udata.u_error = EEXIST;
	goto nogood;
    }
    
    ifnot (parent)
    {
	udata.u_error = ENOENT;
	return(-1);
    }

    ifnot (ino = newfile(parent,name))
	goto nogood2;

    /* Initialize mode and dev */
    ino->c_node.i_mode = mode & ~udata.u_mask;
    ino->c_node.i_addr[0] = isdevice(ino) ? dev : 0;
    setftime(ino, A_TIME|M_TIME|C_TIME);
    wr_inode(ino);

    i_deref(ino);
    return (0);

nogood:
    i_deref(ino);
nogood2:
    i_deref(parent);
    return (-1);
}



_sync()
{
    register j;
    register inoptr ino;
    register char *buf;
    char *bread();

    /* Write out modified inodes */

    udata.u_error = 0;
    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if ((ino->c_refs) > 0 && ino->c_dirty != 0)
	{
	    wr_inode(ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread(j, 1, 1);
	    bcopy((char *)&fs_tab[j], buf, 512);
	    bfree(buf, 2);
	}
    }
    bufsync();   /* Clear buffer pool */
}



_chdir(dir)
char *dir;
{
    register inoptr newcwd;
    inoptr n_open();
    int    getmode();

    udata.u_error = 0;
    ifnot (newcwd = n_open(dir,NULLINOPTR))
	return(-1);

    if (getmode(newcwd) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	i_deref(newcwd);
	return(-1);
    }
    i_deref(udata.u_cwd);
    udata.u_cwd = newcwd;
    return(0);
}



min(a, b)
int a, b;
{
    return ( a < b ? a : b);
}



_access(path,mode)
char *path;
int16 mode;
{
    register inoptr ino;
    register int16 euid;
    register int16 egid;
    register int16 retval;
    inoptr n_open();
    int    getperm();

    udata.u_error = 0;
    if ((mode & 07) && !*(path))
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    /* Temporarily make eff. id real id. */
    euid = udata.u_euid;
    egid = udata.u_egid;
    udata.u_euid = udata.u_ptab->p_uid;
    udata.u_egid = udata.u_gid;

    ifnot (ino = n_open(path,NULLINOPTR))
    {
	retval = -1;
	goto nogood;
    }
    
    retval = 0;
    if (~getperm(ino) & (mode&07))
    {
	udata.u_error = EPERM;
	retval = -1;
    }

    i_deref(ino);
nogood:
    udata.u_euid = euid;
    udata.u_egid = egid;

    return(retval);
}


_chmod(path,mode)
char *path;
int16 mode;
{
    inoptr ino;
    inoptr n_open();
    int    super();

    udata.u_error = 0;
    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }
    ino->c_node.i_mode = (mode & MODE_MASK) | (ino->c_node.i_mode & F_MASK);
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}


_chown(path, owner, group)
char *path;
int owner;
int group;
{
    register inoptr ino;
    inoptr n_open();
    int    super();

    udata.u_error = 0;
    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_uid = owner;
    ino->c_node.i_gid = group;
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}



_stat(path,buf)
char *path;
char *buf;
{
    register inoptr ino;
    inoptr n_open();
    int    valadr();

    udata.u_error = 0;
    ifnot (valadr(buf,sizeof(struct stat)) && (ino = n_open(path,NULLINOPTR)))
    {
	return (-1);
    }
    stcpy(ino,buf);
    i_deref(ino);
    return(0);
}



_fstat(fd, buf)
int16 fd;
char *buf;
{
    register inoptr ino;
    inoptr getinode();
    int    valadr();

    udata.u_error = 0;
    ifnot (valadr(buf,sizeof(struct stat)))
	return(-1);

    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    stcpy(ino,buf);
    return(0);
}



/* Utility for stat and fstat */
stcpy(ino, buf)
inoptr ino;
char *buf;
{
#if CPM
    /* violently system-dependent */
    bcopy((char *)&(ino->c_dev), buf, 12);
    bcopy((char *)&(ino->c_node.i_addr[0]), buf+12, 2);
    bcopy((char *)&(ino->c_node.i_size), buf+14, 16);
#else
    struct stat *b = (struct stat *)buf;

    b->st_dev = ino->c_dev;
    b->st_ino = ino->c_num;
    b->st_mode = ino->c_node.i_mode;
    b->st_nlink = ino->c_node.i_nlink;
    b->st_uid = ino->c_node.i_uid;
    b->st_gid = ino->c_node.i_gid;

    b->st_rdev = ino->c_node.i_addr[0];
    
    b->st_size.o_blkno = ino->c_node.i_size.o_blkno;
    b->st_size.o_offset = ino->c_node.i_size.o_offset;
    b->st_atime.t_time = ino->c_node.i_atime.t_time;
    b->st_atime.t_date = ino->c_node.i_atime.t_date;
    b->st_mtime.t_time = ino->c_node.i_mtime.t_time;
    b->st_mtime.t_date = ino->c_node.i_mtime.t_date;
    b->st_ctime.t_time = ino->c_node.i_ctime.t_time;
    b->st_ctime.t_date = ino->c_node.i_ctime.t_date;
#endif
}



_dup(oldd)
int16 oldd;
{
    register int newd;
    inoptr getinode();
    int    uf_alloc();

    udata.u_error = 0;
    if (getinode(oldd) == NULLINODE)
	return(-1);

    if ((newd = uf_alloc()) == -1)
	return (-1);
    
    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(newd);
}



_dup2(oldd, newd)
int16 oldd;
int16 newd;
{
    inoptr getinode();

    udata.u_error = 0;
    if (getinode(oldd) == NULLINODE)
	return(-1);

    if (newd < 0 || newd >= UFTSIZE)
    {
	udata.u_error = EBADF;
	return (-1);
    }
    
    ifnot (udata.u_files[newd] & 0x80)
	doclose(newd);
    
    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(0);
}



_umask(mask)
int mask;
{
    register int omask;

    udata.u_error = 0;
    omask = udata.u_mask;
    udata.u_mask = mask & 0777;
    return(omask);
}



/* Special system call returns super-block of given filesystem for
 * users to determine free space, etc.  Should be replaced with a
 * sync() followed by a read of block 1 of the device.
 */

_getfsys(dev,buf)
{
    udata.u_error = 0;
   if (dev < 0 || dev >= NDEVS || fs_tab[dev].s_mounted != SMOUNTED)
   {
       udata.u_error = ENXIO;
       return(-1);
    }

    bcopy((char *)&fs_tab[dev],(char *)buf,sizeof(struct filesys));
    return(0);
}



_ioctl(fd, request, data)
int fd;
int request;
char *data;
{
    register inoptr ino;
    register int dev;
    inoptr getinode();
    int    isdevice(), getperm(), d_ioctl();

    udata.u_error = 0;
    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    ifnot (isdevice(ino))
    {
	udata.u_error = ENOTTY;
	return(-1);
    }

    ifnot (getperm(ino) & OTH_WR)
    {
	udata.u_error = EPERM;
	return(-1);
    }

    dev = ino->c_node.i_addr[0];

    if (d_ioctl(dev, request,data))
	return(-1);
    return(0);
}



_mount(spec, dir, rwflag)
char *spec;
char *dir;
int rwflag;
{
    register inoptr sino, dino;
    register int dev;
    inoptr n_open();
    int    super(), getmode(), fmount(), d_open();

    udata.u_error = 0;
    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }
    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    ifnot (dino = n_open(dir,NULLINOPTR))
    {
	i_deref(sino);
	return (-1);
    }
    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }
    if (getmode(dino) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	goto nogood;
    }
    dev = (int)sino->c_node.i_addr[0];

    if ( dev >= NDEVS || d_open(dev))
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

    if (fmount(dev,dino))
    {
       udata.u_error = EBUSY;
       goto nogood;
    }
    i_deref(dino);
    i_deref(sino);
    return(0);
nogood:
    i_deref(dino);
    i_deref(sino);
    return (-1);
}



_umount(spec)
char *spec;
{
    register inoptr sino;
    register int dev;
    register inoptr ptr;
    inoptr n_open();
    int    super(), getmode(), validdev();

    udata.u_error = 0;
    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];
    ifnot (validdev(dev))
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
    i_deref(fs_tab[dev].s_mntpt);

    i_deref(sino);
    return(0);

nogood:
    i_deref(sino);
    return (-1);
}



_time(tvec)
int tvec[];
{
    udata.u_error = 0;
    rdtime(tvec);  /* In machdep.c */
    return(0);
}
