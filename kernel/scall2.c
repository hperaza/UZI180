/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  scall2.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  25.2.96  Bug fix kill(), permission test is now done.     SN
 *  18.2.98  Adapted addresses and schemes for UZI180,
 *		Added documentation.			     HFB
 *  27.9.99  kill() should return ESRCH if process was not
 *              found.                                        HP 
 */
/*LINTLIBRARY*/

#include <unix.h>
#include <config.h>
#include <extern.h>

#undef DEBUG              /* Define this if you want Debugging messages */

extern int  uzicom;              /* Flag for UZI (1) or CP/M (0) File */
                                 /* Set in execve fcn, used by doexec */

/*******************************************
Getpid ()                        Function 18
============================================
Return Process ID Number (PID) to Caller.
********************************************/

_getpid()
{
    return (udata.u_ptab->p_pid);
}


/*******************************************
Getppid ()                       Function 19
============================================
Return Parent's Process ID Number (PPID).
********************************************/

_getppid()
{
    return (udata.u_ptab->p_pptr->p_pid);
}


/*******************************************
Getuid ()                        Function 20
============================================
Return User ID Number (UID) to Caller.
********************************************/

_getuid()
{
    return (udata.u_ptab->p_uid);
}


/*******************************************
Geteuid ()                       Function 44
============================================
Return Effective User ID Number (EUID).
********************************************/

_geteuid()
{
    return (udata.u_euid);
}


/*******************************************
Getgid ()                        Function 41
============================================
Return Group ID Number (GID) to Caller.
********************************************/

_getgid()
{
    return (udata.u_gid);
}


/*******************************************
Getegid ()                       Function 45
============================================
Return Effective Group ID Number (EGID).
********************************************/

_getegid()
{
    return (udata.u_egid);
}



/*******************************************
setuid (uid)                     Function 25		?
int uid;
============================================
Set User ID Number (UID) of Process.  Must
be SuperUser or owner, else Error (EPERM).
********************************************/
#define uid (int)udata.u_argn

_setuid()
{
    int  super();

    if (super() || udata.u_ptab->p_uid == uid)
    {
        udata.u_ptab->p_uid = uid;
        udata.u_euid = uid;
        return (0);
    }
    udata.u_error = EPERM;
    return (-1);
}

#undef uid



/*******************************************
setgid (gid)                     Function 26		?
int gid;
============================================
Set Group ID Number (GID).  Must be Super-
User or in Group to Set, else Error (EPERM).
********************************************/
#define gid (int16)udata.u_argn

_setgid()
{
    int  super();

    if (super() || udata.u_gid == gid)
    {
        udata.u_gid = gid;
        udata.u_egid = gid;
        return (0);
    }
    udata.u_error = EPERM;
    return (-1);
}

#undef gid;



/*******************************************
time (tvec)                      Function 27
int tvec[];
============================================
Read Clock Time/Date to User Buffer.
********************************************/
#define tvec (int *)udata.u_argn

_time()
{
    uputw (tod.t_time, tvec);
    uputw (tod.t_date, tvec+1);
    return (0);
}

#undef tvec



/*******************************************
stime (tvec)                     Function 28
int tvec[];
============================================
Set Clock Time (Currently unimplemented).
When active, must be SuperUser to Set Time.
********************************************/
#define tvec (int *)udata.u_argn

_stime()
{
/*
    ifnot (super())
    {
        udata.u_error = EPERM;
        return (-1);
    }
    sttime (tvec);				180 - move from user area
    return (0);
*/

    udata.u_error = EPERM;
    return (-1);
}

#undef tvec



/*******************************************
times (buf)                      Function 42		?
char *buf;
********************************************/
#define buf (char *)udata.u_argn

_times()
{
    int  valadr();

    ifnot (valadr (buf, 6*sizeof(time_t)))
        return (-1);

    di();
     /* If Destination is in Common Area (F000-FFFF is always in Context),
      * simply copy the data, otherwise perform Interbank "uput" to User Bank.
      */
    if (buf >= (char *) 0xf000) {
        bcopy (&udata.u_utime, buf, 4*sizeof (time_t));
        bcopy (&ticks, buf + 4*sizeof (time_t), sizeof (time_t));
    }
    else {
        uput (&udata.u_utime, buf, 4*sizeof (time_t));
        uput (&ticks, buf + 4*sizeof (time_t), sizeof (time_t));
    }
    ei();
    return (0);
}

#undef buf



