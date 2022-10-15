/****************************************************
UZI (Unix Z80 Implementation) Kernel:  unix.h
From UZI by Doug Braun and UZI280 by Stefan Nitschke.
*****************************************************/
/* History:
 *   21.12.97 - Removed leading ? from Per-Process equates.	HFB
 *   11.07.98 - Shortened Time Slices to 50 Ticks/Sec (20 mS).	HFB
 */

#ifndef vax
#ifndef CPM
#define CPM
#endif
#endif

#define UFTSIZE 10       /* Number of user files */		/*280 = 22*/
#define OFTSIZE 15       /* Open file table size */		/*280 = 45*/
#define ITABSIZE 20      /* Inode table size */			/*280 = 45*/
#define PTABSIZE 15      /* Process table size. This is the
                          * theoretical maximum that UZI180 can
                          * can handle on a P112 board. The real
                          * maximum number of processes, according
                          * to the amount of available RAM, is held
                          * in the numproc variable (see external.h) */
                          
#define NSIGS 16         /* Number of signals <= 16 */

#define ROOTINODE 1      /* Inode # of / for all mounted filesystems. */

#define TICKSPERSEC 50   /* Ticks per second */
#define MAXTICKS     4   /* Max ticks before swapping out (time slice)
                            default process time slice */
#define MAXBACK      3   /* Process time slice for tasks not connected
                            to the current tty */
#define MAXBACK2     2   /* Process time slice for background tasks */
#define MAXINTER     5   /* Process time slice for interactive tasks */

#define ARGBLK       0   /* Block number on SWAPDEV for arguments */
#define PROGBASE  ((char *)(0x0100))  /* also data base */
#define PROGTOP   ((char *)(0xf000))  /* Top of program, base of U_DATA */

#define EMAGIC    0xc3   /* Header of executable */
#define CMAGIC   24721   /* Random number for cinode c_magic */
#define SMOUNTED 12742   /* Magic number to specify mounted filesystem */

#ifndef NULL
#define NULL         0
#endif

#define Hi_TECH_C

#ifndef Hi_TECH_C
/* Speed and code length optimized for Q/C Compiler */
#define ifnull(e) if(e){}else
#define ifnot(e) if(e){}else
#define ifzero(e) if(e){}else
#else           /* Define for Hi-Tech Compiler */
#define FALSE 0
#define ifnull(e) if ((e)==0)
#define ifnot(e)  if ((e)==FALSE)
#define ifzero(e) if ((e)==NULL)
#endif


#ifdef CPM
    typedef unsigned uint16;
    typedef int int16;
#else
    typedef unsigned short uint16;
    typedef short int16;
#endif


typedef struct s_queue {
    char *q_base;    /* Pointer to data */
    char *q_head;    /* Pointer to addr of next char to read. */
    char *q_tail;    /* Pointer to where next char to insert goes. */
    int   q_size;    /* Max size of queue */
    int   q_count;   /* How many characters presently in queue */
    int   q_wakeup;  /* Threshold for waking up processes waiting on queue */
} queue_t;



typedef struct time_s {
    uint16 t_time;
    uint16 t_date;
} time_t;


/* User's structure for times() system call */

struct tms {
	time_t  tms_utime;
	time_t  tms_stime;
	time_t  tms_cutime;
	time_t  tms_cstime;
	time_t  tms_etime;      /* Elapsed real time */
} ;


/* Flags for setftime() */
#define A_TIME 1
#define M_TIME 2
#define C_TIME 4


typedef struct off_t {
    uint16 o_blkno;      /* Block number */
    int16  o_offset;     /* Offset within block 0-511 */
} off_t;


typedef uint16 blkno_t;    /* Can have 65536 512-byte blocks in filesystem */
#define NULLBLK ((blkno_t)-1)


typedef struct blkbuf {
    char        bf_data[512];    /* This MUST be first ! */
    char        bf_dev;
    blkno_t     bf_blk;
    char        bf_dirty;
    char        bf_busy;
    uint16      bf_time;         /* LRU time stamp */
} blkbuf, *bufptr;


typedef struct dinode {
    uint16 i_mode;
    uint16 i_nlink;
    uint16 i_uid;
    uint16 i_gid;
    off_t    i_size;
    time_t   i_atime;
    time_t   i_mtime;
    time_t   i_ctime;
    blkno_t  i_addr[20];
} dinode;               /* Exactly 64 bytes long! */


struct  stat    /* Really only used by users */
{
	int16   st_dev;
	uint16  st_ino;
	uint16  st_mode;
	uint16  st_nlink;
	uint16  st_uid;
	uint16  st_gid;
	uint16  st_rdev;
	off_t   st_size;
	time_t  st_atime;
	time_t  st_mtime;
	time_t  st_ctime;
};

/* Bit masks for i_mode and st_mode */

