/**************************************************
UZI (Unix Z80 Implementation) Kernel:  config.h
***************************************************/
/* ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! */
/* ! ! !  Remake devio.c when this file is changed ! ! ! */
/* ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! */

#ifdef DEVIO

extern wd_open(), wd_read(),wd_write();
extern fd_open(), fd_read(),fd_write();
extern tty_open(), tty_close(), tty_read(), tty_write(), tty_ioctl();
extern lpr_open(), lpr_close(), lpr_write();
extern mem_read(), mem_write();
extern proc_read(), proc_write(), proc_ioctl();
extern mt_read(), mt_write(), mt_open(), mt_close();
extern zero_read(), zero_write(), null_read(), null_write();

static struct devsw dev_tab[] =  /* The device driver switch table */
{
/* minor  open     close     read      write      ioctl */
/*------------------------------------------------------*/
  { 0,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /* 0 /dev/hd0  */
  { 0,  fd_open,     ok,    fd_read,  fd_write,   nogood },   /* 1 /dev/fd0  */
  { 1,  fd_open,     ok,    fd_read,  fd_write,   nogood },   /* 2 /dev/fd1  */
  { 2,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /* 3 Swap      */
  { 0, lpr_open, lpr_close, nogood,   lpr_write,  nogood },   /* 4 /dev/lp   */
  { 1, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 5 /dev/tty1 */
  { 0,     ok,       ok,    null_read, null_write, nogood },  /* 6 /dev/null */
  { 0,     ok,       ok,    zero_read, zero_write, nogood },  /* 7 /dev/zero */
  { 0,     ok,       ok,    mem_read, mem_write,  nogood },   /* 8 /dev/kmem */
  { 0,     ok,       ok, proc_read, proc_write, proc_ioctl }, /* 9 /dev/proc */
  { 0,  mt_open,  mt_close, mt_read,  mt_write,   nogood },  /* 10 /dev/mt   */
  { 2, tty_open, tty_close, tty_read, tty_write, tty_ioctl } /* 11 /dev/tty1 */
  /* Add more tty channels here if available, incrementing minor# */
};

#endif

#define BOOT_TTY  5    /* Set this to default device for stdio, stderr */
                       /* In this case, the default is the first TTY device */

#define NDEVS   3         /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*   (add new mountable devices to beginning area. */
#define TTYDEV  BOOT_TTY  /* Device used by kernel for messages, panics */
#define SWAPDEV 3         /* Device for swapping. */
#define NBUFS   10         /* Number of block buffers */