/* User's execve() call. All other flavors are library routines. */
/*******************************************
execve (name, argv, envp)        Function 23
char *name;
char *argv[];
char *envp[];
********************************************/
#define name (char *)udata.u_argn
#define argv (char **)udata.u_argn1
#define envp (char **)udata.u_argn2

_execve()
{
    register inoptr ino;
    char     *buf;
    inoptr   n_open();
    char     *bread();
    blkno_t  bmap();
    blkno_t  blk;
    char     **nargv;             /* In user space */
    char     **nenvp;             /* In user space */
    struct s_argblk *abuf, *ebuf;
    int      (**sigp)();
    int      argc;
    char     **rargs();
    unsigned *pbuf;
    char     *progptr;            /* Original UZI */
    int      getperm(), wargs();
    unsigned tmpbuf();

    if (! (ino = n_open (name, NULLINOPTR)))
        return (-1);

    if (!( (getperm (ino) & OTH_EX) &&
           (ino->c_node.i_mode & F_REG) &&
           (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)) ) )
    {
        udata.u_error = EACCES;
        goto nogood;
    }

    setftime (ino, A_TIME);

                   /* Read in the first block of the new program */
    buf = bread (ino->c_dev, bmap (ino, 0, 1), 0);
    pbuf = (unsigned *)buf;

    /****************************************
     * Get magic number into var magic
     * C3    : executable file no C/D sep.
     * 00FF  :     "        "  with C/D sep. (not supported in UZI180)
     * other : maybe shell script (nogood2)
     ****************************************/
    if ((*buf & 0xff) != EMAGIC)
    {
         udata.u_error = ENOEXEC;
         goto nogood2;
    }
           /* Set Flag to indicate type of Executable (UZI-CP/M) */
    if ((*(buf+3)=='U') && (*(buf+4)=='Z') && (*(buf+5)=='I'))
        uzicom = 1;
    else
        uzicom = 0;
           /* Gather the arguments, and put them in temporary buffers. */
                                 /* Put environment in another buffer. */
    abuf = (struct s_argblk *)tmpbuf();
    ebuf = (struct s_argblk *)tmpbuf();

    if (wargs (argv, abuf) || wargs (envp, ebuf))
        goto nogood3;     /* SN */

    di();
           /* Check setuid stuff here. No other changes needed in user data */
    if (ino->c_node.i_mode & SET_UID)
        udata.u_euid = ino->c_node.i_uid;

    if (ino->c_node.i_mode & SET_GID)
        udata.u_egid = ino->c_node.i_gid;

    udata.u_ptab->p_fork_inf = 0;       /* Reset child of fork */

            /* We are definitely going to succeed with the exec,
             * so we can start writing over the old program
             */
    uput (buf, PROGBASE, 512);          /* Move 1st Block to user bank */
    brelse (buf);

            /* At this point, we are committed to reading in and
             * executing the program. We switch to a local stack,
             * and pass to it the necessary parameter: ino
             */
    udata.u_ino = ino;     /* Temporarily stash these here */

    tempstack();

            /* Read in the rest of the program */
    progptr = PROGBASE+512;
    for (blk = 1; blk <= ino->c_node.i_size.o_blkno; ++blk)
    {
        buf = bread (ino->c_dev, bmap (ino, blk, 1), 0);
        uput (buf, progptr, 512);
        brelse ((bufptr)buf);
        progptr += 512;
    }
    i_deref (ino);
    udata.u_break = (int)progptr;              /* Set Break for program */

            /* Turn off caught signals */
    for (sigp = udata.u_sigvec; sigp < (udata.u_sigvec+NSIGS); ++sigp)
        if (*sigp != SIG_IGN)
            *sigp = SIG_DFL;

            /* Read back the arguments and the environment */
    nargv = rargs (PROGTOP-2, abuf, &argc);
    nenvp = rargs ((char *)(nargv), ebuf, NULL);

            /* Fill in udata.u_name with Program invocation name */
    uget (ugetw (nargv), udata.u_name, 8);
    bcopy(udata.u_name, udata.u_ptab->p_name, 8);  /**HP**/

    brelse ((char *)abuf);
    brelse ((char *)ebuf);

            /* Shove argc and the address of argv just below envp */
    uputw (nargv, nenvp - 1);
    uputw (argc, nenvp - 2);

            /* Jump into the program, first setting the stack
             * and copying the udata block
             */
    ei();
    doexec ((int16 *)(udata.u_isp = nenvp - 2));

