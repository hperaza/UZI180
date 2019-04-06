/**************************************************
UZI (Unix Z80 Implementation) Kernel:  extern.h
***************************************************/
/* These are the global data structures */

#ifdef MAIN
#define extern
#endif

#undef UTIL		/* Change this to define UTIL if Not running kernel */
/* This is accessed by the macro udata which is really ub.u_d
 * for the running system, _ub is defined in MACHASM.ASZ, but
 * it needs to be here for the utilities that run under CP/M.
 */
#ifndef UTIL
#ifndef MAIN
extern struct u_block ub;
#endif
#endif

#define udata ub.u_d

extern struct p_tab ptab[PTABSIZE];
extern unsigned maxproc;   /* Actual max number of processes */
extern unsigned ramsize;
extern unsigned procmem;

extern inoptr root;     /* Address of root dir in inode table */
extern int16 ROOTDEV;   /* Device number of root filesystem. */

extern struct cinode i_tab[ITABSIZE];    /* In-core inode table */
extern struct oft of_tab[OFTSIZE];       /* Open File Table */

extern struct filesys fs_tab[NDEVS];     /* Table entry for each
                                            device with a filesystem. */
extern struct blkbuf bufpool[NBUFS];

extern ptptr initproc;  /* The process table address of the first process. */
extern int16 inint;     /* flag is set whenever interrupts are being serviced */

extern int16 sec;       /* Tick counter for counting off one second */
extern int16 runticks;  /* Number of ticks current process has been
                           swapped in */

extern time_t tod;      /* Time of day */
extern time_t ticks;    /* Cumulative tick counter, in minutes and ticks  */

extern char *swapbase;  /* Used by device driver for swapping */
extern unsigned swapcnt;
extern blkno_t swapblk;

extern uint16 waitno;   /* Serial number of processes entering wait state */

#ifdef MAIN
#undef extern
#endif
