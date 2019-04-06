/* isatty -- UZI180 version */

#define TIOCGETP  0

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;
};

extern int ioctl();

int isatty(fd)
int fd;
{
    struct tty_data dummy;

    return (ioctl(fd, TIOCGETP, (char *) &dummy) == 0);
}
