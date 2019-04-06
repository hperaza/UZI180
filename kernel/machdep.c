/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  machdep.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  9.12.97 - modified #asm for only Z80, added include for
 *	config.h, deleted from unix.h.			     HFB
 */

#undef MEMDEBUG

#include <unix.h>
#include <config.h>
#include <extern.h>

/* This is chained to from the initial hardware setup in MACHASM.ASZ which
 * performs initial required actions such as clearing uninitialized RAM,
 * setting up the interrupt vector table, and disabling interrupts.  Unique
 * hardware setups should also have been performed prior to entering here.
 * This is the equivalent of main()
 */

fs_init()
{
    inint = 0;
    udata.u_insys = 1;
    ei();
    init2();                 /* in process.c, never returns here */
}


/* This checks to see if a user-supplied address is legitimate */

valadr (base, size)
char *base;
uint16 size;
{
    if (base < PROGBASE || base+size >= (char *)&udata)
    {
        udata.u_error = EFAULT;
        return(0);
    }
    return(1);
}


/* This adds two tick counts together.  The t_time field holds up to
 *  one second of ticks, while the t_date field counts minutes.
 */

addtick (t1, t2)
time_t *t1, *t2;
{
    t1->t_time += t2->t_time;
    t1->t_date += t2->t_date;
    if (t1->t_time >= 60*TICKSPERSEC)
    {
        t1->t_time -= 60*TICKSPERSEC;
        ++t1->t_date;
    }
}


incrtick (t)
time_t *t;
{
    if (++t->t_time == 60*TICKSPERSEC)
    {
        t->t_time = 0;
        ++t->t_date;
    }
}


#if 0  /* moved to machasm.asz, this code should be in common area,
        * and should be called after switching banks back to user space
        */

static int cursig;
static int (*curvec)();

/*----------------------------------------------------------*/
calltrap()
{
    /* Deal with a pending caught signal, if any. */
    /* udata.u_insys should be false, and interrupts enabled.
     * Remember, the user may never return from the trap routine.
     */

    if (udata.u_cursig)
    {
        cursig = udata.u_cursig;
        curvec = udata.u_sigvec[cursig];
        udata.u_cursig = 0;
        udata.u_sigvec[cursig] = SIG_DFL;   /* Reset to default */
        ei();
        (*curvec)(cursig);
        di();
    } 
}

#endif

/*-----------------------------------------------------------*/
/* Read date/time to System location from global variable    */
rdtime (tloc)
time_t *tloc;
{
    di();
    tloc->t_time = tod.t_time;
    tloc->t_date = tod.t_date;
    ei();
}


/*--------------------------------------------------------*/
/* Set Clock Date/Time.  Error exit since Not implemented */

sttime()
{
    panic ("Calling sttime\n");
}


/* This prints an error message and dies. */

panic (s)
char *s;
{
    di();
    inint = 1;
    kprintf("PANIC: %s\n",s);
    idump();
    abort();
}


warning (char *s)
{
    kprintf("WARNING: %s\n",s);
}


idump()
{
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];

    kprintf("Err %d root %d\n", udata.u_error, root - i_tab);
    kprintf ("\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY\n");

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
        kprintf ("%d\t%d\t%d\t%u\t0%o\t",
                ip-i_tab, ip->c_magic,ip->c_dev, ip->c_num,
                ip->c_node.i_mode);
        kprintf ("%d\t%d\t%d\t%d\n",             /* line split for compiler */
                ip->c_node.i_nlink,ip->c_node.i_addr[0],
                ip->c_refs,ip->c_dirty);
        ifnot (ip->c_magic)     
            break;
    }

    kprintf ("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\tCHILD\n");
    for (pp=ptab; pp < ptab+PTABSIZE /*maxproc*/; ++pp)
    {
        if (pp->p_status == P_EMPTY) continue;
        kprintf ("%d\t%d\t0x%x\t%d\t",
                pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid);
	kprintf ("%d\t%d\t0x%x\t0x%x\t%x\n",      /* line split for compiler */
                pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
                pp->p_ignored, pp->p_fork_inf);
    }   
    
    bufdump();

    kprintf ("insys %d ptab %d call %d cwd %d sp 0x%x\n",
            udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
            udata.u_sp);
}
