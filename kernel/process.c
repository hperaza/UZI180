/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  process.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  09.10.99  Corrected a minor bug in clkint2()            HP
 *  23.01.04  Call fdInit() in order to initialize the FDC  HP
 *  31.01.04  Call wd_init() in order to read the partition
 *            table                                         HP
 */

#undef DEBUG                   /* Define to activate Debug Code */

#include <unix.h>
#include <config.h>
#include <extern.h>

#include <version.h>

extern char osBank;				/* defined in machasm */
extern int  fdInit(int);			/* defined in flopasm */
extern int  wd_init();                          /* defined in devhd.c */

init2()
{
    register char *j;
    static char bootline[2];
    static char arg[] = { '/','i','n','i','t',0,0,0x01,0x01,0,0,0,0 };
    inoptr i_open();
    ptptr ptab_alloc();
    int   d_open(), fmount();
    char  *cmdline = (char *) 0x81;		/* CP/M command line */

    bufinit();

       /* Create the context for the first process
        */
    newproc (udata.u_ptab = initproc = ptab_alloc());
    initproc->p_status = P_RUNNING;
    initproc->p_fork_inf = udata.u_page;       /* set for use in execve */

       /* User's file table
        */
    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
        *j = -1;

    ei();

       /* Set Time-Of-Day absolutely, instead of waiting for interrupt */
    rdtod();

       /* Open the console tty device thereby establishing the init
        * process' controlling tty
        */
    if (d_open(TTYDEV) != 0)
        panic("no tty");

    kprintf("UZI180 version "VERSION" built "BUILDDATE"\n");
    kprintf("Copyright (c) 1998-2002 by H.F.Bower, D.Braun, S.Nitschke, H.Peraza\n");
    
    kprintf("%dkB total RAM, %dkB available to processes (%d processes max)\n",
            ramsize, procmem, maxproc);

    fdInit(0);					/* this should reset the FDC */
    wd_init();

    if (*cmdline) {

	while (*cmdline == ' ') ++cmdline;
        ROOTDEV = *cmdline - '0';

    } else {

        kprintf("boot: ");
        udata.u_base = bootline;
        udata.u_sysio = 1;
        udata.u_count = 2;
        udata.u_euid = 0;		/* Always begin as superuser */

        cdread(TTYDEV);
        ROOTDEV = *bootline - '0';
    }

       /* Mount the root device */
    kprintf("Mounting root fs: ");

    if (fmount(ROOTDEV, NULLINODE))
        panic("no filesys");
    ifnot (root = i_open(ROOTDEV, ROOTINODE))
        panic("no root");

    kprintf("OK\n");

    i_ref(udata.u_cwd = root);
    rdtime(&udata.u_time);

       /* Poke the execve arguments into user data space the UZI280 way */
    uput(arg, PROGBASE, sizeof (arg));

    udata.u_argn  = (int16)PROGBASE;
    udata.u_argn1 = 0x107;       /* Arguments (just "/init") */
    udata.u_argn2 = 0x10b;       /* Environment (none) */

    _execve();

    panic("no /init");           /* BIG Trouble if we Get Here!! */
}


/* psleep() puts a process to sleep on the given event.  If another
 * process is runnable, it swaps out the current one and starts the
 * new one.  Normally when psleep is called, the interrupts have
 * already been disabled.   An event of 0 means a pause(), while an
 * event equal to the process's own ptab address is a wait().
 */

psleep (event)
char *event;
{
    di();
    if (udata.u_ptab->p_status != P_RUNNING)
        panic ("psleep: voodoo");
    if (!event)
        udata.u_ptab->p_status = P_PAUSE;
    else if (event == (char *)udata.u_ptab)
        udata.u_ptab->p_status = P_WAIT;
    else
        udata.u_ptab->p_status = P_SLEEP;

    udata.u_ptab->p_wait = event;
    udata.u_ptab->p_waitno = ++waitno;

    ei();
    swapout();          /* Swap us out, and start another process */
            /* Swapout doesn't return until we have been swapped back in */
}



/* wakeup() looks for any process waiting on the event,
 * and makes it runnable
 */

