/**************************************************
UZI (Unix Z80 Implementation) Kernel:  xmachdep.c
***************************************************/
/* Revisions:
 *  1.4.98 - Adapted from UZI180 Work.			HFB
 * 20.9.99 - Linux version for working with uzi images -- HP
 */
#include "unix.h"
#include <config.h>		/* added from unix.h 9.12.97 */
#include "extern.h"


/* This is called at the very beginning to initialize everything.
 * It is the equivalent of main()
 */

fs_init()
{
    inint = 0;
    udata.u_euid = 0;
    udata.u_insys = 1;
}


/* This checks to see if a user-suppled address is legitimate
 */

valadr(base,size)
char *base;
uint16 size;
{
    return(1);
}


/* This adds two tick counts together.  The t_time field holds up to
 * one second of ticks, while the t_date field counts minutes.
 */

addtick(t1,t2)
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


incrtick(t)
time_t *t;
{
    if (++t->t_time == 60*TICKSPERSEC)
    {
        t->t_time = 0;
        ++t->t_date;
    }
}


/*
stkreset()
{
#asm
	pop	hl
	ld	sp,_udata-2
	jp	(hl)
#endasm
}
*/

/*
tempstack()
{
#asm
	pop	hl
	ld	sp,100h
	jp	(hl)
#endasm
}
*/

/*-----------------------------------------------------------*/
/* Read date/time to specified location from global variable */
rdtime(tloc)
time_t *tloc;
{
    di();
    rdtod();			/*== Added 17 Mar 1998 ==*/
    tloc->t_time = tod.t_time;
    tloc->t_date = tod.t_date;
    ei();
}


/*--------------------------------------------------------*/
/* Set Clock Date/Time.  Error exit since Not implemented */

sttime()
{
    panic("Calling sttime\n");
}

/*------------------------------------------------*/
/* Read ZSDOS Time/Date and convert to UZI format.
 * Update global time of day.
 */

#ifndef ZSDOS
#define _TIME_T
/*#include <time.h>*/
struct tm {
        int tm_sec;
        int tm_min;
        int tm_hour;
        int tm_mday;
        int tm_mon;
        int tm_year;
        int tm_wday;
        int tm_yday;
        int tm_isdst;
        /* Those are for future use. */
        long int __tm_gmtoff__;
        __const char *__tm_zone__;
};
long time(long *);
struct tm *localtime(long *);
#endif

rdtod()
{
    char td[6];
    int  tread();

    bzero (&td, 6);      /* Clear to Null Date/Time in case of errors */
#ifdef ZSDOS
    bdos(98, &td);
    tod.t_time = (tread(td[5])>>1) | (tread(td[4])<<5) | (tread(td[3])<<11);
    tod.t_date = tread(td[2]) | (tread(td[1])<<5) | (tread(td[0])<<9);
#else
    {
    long t;
    struct tm *tms;
    int y;
    
    t = time(NULL);
    tms = localtime(&t);
    y = tms->tm_year;
    if (y >= 100) y -= 100;
    tod.t_time = (tms->tm_sec>>1) | (tms->tm_min<<5) | (tms->tm_hour<<11);
    tod.t_date = tms->tm_mday | ((tms->tm_mon+1)<<5) | (y<<9);
    }
#endif
}


/* Convert BCD byte to Binary value. */
tread(bb)
char bb;
{
    return (10 * (((int)bb >> 4) & 0x0f) + ((int)bb & 0x0f));
}


/* Disable interrupts */
di()
{
/*
#asm
	DI		; Disable Interrupts
#endasm
*/
}


/* Enable interrupts if we are not in service routine */
ei()
{
    if (inint)
        return;
    ;             /* Empty statement necessary to fool compiler */
/*
#asm
	EI		; Enable Interrupts
#endasm
*/
}


/* Print Character to List Device (Printer) */

lpout(c)
int c;
{
/*    bdos(5, c); */
}


/*---------------------------------------------*/
/* This shifts an unsigned int right 8 places. */

