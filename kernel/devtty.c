/***************************************************************
   UZI (Unix Z80 Implementation) Kernel:  devtty.c
----------------------------------------------------------------
 Adapted from UZI By Doug Braun, and UZI280 by Stefan Nitschke
            Copyright (C) 1998 by Harold F. Bower
       Portions Copyright (C) 1995 by Stefan Nitschke
****************************************************************/
/* Revisions:
 *  22.12.97 - Moved __putc & tty_int to MACHASM.ASZ.	     HFB
 *  24.05.98 - Restructured tty_inproc, parse CTRL chars
 *	based on flag set bit (deletes CPM_CTRL in 280), add
 *	equate for Number of TTY terminals.		     HFB
 */

#define DEBUG            /* UNdefine to delete debug code sequences */

#define NTTYS  2         /* Number of TTY ports defined */

#include <unix.h>
#include <config.h>
#include <extern.h>

#define TIOCGETP  0
#define TIOCSETP  1
#define TIOCSETN  2
#define TIOCEXCL  3     /** currently not implemented  SN **/
#define UARTSLOW  4     /* Normal interrupt routine (UZI280) */
#define UARTFAST  5     /* Fast interrupt routine for modem usage (UZI280) */
#define TIOCFLUSH 6
#define TIOCGETC  7
#define TIOCSETC  8
              /* UZI280 extensions used by UZI180 in the CP/M 2.2 Emulator */
#define TIOCTLSET 9     /* Don't parse ctrl-chars */
#define TIOCTLRES 10    /* Normal Parse */

#define XTABS   0006000
#define RAW     0000040
#define CRMOD   0000020
#define ECHO    0000010
#define LCASE   0000004
#define CBREAK  0000002
#define COOKED  0000000

#define DFLT_MODE  XTABS|CRMOD|ECHO|COOKED

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;

    char t_intr;
    char t_quit;
    char t_start;
    char t_stop;
    char t_eof;

    char ctl_char;
};

struct tty_data ttydata[NTTYS+1];        /* ttydata[0] is not used */

#define CTRL(c)  (c & 0x1f)

struct tty_data ttydflt = {
    0,         0,         '\b',       CTRL('x'), DFLT_MODE,
    CTRL('c'), CTRL('\\'), CTRL('q'), CTRL('s'), CTRL('d'), 0
};

/* Character Input Queue size */
#define TTYSIZ 132

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];