wakeup (event)
char *event;
{
    register ptptr p;

    di();
    for (p=ptab; p < ptab+maxproc; ++p)
    {
        if (p->p_status > P_RUNNING && p->p_wait == event)
        {
            p->p_status = P_READY;
            p->p_wait = (char *)NULL;
        }
    }
    ei();
}



/* Getproc returns the process table pointer of a runnable process.
 * It is actually the scheduler.  If there are none, it loops.
 * This is the only time-wasting loop in the system.
 */

ptptr
getproc()
{
    register status;
    static  ptptr pp = ptab;    /* Pointer for round-robin scheduling */

    for (;;)
    {
        if (++pp >= ptab + maxproc)
            pp = ptab;
        di();
        status = pp->p_status;
#if 1
#asm
	ei		; Interrupts ON absolutely!  (instead of "ei()")
#endasm
#else
	udata.u_bc = 0;
	inint = 0;
	ei(); /*_cin();*/
#endif
        if (status == P_RUNNING)
            panic ("getproc: extra running");
        if (status == P_READY)
            return (pp);
    }
}


/* Newproc fixes up the tables for the child of a fork
 */
newproc (p)           /* Passed New process table entry */
ptptr p;
{
    register char *j;

    /* Note that ptab_alloc clears most of the entry */
    di();
    /* p_swap is replaced by p_page for UZI180 Banked memory access.  It is
     * the BBR value for the base of the process area in memory (Byte value).
     * The same value is also set in udata.u_page for use by put/get routines.
     */
    udata.u_page = p->p_page = (p-ptab) * 0x10 + 0x10 + osBank;
    p->p_status = P_RUNNING;

    p->p_pptr = udata.u_ptab;
    p->p_ignored = udata.u_ptab->p_ignored;
    p->p_tty = udata.u_ptab->p_tty;
    ifnot (p->p_tty)                /* If no tty, try tty of parent's parent */
        p->p_tty = udata.u_ptab->p_pptr->p_tty;
    p->p_uid = udata.u_ptab->p_uid;
    udata.u_ptab = p;

    bzero (&udata.u_utime, 4 * sizeof (time_t)); /* Clear tick counters */

    rdtime (&udata.u_time);
    if (udata.u_cwd)
        i_ref (udata.u_cwd);
    udata.u_cursig = udata.u_error = 0;

    /* Set default priority */
    p->p_priority = MAXTICKS;

    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
        if (*j >= 0)
           ++of_tab[*j].o_refs;
    ei();
}



/* This allocates a new process table slot, and fills in its
 * p_pid field with a unique number.
 */
ptptr
ptab_alloc()
{
    register ptptr p;
    register ptptr pp;
    static int nextpid = 0;

    di();
    for (p=ptab; p < ptab+maxproc; ++p)
    {
        if (p->p_status == P_EMPTY)
            goto found;
    }
    ei();
    return (NULL);

found:
       /* See if next pid number is unique */
nogood:
    if (nextpid++ > 32000)
        nextpid = 1;
    for (pp=ptab; pp < ptab+maxproc; ++pp)
    {
        if (pp->p_status != P_EMPTY && pp->p_pid == nextpid)
            goto nogood;
    }

    bzero (p, sizeof (struct p_tab));
    p->p_pid = nextpid;
    p->p_status = P_FORKING;

    ei();
    return (p);
}



/* This is the clock interrupt routine.   Its job is to increment the clock
 * counters, increment the tick count of the running process, and either
 * swap it out if it has been in long enough and is in user space or mark
 * it to be swapped out if in system space.  Also it decrements the alarm
 * clock of processes.
 * UZI180: This is CALLed from the actual Interrupting service
 *      routine (See MACHASM.ASZ).                             HFB
 * This must have no automatic or register variables.
 */

