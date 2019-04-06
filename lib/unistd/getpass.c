/* getpass - read a password, UZI180 version */

#include <unix.h>
#include <string.h>

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

#define TIOCGETP  0
#define TIOCSETP  1
#define TIOCSETN  2
#define TIOCTLSET 9     /* Don't parse ctrl-chars */
#define TIOCTLRES 10    /* Normal Parse */

#define XTABS   0006000
#define RAW     0000040
#define CRMOD   0000020
#define ECHO    0000010
#define LCASE   0000004
#define CBREAK  0000002
#define COOKED  0000000

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;
};

extern int ioctl(), open(), read(), close(), isatty();


char *getpass(prompt)
char *prompt;
{
    struct tty_data ttystat, ttyraw;
    int n, fd;
    static char password[32 + 1];

    /* try to open the controlling terminal */
    if ((fd = open(getenv("CTTY"), O_RDONLY)) < 0)
	fd = STDIN_FILENO;

    if (!isatty(fd)) return NULL;

    /* set the terminal to raw and non-echo mode,
       disable ctrl char processing */
    ioctl(fd, TIOCGETP, (char *) &ttystat);
    ttyraw = ttystat;
    ttyraw.t_flags = (ttyraw.t_flags & 0xFFC4) | 0x20;
    ioctl(fd, TIOCSETP, (char *) &ttyraw);
    ioctl(fd, TIOCTLSET, NULL);

    /* display the prompt */
    write(STDERR_FILENO, prompt, strlen(prompt));

    /* zero the password buffer */
    memset(password, 0, 32 + 1);

    /* read the password, 32 characters max */
    n = 0;
    while (read(fd, password + n, 1) > 0) {
	if (password[n] == '\n' || password[n] == '\r') break;
	if (n < 32) ++n;
    }
    password[n] = '\0';

    /* set the terminal back to cooked mode */
    ioctl(fd, TIOCSETP, (char *) &ttystat);
    ioctl(fd, TIOCTLRES, NULL);

    write(STDERR_FILENO, "\n", 1);
            
    if (fd != STDIN_FILENO) close(fd);

    return password;
}