struct  s_queue  ttyinq[NTTYS+1] = {       /* ttyinq[0] is never used */
{   NULL,    NULL,    NULL,    0,        0,       0    },
{   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
{   tbuf2,   tbuf2,   tbuf2,   TTYSIZ,   0,   TTYSIZ/2 }
};

int stopflag[NTTYS+1];   /* Flag for ^S/^Q */
int flshflag[NTTYS+1];   /* Flag for ^O */

tty_read (minor, rawflag)
int16 minor;
int16 rawflag;
{
    int  nread;
    char c;
    int  remq();

       /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
        minor = udata.u_ptab->p_tty;
    ifnot (udata.u_ptab->p_tty)
        udata.u_ptab->p_tty = minor;

    if ((minor < 1) || (minor > NTTYS+1)) {
        udata.u_error = ENODEV;
        return (-1);
    }

    nread = 0;
    while (nread < udata.u_count)
    {
        for (;;)
        {
            di();
            if (remq (&ttyinq[minor], &c)) {
                if (udata.u_sysio)
                    *udata.u_base = c;
                else
                    uputc (c, udata.u_base);
                break;
            }
            psleep (&ttyinq[minor]);
            if (udata.u_cursig || udata.u_ptab->p_pending) {     /* messy */
                udata.u_error = EINTR;
                return (-1);
            }
        }
        ei();

        ++nread;

           /* In raw or cbreak mode, return after one char */
        if (ttydata[minor].t_flags & (RAW|CBREAK))
            break;

        if ((nread == 1) && (c == ttydata[minor].t_eof))   /* ^D */
            return(0);

        if (c == '\n')
            break;

        ++udata.u_base;
    } 
    return(nread);
}



tty_write (minor, rawflag)
int16 minor;
int16 rawflag;
{
    int towrite, c;
    int ugetc();

       /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
        minor = udata.u_ptab->p_tty;
    ifnot (udata.u_ptab->p_tty)
        udata.u_ptab->p_tty = minor;

    if ((minor < 1) || (minor > NTTYS+1)) {
        udata.u_error = ENODEV;
        return (-1);
    }

    towrite = udata.u_count;

    while (udata.u_count-- != 0)
    {
        for (;;)        /* Wait on the ^S/^Q flag */
        {
            di();
            ifnot (stopflag[minor])    
                break;
            psleep (&stopflag[minor]);
            if (udata.u_cursig || udata.u_ptab->p_pending)  /* messy */
            {
                udata.u_error = EINTR;
                return (-1);
            }
        }
        ei();
        
        ifnot (flshflag[minor])
        {
            if (udata.u_sysio)
                c = *udata.u_base;
            else
                c = ugetc (udata.u_base);

            if (c == '\n' && (ttydata[minor].t_flags & CRMOD))
                _putc (minor, '\r');
            _putc (minor, c);
        }
        ++udata.u_base;
    }
    return (towrite);
}



tty_open (minor)
int minor;
{
       /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
        minor = udata.u_ptab->p_tty;

       /* If there is no controlling tty for the process, establish it */
    ifnot (udata.u_ptab->p_tty)
        udata.u_ptab->p_tty = minor;

       /* Initialize the ttydata */
    bcopy (&ttydflt, &ttydata[minor], sizeof (struct tty_data));
    return (0);
}


tty_close (minor)
int minor;
{
       /* If we are closing the controlling tty, make note */
    if (minor == udata.u_ptab->p_tty)
        udata.u_ptab->p_tty = 0;
    return (0);
}


tty_ioctl (minor, request, data)           /* Data in User Space */
int minor;
int request;
char *data;
{
       /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
        minor = udata.u_ptab->p_tty;
    if ((minor < 1) || (minor > NTTYS+1)) {
        udata.u_error = ENODEV;
        return (-1);
    }
    switch (request)
    {
        case TIOCGETP:
            uput (&ttydata[minor], data, 6);
            break;
        case TIOCSETP:
            uget (data, &ttydata[minor], 6);
            break;
        case TIOCGETC:
            uput (&ttydata[minor].t_intr, data, 5);
            break;
        case TIOCSETC:
            uget (data, &ttydata[minor].t_intr, 5);
            break;
        case TIOCSETN:
            uput (&ttyinq[minor].q_count, data, 2);
            break;
        case TIOCFLUSH:
            clrq (&ttydata[minor]);
            break;
        case TIOCTLSET:
            ttydata[minor].ctl_char = 1;
            break;
        case TIOCTLRES:
            ttydata[minor].ctl_char = 0;
            break;
        default:
            udata.u_error = EINVAL;
            return (-1);
    }
    return (0);
}


/* This routine processes a character in response to an interrupt.  It
 * adds the character to the tty input queue, echoing and processing
 * backspace and carriage return.  If the queue contains a full line,
 * it wakes up anything waiting on it.  If it is totally full, it beeps
 * at the user.
 * UZI180 - This routine is called from the raw Hardware read routine,
 * either interrupt or polled, to process the input character.  HFB
 */

tty_inproc (minor, c)
int minor;
char c;
{
    char  oc;
    struct tty_data *td;
    int   mode;
    int   insq(), uninsq();

    td = &ttydata[minor];
    mode = td->t_flags & (RAW|CBREAK|COOKED);

/***    if (c == 0x1b && td->t_flags == DFLT_MODE)
/***            return;                   /* my terminal hates it SN */

    if (mode != RAW)
        c &= 0x7f;                /* Strip off parity */
    if (!c)
        return;                   /* Simply quit if Null character */

#ifdef DEBUG
    if (c == 0x1a)                /* ^Z */
        idump();                  /*   (For debugging) */
    if (c == 0x1) /* ^A */
    {
#asm
    global trap_ept
    call trap_ept
#endasm
    }
#endif

    if (td->ctl_char == 0)         /* Don't parse ctl chars if Non-0 */
    {
        if ((mode & RAW) == 0)       /* if mode == COOKED or CBREAK */
        {
            if (c == '\r' && (td->t_flags & CRMOD))
                c = '\n';

            if (c == td->t_intr) {                   /* ^C */
                sgrpsig (minor, SIGINT);
                clrq (&ttyinq[minor]);
                stopflag[minor] = flshflag[minor] = 0;
                return;
            }
            else if (c == td->t_quit) {              /* ^\ */
                sgrpsig (minor, SIGQUIT);
                clrq (&ttyinq[minor]);
                stopflag[minor] = flshflag[minor] = 0;
                return;
            }
            else if (c == '\017') {                  /* ^O */
                flshflag[minor] = !flshflag[minor];
                return;
            }
            else if (c == td->t_stop) {              /* ^S */
                stopflag[minor] = 1;
                return;
            }
            else if (c == td->t_start) {             /* ^Q */
                stopflag[minor] = 0;
                wakeup (&stopflag[minor]);
                return;
            }
        }

        if (mode == COOKED)
        {
            if (c == td->t_erase)
            {
                if (uninsq (&ttyinq[minor], &oc))
                {
                   if (oc == '\n')
                        insq (&ttyinq[minor], oc);  /* Don't erase past nl */
                    else
                    {
                        echo (minor, '\b');
                        echo (minor, ' ');
                        echo (minor, '\b');
                    }
                }
                return;
            }
            else if (c == td->t_kill)
            {
                while (uninsq (&ttyinq[minor], &oc))
                {
                    if (oc == '\n')
                    {
                        insq (&ttyinq[minor], oc);  /* Don't erase past nl */
                        break;
                    }
                    echo (minor, '\b');
                    echo (minor, ' ');
                    echo (minor, '\b');
                }
                return;
            }
        }
    }
       /* All modes come here */

    if (c == '\n' && (td->t_flags & CRMOD))
        echo (minor, '\r');

    if (insq (&ttyinq[minor], c))
        echo (minor, c);
    else
        _putc (minor, '\007');        /* Beep if no more room */

    if ((mode != COOKED) || ((c == '\n' || c == td->t_eof)
       && (td->ctl_char == 0)))                 /* ^D */
          wakeup (&ttyinq[minor]);
}


echo (minor, c)
int  minor;
char c;
{
    if (ttydata[minor].t_flags & ECHO)
        _putc (minor, c);
}