clkint2()
{
    static ptptr p;

    inint = 1;                    /* We arrived from H/W Interrupt */

       /* Increment processes and global tick counters */
    if (udata.u_ptab->p_status == P_RUNNING)
        incrtick (udata.u_insys ? &udata.u_stime : &udata.u_utime);

    incrtick (&ticks);

       /* Do once-per-second things */
    if (++sec == TICKSPERSEC)
    {
           /* Update global time counters */
        sec = 0;
        rdtod();          /* Update time-of-day */

           /* Update process alarm clocks */
        for (p=ptab; p < ptab+maxproc; ++p)
        {
            if (p->p_alarm)
                ifnot (--p->p_alarm)
                    ssig (p, SIGALRM);
        }
    }
       /* Check run time of current process */
    if ((++runticks >= udata.u_ptab->p_priority)
               &&  !udata.u_insys  &&  (inint == 1))     /* Time to swap out */
    {
        di();
        udata.u_insys = 1;
        /*=== inint = 0;  do this in swapout() after the current process
                          context has been saved -HP */
        udata.u_ptab->p_status = P_READY;
        swapout();
        udata.u_insys = 0;               /* We have swapped back in */
    }
    /* inint = 0; === do this only at the end of timer_int HP */
}



extern int (*disp_tab[])();     /* in data.c */
extern int ncalls;

/* Unix System Call Trap Routine secondary processing.  The primary
 * Interrupt trap is in PROCASM.ASZ which manages state saving, then
 * vectors here for final activity.
 * No auto vars here, so flags will be preserved.
 */
unix2()
{
    udata.u_insys = 1;
    udata.u_error = 0;

       /* UZI180 saves the Stack Pointer and arguments in the
        * Assembly Language Function handler in MACHASM.ASZ
        */
    if (udata.u_callno >= ncalls)
        udata.u_error = EINVAL;
    ei();

#ifdef DEBUG
    kprintf ("\t\tcall %d (%x, %x, %x)\n", udata.u_callno,
              udata.u_argn, udata.u_argn1, udata.u_argn2);
#endif

       /* Branch to correct routine */
    ifnot (udata.u_error)           /* Could be set by ugetw() */
        udata.u_retval = (*disp_tab[udata.u_callno])();

#ifdef DEBUG
    kprintf ("\t\t\tcall %d, ret %x err %d\n",
              udata.u_callno, udata.u_retval, udata.u_error);
#endif

    chksigs();

    di();
    if (runticks >= udata.u_ptab->p_priority)       /* Time to swap out */
    {
        udata.u_ptab->p_status = P_READY;
        swapout();
    }
    ei();
    udata.u_insys = 0;
/*===    calltrap();            /* Call Trap Routine if necessary */

    if (udata.u_error)
        return (-1);

    return (udata.u_retval);
}



/* This sees if the current process has any signals set, and handles them.
 */
chksigs()
{
    register j;

    di();
    ifnot (udata.u_ptab->p_pending)
    {
        ei();
        return;
    }

    for (j=1; j < NSIGS; ++j)
    {
        ifnot (sigmask(j) & udata.u_ptab->p_pending)
            continue;
        if (udata.u_sigvec[j] == SIG_DFL)
        {
            ei();
#ifdef DEBUG
            kprintf("process terminated by signal: ");
#endif
            doexit (0, j);
        }

        if (udata.u_sigvec[j] != SIG_IGN)
        {
               /* Arrange to call the user routine at return */
            udata.u_ptab->p_pending &= ~sigmask(j);
#ifdef DEBUG
            kprintf("about to process signal %d\n", j);
#endif
            udata.u_cursig = j;
        }
    }
    ei();
}



sgrpsig (tty, sig)
int tty;
int16 sig;
{
    register ptptr p;

    for (p=ptab; p < ptab+maxproc; ++p) {
        if (p->p_status && p->p_tty == tty)
            ssig (p, sig);
    }
}



/*-- sendsig (ptptr proc, int16 sig)
 *-- {
 *--     register ptptr p;

 *--     if (proc)
 *--         ssig (proc, sig);
 *--     else
 *--         for (p=ptab; p < ptab+maxproc; ++p)
 *--             if (p->p_status)
 *--                 ssig (p, sig);
 *-- }--*/



ssig (proc, sig)
ptptr proc;
int16 sig;
{
    register stat;

    di();
    ifnot (proc->p_status)
        goto done;              /* Presumably was killed just now */

    if (proc->p_ignored & sigmask (sig))
        goto done;

    stat = proc->p_status;
    if (stat == P_PAUSE || stat == P_WAIT || stat == P_SLEEP)
        proc->p_status = P_READY;

    proc->p_wait = (char *)NULL;
    proc->p_pending |= sigmask (sig);
done:
    ei();
}