nogood3:
    brelse ((char *)abuf);
    brelse ((char *)ebuf);
nogood2:
    brelse (buf);
nogood:
    i_deref (ino);
    return (-1);
}

#undef name
#undef argv
#undef envp
/* SN    TODO      max (1024) 512 bytes for argv
               and max  512 bytes for environ
*/


wargs (argv, argbuf)       /* argv in user space */
char **argv;
struct s_argblk *argbuf;
{
    register char *ptr;    /* Address of base of arg strings in user space */
    int      c;
    register char *bufp;
    unsigned ugetw();
    char     ugetc();

    argbuf->a_argc = 0;              /* Store argc in argbuf */
    bufp = argbuf->a_buf;

    while (ptr = (char *)ugetw (argv++))
    {
        ++(argbuf->a_argc);          /* Store argc in argbuf. */
        do
        {
            *bufp++ = c = ugetc (ptr++);
            if (bufp > argbuf->a_buf+500)
            {
                udata.u_error = E2BIG;
                return (1);
            }
        }
        while (c);
    }
    argbuf->a_arglen = bufp - argbuf->a_buf;    /*Store total string size. */
    return (0);
}



char *
rargs (ptr, argbuf, cnt)
char *ptr;
struct s_argblk *argbuf;
int  *cnt;
{
    char **argv;         /* Address of users argv[], just below ptr */
    int  argc, arglen;
    char **argbase;
    char *sptr;

    sptr = argbuf->a_buf;

                /* Move them into the users address space, at the very top */
    ptr -= (arglen = argbuf->a_arglen);

    if (arglen)
        uput (sptr, ptr, arglen);

                /* Set argv to point below the argument strings */
    argc = argbuf->a_argc;
    argbase = argv = (char **)ptr - (argc + 1);

    if (cnt)
        *cnt = argc;

              /* Set each element of argv[] to point to its argument string */
    while (argc--)
    {
        uputw (ptr, argv++);
        if (argc)
        {
            do
                ++ptr;
            while (*sptr++);
        }
    }
    uputw ((char *)0, argv);      /*;;26Feb- Add Null Pointer to end of array*/
    return ((char *)argbase);
}



/*******************************************
brk (addr)                       Function 30
char *addr;
********************************************/
#define addr (char *)udata.u_argn

_brk()
{
            /* Don't allow break to be set past user's stack pointer */
            /*** St. Nitschke allow min. of 512 bytes for Stack ***/
    if (addr >= (char *)(udata.u_sp)-512)
    {
        udata.u_error = ENOMEM;
        return (-1);
    }
    udata.u_break = (unsigned)addr;
    return (0);
}

#undef addr



/*******************************************
sbrk (incr)                      Function 31
uint16 incr;
********************************************/
#define incr (uint16)udata.u_argn

_sbrk()
{
    register unsigned oldbrk;

    udata.u_argn += (oldbrk = udata.u_break);
    if ((unsigned)udata.u_argn < oldbrk)
        return (-1);
    if (_brk())             /* brk (udata.u_argn) */
        return (-1);

    return ((unsigned)oldbrk);
}

#undef incr



/*******************************************
wait (statloc)                   Function 24		?
int *statloc;
********************************************/
#define statloc (int *)udata.u_argn

_wait()
{
    register ptptr p;
    register int retval;
    int      valadr();

    if (statloc && !valadr (statloc, sizeof(int)))
    {
        udata.u_error = EFAULT;
        return (-1);
    }

    di();
        /* See if we have any children. */
    for (p=ptab; p < ptab+maxproc; ++p)
    {
        if (p->p_status && p->p_pptr == udata.u_ptab && p != udata.u_ptab)
            goto ok;
    }
    udata.u_error = ECHILD;
    ei();
    return (-1);

ok:
        /* Search for an exited child; */
    ei();
    for (;;)
    {
        chksigs();
        if (udata.u_cursig)
        {
            udata.u_error = EINTR;
            return (-1);
        }
        di();
        for (p=ptab; p < ptab+maxproc; ++p)
        {
            if (p->p_status == P_ZOMBIE && p->p_pptr == udata.u_ptab)
            {
                if (statloc)
                    *statloc = p->p_exitval;
                p->p_status = P_EMPTY;
                retval = p->p_pid;

                        /* Add in child's time info.  It was stored on top */
                        /* of p_wait in the childs process table entry. */
                addtick (&udata.u_cutime, p->p_wait);
                addtick (&udata.u_cstime, p->p_wait +
                                          sizeof (time_t));
                ei();
                return (retval);
            }
        }
            /* Nothing yet, so wait */
        psleep (udata.u_ptab);
    }
}

