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
  { 0,  fd_open,     ok,    fd_read,  fd_write,   nogood },   /*  0 /dev/fd0  */
  { 1,  fd_open,     ok,    fd_read,  fd_write,   nogood },   /*  1 /dev/fd1  */
  { 0,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  2 /dev/hda1 */
  { 1,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  3 /dev/hda2 */
  { 2,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  4 /dev/hda3 */
  { 3,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  5 /dev/hda4 */
  { 4,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  6 /dev/hda5 */
  { 5,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  7 /dev/hda6 */
  { 6,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  8 /dev/hda7 */
  { 7,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /*  9 /dev/hda8 */
  { 8,  wd_open,     ok,    wd_read,  wd_write,   nogood },   /* 10 Swap      */
  { 0, lpr_open, lpr_close, nogood,   lpr_write,  nogood },   /* 11 /dev/lp   */
  { 1, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 12 /dev/tty1 */
  { 2, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 13 /dev/tty2 */
  { 0,     ok,       ok,    null_read, null_write, nogood },  /* 14 /dev/null */
  { 0,     ok,       ok,    zero_read, zero_write, nogood },  /* 15 /dev/zero */
  { 0,     ok,       ok,    mem_read, mem_write,  nogood },   /* 16 /dev/kmem */
  { 0,     ok,       ok, proc_read, proc_write, proc_ioctl }, /* 17 /dev/proc */
  { 0,  mt_open,  mt_close, mt_read,  mt_write,   nogood }    /* 18 /dev/mt   */
  /* Add more tty channels here if available, incrementing minor# */
};

#endif

#define BOOT_TTY 12       /* Set this to default device for stdio, stderr */
                          /* In this case, the default is the first TTY device */

#define NDEVS    10       /* Devices 0..NDEVS-1 are capable of being mounted */
                          /*  (add new mountable devices to beginning area.) */
#define TTYDEV   BOOT_TTY /* Device used by kernel for messages, panics */
#define SWAPDEV  10       /* Device for swapping. */
#define NBUFS    10       /* Number of block buffers */