#define OTH_EX  0001
#define OTH_WR  0002
#define OTH_RD  0004
#define GRP_EX  0010
#define GRP_WR  0020
#define GRP_RD  0040
#define OWN_EX  0100
#define OWN_WR  0200
#define OWN_RD  0400

#define SAV_TXT 01000
#define SET_GID 02000
#define SET_UID 04000

#define MODE_MASK 07777

#define F_REG   0100000
#define F_DIR   040000
#define F_PIPE  010000
#define F_BDEV  060000
#define F_CDEV  020000

#define F_MASK  0170000


typedef struct cinode {
    int        c_magic;           /* Used to check for corruption. */
    int        c_dev;             /* Inode's device */
    unsigned   c_num;             /* Inode # */
    dinode     c_node;
    char       c_refs;            /* In-core reference count */
    char       c_dirty;           /* Modified flag. */
} cinode, *inoptr;

#define NULLINODE ((inoptr)NULL)
#define NULLINOPTR ((inoptr*)NULL)


typedef struct direct {
    uint16   d_ino;
    char     d_name[14];
} direct;


typedef struct filesys {
    int16       s_mounted;
    uint16      s_isize;
    uint16      s_fsize;
    int16       s_nfree;
    blkno_t     s_free[50];
    int16       s_ninode;
    uint16      s_inode[50];
    int16       s_fmod;
    time_t      s_time;
    blkno_t     s_tfree;
    uint16      s_tinode;
    inoptr      s_mntpt;     /* Mount point */
} filesys, *fsptr;

typedef struct oft {
    off_t     o_ptr;      /* File position pointer */
    inoptr    o_inode;    /* Pointer into in-core inode table */
    char      o_access;   /* O_RDONLY, O_WRONLY, or O_RDWR */
    char      o_refs;     /* Reference count: depends on # of active children */
} oft;


/* Process table p_status values */

#define P_EMPTY         0    /* Unused slot */
#define P_RUNNING       1    /* Currently running process */
#define P_READY         2    /* Runnable   */
#define P_SLEEP         3    /* Sleeping; can be awakened by signal */
#define P_XSLEEP        4    /* Sleeping, don't wake up for signal */
#define P_PAUSE         5    /* Sleeping for pause(); can wakeup for signal */
#define P_FORKING       6    /* In process of forking; do not mess with */
#define P_WAIT          7    /* Executed a wait() */
#define P_ZOMBIE2       8    /* Exited but code pages still valid. */
#define P_ZOMBIE        9    /* Exited. */


#define SIGHUP  1       /* hangup */
#define SIGINT  2       /* interrupt */
#define SIGQUIT 3       /* quit */
#define SIGILL  4       /* illegal instruction (not reset when caught */
#define SIGTRAP 5       /* trace trap (not reset when caught) */
#define SIGIOT  6       /* IOT instruction */
#define SIGEMT  7       /* EMT instruction */
#define SIGFPE  8       /* floating point exception */
#define SIGKILL 9       /* kill */
#define SIGBUS  10      /* bus error */
#define SIGSEGV 11      /* segmentation violation */
#define SIGSYS  12      /* bad argument to system call */
#define SIGPIPE 13      /* write on a pipe with no one to read it */
#define SIGALRM 14      /* alarm clock */
#define SIGTERM 15      /* software termination signal from kill */

#define  SIG_DFL   (int (*)())0
#define  SIG_IGN   (int (*)())1

#define sigmask(sig)    (1<<(sig))

/* Process table entry */

typedef struct p_tab {
    char        p_status;       /* Process status */
    char        p_tty;          /* Process' controlling tty minor # */
    int         p_pid;          /* Process ID */
    int         p_uid;
    struct  p_tab *p_pptr;      /* Process parent's table entry */
    unsigned    p_alarm;        /* Seconds until alarm goes off */
    unsigned    p_exitval;      /* Exit value */
    char       *p_wait;         /* Address of thing waited for */
    /* Everything below here is overlaid by time info at exit */
    int         p_page;         /* Process BBR base value in UZI180 */
    int         p_priority;     /* Process priority */
    uint16      p_pending;      /* Pending signals */
    uint16      p_ignored;      /* Ignored signals */
    struct   u_block *p_ublk;   /* Pointer to udata block when not running */
    uint16      p_waitno;       /* wait #; for finding longest waiting proc */
  /* Added for correct execve() of type 1 binaries */
    unsigned    p_fork_inf;     /* 0 = not child of fork
                                   1 = child of fork  */

/**HP**/
    char    p_name[8];
    time_t  p_time, p_utime, p_stime, p_cutime, p_cstime;
/**HP**/
} p_tab, *ptptr;

/* Per-process data (Swapped with process) */
/*------ <use ASMDEF.I in Assembly Modules when needed>
#asm
OSYS	equ	2	; byte offsets of elements of u_data
OCALL	equ	3
ORET	equ	4	; Return Location
ORVAL	equ	6	; Return Value
OERR	equ	8	; Error Number
OSP	equ	10	; User's Stack Pointer
OBC	equ	12	; User's Frame Pointer
OCURSIG	equ	14	; Signal currently being caught
OSIGVEC	equ	65	; Array of signal vectors
OPAGE	equ	121	; User's base BBR register value for Process
#endasm
------*/