shift8(uint16 x)
{
/*
#asm
	pop	de
	pop	hl
	ld	l,h
	ld	h,0
	ld	a,l
	and	a
	push	hl
	push	de
#endasm
*/
return x >> 8;
}


/* This prints an error message and dies. */

panic(s)
char *s;
{
    di();
    inint = 1;
    kprintf("PANIC: %s\n",s);
    idump();
    abort();
}


warning(s)
char *s;
{
    kprintf("WARNING: %s\n",s);
}


kputs(s)
char *s;
{
    while (*s)
        kputchar(*s++);
}


kputchar(c)
char c;
{
    if (c == '\n')
        _putc('\r');
    _putc(c);
}


idump()
{
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];
    bufptr j;

    kprintf("Err %d root %d\n", udata.u_error, root - i_tab);

    kprintf("\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY\n");

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
        kprintf("%d\t%d\t%d\t%u\t0%o",
               ip-i_tab, ip->c_magic,ip->c_dev, ip->c_num,
               ip->c_node.i_mode);
        kprintf("\t%d\t%d\t%d\t%d\n",   /* 10.12.97 split for compiler */
               ip->c_node.i_nlink,ip->c_node.i_addr[0],
               ip->c_refs,ip->c_dirty);
        ifnot (ip->c_magic)     
            break;
    }

    kprintf("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\n");
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
        kprintf("%d\t%d\t0x%x\t%d",
               pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid);
        kprintf("\t%d\t%d\t0x%x\t0x%x\n",        /* 10.12.97 split for compiler 
*/
               pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
               pp->p_ignored);
        ifnot(pp->p_pptr)
            break;
    }   
/*==
 *==    kprintf("\nndev blk drty bsy\n");
 *==    for (j=bufpool; j < bufpool+NBUFS; ++j)
 *==        kprintf("%d %u %d %d\n",j->bf_dev,j->bf_blk,j->bf_dirty,j->bf_busy);
 ==*/
    bufdump();          /*== in DEVIO ==*/

    kprintf("\ninsys %d ptab %d call %d cwd %d sp 0x%x\n",
        udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
       udata.u_sp);
}


/* Short version of printf to save space.
 * NOTE: This code may need to be rewritten for different compilers
 *       since it makes assumptions about the parameter ordering and
 *       sizing on the system stack.
 */

_kprintf(nargs)
char *nargs;
{
    register char *fmt;
    register char c;
    int  base;
    char s[7], *itob();

    fmt = &nargs+1;             /* NOTE: Assumes Pointer to 2-byte Word */
    while (c = *nargs++) {
        if (c != '%') {
            kputchar(c);
            continue;
        }
        switch (c = *nargs++) {
            case 'c': kputchar(*nargs++);
                      continue;

            case 'd': base = -10;
                      goto prt;

            case 'o': base = 8;
                      goto prt;

            case 'u': base = 10;
                      goto prt;

            case 'x': base = 16;
   prt:
                      kputs(itob(*(int *)fmt, s, base));
                      fmt++;  fmt++;	/* NOTE: for Size of Word Ptr */
                      continue;

            case 's': kputs(*(char **)fmt);
                      fmt++;  fmt++;    /* NOTE: for Size of Word Ptr */
                      continue;

            default: kputchar(c);
                     continue;
        }
    }
}


#define TRUE  1
#define FALSE 0

/* convert an integer to a string in any base (2-36) */
char *itob (int n, char *s, int base)
{
    unsigned int u;
    char *p, *q;
    int negative, c;

    if ((n < 0) && (base == -10)) {
        negative = TRUE;
        u = -n;
    }
    else {
        negative = FALSE;
        u = n;
    }

    if (base == -10)            /* Signals signed conversion */
         base = 10;
    p = q = s;
    do {                        /* Generate digits in reverse order */
        if ((*p = u % base + '0') > '9')
            *p += ('A' - ('9' + 1));
        ++p;
        u = u / base;
    } while (u > 0);
    if (negative)
        *p++ = '-';
    *p = '\0';                  /* Terminate the string */
    while (q < --p) {           /* Reverse the digits */
        c = *q;
        *q++ = *p;
        *p = c;
    }
    return s;
}