#undef statloc



/*******************************************
_exit (val)                       Function 0
int16 val;
********************************************/
#define val (int16)udata.u_argn

__exit()
{
    doexit (val, 0);
}

#undef val



/* Subject of change in version 1.3h */
doexit (val, val2)
int16 val;
int16 val2;
{
    register int16 j;
    register ptptr p;

#ifdef DEBUG
    kprintf("process %d exiting\n", udata.u_ptab->p_pid);
#endif

    di();

    for (j=0; j < UFTSIZE; ++j)
    {
        if (udata.u_files[j] >= 0)      /* Portable equivalent of == -1 */
            doclose(j);
    }

    _sync();                         /* Not necessary, but a good idea. */

    udata.u_ptab->p_exitval = (val<<8) | (val2 & 0xff);

    i_deref(udata.u_cwd);

    /* Stash away child's execution tick counts in process table,
     * overlaying some no longer necessary stuff.
     */
    addtick(&udata.u_utime, &udata.u_cutime);
    addtick(&udata.u_stime, &udata.u_cstime);
    bcopy(&udata.u_utime, &(udata.u_ptab->p_wait), 2 * sizeof(time_t));

    /* See if we have any children. Set child's parents to our parent */
    for (p=ptab; p < ptab+maxproc; ++p)
    {
        if (p->p_status  &&  p->p_pptr == udata.u_ptab  &&  p != udata.u_ptab)
            p->p_pptr = udata.u_ptab->p_pptr;
    }
    ei();

    /* Wake up a waiting parent, if any. */
    wakeup((char *)udata.u_ptab->p_pptr);

    udata.u_ptab->p_status = P_ZOMBIE;

    swapin(getproc());
    panic("doexit: won't exit");
}



/*******************************************
fork ()                          Function 32
********************************************/

_fork()
{
    int dofork();

    return (dofork());
}



/*******************************************
pause ()                         Function 37
********************************************/

_pause()
{
    psleep (0);
    udata.u_error = EINTR;
    return (-1);
}



/*******************************************
signal (sig, func)               Function 35		?
int16 sig;
int16 (*func)();
********************************************/
#define sig (int16)udata.u_argn
#define func (int (*)())udata.u_argn1

_signal()
{
    int retval;
    int valadr();

    di();
/*    if (sig < 1 || sig == SIGKILL || sig >= NSIGS) */
    if (sig < 1 || sig >= NSIGS)
    {
        udata.u_error = EINVAL;
        goto nogood;
    }

    if (func == SIG_IGN)
        udata.u_ptab->p_ignored |= sigmask (sig);
    else
    {
        if (func != SIG_DFL && !valadr ((char *)func, 1))
        {
            udata.u_error = EFAULT;
            goto nogood;
        }
        udata.u_ptab->p_ignored &= ~sigmask (sig);
    }
    retval = (int)udata.u_sigvec[sig];
    udata.u_sigvec[sig] = func;
    ei();
    return (retval);

nogood:
    ei();
    return (-1);
}

#undef sig
#undef func



/*******************************************
kill (pid, sig)                  Function 39
int16 pid;
int16 sig;
********************************************/
#define pid (int16)udata.u_argn
#define sig (int16)udata.u_argn1

_kill()
{
    ptptr p;
    int   super();

    if (sig <= 0 || sig > 15)
    {
        udata.u_error = EINVAL;
        return (-1);
    }

    for (p=ptab; p < ptab+maxproc; ++p)
    {
            /*
             *     SN wcy permission check
             */
        if (p->p_pid == pid)
        {
            if (udata.u_ptab->p_uid == p->p_uid || super() )
            {
                ssig (p, sig);
                return (0);
            }
            else
            {
                udata.u_error = EPERM;
                return (-1);
            }
        }
    }
    udata.u_error = ESRCH;
    return (-1);
}

#undef pid
#undef sig



/*******************************************
alarm (secs)                     Function 38		?
uint16 secs;
********************************************/
#define secs (int16)udata.u_argn

_alarm()
{
    int retval;

    di();
    retval = udata.u_ptab->p_alarm;
    udata.u_ptab->p_alarm = secs;
    ei();
    return (retval);
}

#undef secs