typedef struct u_data {
    struct p_tab *u_ptab;       /* Process table pointer */
    char        u_insys;        /* True if in kernel */
    char        u_callno;       /* sys call being executed. */
    char        *u_retloc;      /* Return location from sys call */
    int         u_retval;       /* Return value from sys call */
    int         u_error;        /* Last error number */
    int         *u_sp;          /* Used when process is swapped. */
    int         *u_bc;          /* Place for user's frame pointer */
    int         u_cursig;       /* Signal currently being caught */
    int         u_argn;         /* Last system call arg */
    int         u_argn1;        /* This way because args on stack backwards */
    int         u_argn2;
    int         u_argn3;        /* args n-3, n-2, n-1, and n */

    char *      u_base;         /* Source or dest for I/O */
    unsigned    u_count;        /* Amount for I/O */
    off_t       u_offset;       /* Place in file for I/O */
    struct blkbuf *u_buf;
    char        u_sysio;        /* True if I/O to system space */

    int         u_gid;
    int         u_euid;
    int         u_egid;
    int         u_mask;         /* umask: file creation mode mask */
    time_t      u_time;         /* Start time */
    char        u_files[UFTSIZE];       /* Process file table:
	                           indices into open file table. */
    inoptr      u_cwd;          /* Index into inode table of cwd. */
    unsigned    u_break;        /* Top of data space */
    inoptr      u_ino;          /* Used during execve() */
    char        *u_isp;         /* Value of initial sp (argv) */
    int         (*u_sigvec[NSIGS])();   /* Array of signal vectors */
    char        u_name[8];      /* Name invoked with */
    time_t      u_utime;        /* Elapsed ticks in user mode */
    time_t      u_stime;        /* Ticks in system mode */
    time_t      u_cutime;       /* Total childrens ticks */
    time_t      u_cstime;
    char        u_page;         /* Process' MMU Base Address */
} u_data;


/* This is the user data structure, padded out to 512 bytes with the
 * System Stack.
 */
typedef struct u_block {
        u_data u_d;
        char   u_s [512 - sizeof(struct u_data)];
} u_block;


/* Struct to temporarily hold arguments in execve */
struct s_argblk {
    int a_argc;
    int a_arglen;
    int a_envc;
    char a_buf[512-3*sizeof(int)];
};


/* The device driver switch table */

typedef struct devsw {
    int minor;          /* The minor device number (an argument to below) */
    int (*dev_open)();  /* The routines for reading, etc */
    int (*dev_close)(); /* Format: op(minor,blkno,offset,count,buf); */
    int (*dev_read)();  /* Offset would be ignored for block devices */
    int (*dev_write)(); /* Blkno and offset ignored for tty, etc. */
    int (*dev_ioctl)(); /* Count is rounded to 512 for block devices */
} devsw;

/* Open() parameters. */

#define O_RDONLY        0
#define O_WRONLY        1
#define O_RDWR          2

/*
 * Error codes
 */
#define EPERM           1               /* Not owner */
#define ENOENT          2               /* No such file or directory */
#define ESRCH           3               /* No such process */
#define EINTR           4               /* Interrupted System Call */
#define EIO             5               /* I/O Error */
#define ENXIO           6               /* No such device or address */
#define E2BIG           7               /* Arg list too long */
#define ENOEXEC         8               /* Exec format error */
#define EBADF           9               /* Bad file number */
#define ECHILD          10              /* No children */
#define EAGAIN          11              /* No more processes */
#define ENOMEM          12              /* Not enough core */
#define EACCES          13              /* Permission denied */
#define EFAULT          14              /* Bad address */
#define ENOTBLK         15              /* Block device required */
#define EBUSY           16              /* Mount device busy */
#define EEXIST          17              /* File exists */
#define EXDEV           18              /* Cross-device link */
#define ENODEV          19              /* No such device */
#define ENOTDIR         20              /* Not a directory */
#define EISDIR          21              /* Is a directory */
#define EINVAL          22              /* Invalid argument */
#define ENFILE          23              /* File table overflow */
#define EMFILE          24              /* Too many open files */
#define ENOTTY          25              /* Not a typewriter */
#define ETXTBSY         26              /* Text file busy */
#define EFBIG           27              /* File too large */
#define ENOSPC          28              /* No space left on device */
#define ESPIPE          29              /* Illegal seek */
#define EROFS           30              /* Read-only file system */
#define EMLINK          31              /* Too many links */
#define EPIPE           32              /* Broken pipe */

/* math software */
#define EDOM            33              /* Argument too large */
#define ERANGE          34              /* Result too large */

#define ENAMETOOLONG    63              /* File name too long */
