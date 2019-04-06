#include <stdio.h>
#include <unix.h>

char *month[] = { "Jan", "Feb", "Mar", "Apr",
                  "May", "Jun", "Jul", "Aug",
                  "Sep", "Oct", "Nov", "Dec" };

main(argc, argval)
int argc;
char *argval[];
{
    if (argc < 2)
        ls(".");
    else
        ls(argval[1]);
}

prmode(mode)
int mode;
{
    if (mode & 4)
        printf("r");
    else
        printf("-");

    if (mode & 2)
        printf("w");
    else
        printf("-");

    if (mode & 1)
        printf("x");
    else
        printf("-");
}

ls(path)
char *path;
{
    int    d, st, year;
    struct direct buf;
    struct stat statbuf;
    char   dname[128];
    int    _open(), _read(), _stat();

    if (_stat(path, &statbuf) != 0 || (statbuf.st_mode & F_MASK) != F_DIR) {
        printf("ls: can't stat %s\n", path);
        return -1;
    }

    d = _open(path, 0);
    if (d < 0) {
        printf ("ls: can't open %s\n", path);
        return -1;
    }

    while (_read(d, (char *)&buf, 16) == 16) {
        if (buf.d_name[0] == '\0')
            continue;

        if (path[0] != '.' || path[1]) {
            strcpy(dname, path);
            strcat(dname, "/");
        } else {
            dname[0] = '\0';
        }

        strcat(dname, buf.d_name);

        if (_stat(dname, &statbuf) != 0) {
            printf("ls: can't stat %s\n", dname);
            break;
        }

        st = statbuf.st_mode & F_MASK;

        if (st == F_DIR)
            printf("d");
        else if (st == F_CDEV)
            printf("c");
        else if (st == F_BDEV)
            printf("b");
        else if (st == F_PIPE)
            printf("p");
        else if ((st & F_REG) == 0)
            printf("l");
        else
            printf("-");

        prmode(statbuf.st_mode >> 6);
        prmode(statbuf.st_mode >> 3);
        prmode(statbuf.st_mode);

        printf("%4d %5d", statbuf.st_nlink, statbuf.st_ino);
        if ((statbuf.st_mode & F_MASK) == F_DIR)
            strcat(dname, "/");
        else if ((statbuf.st_mode & MODE_MASK) & OWN_EX)
            strcat(dname, "*");

        printf("%12u ",
                (statbuf.st_mode & F_CDEV) ?
                    statbuf.st_rdev :
                    512 * statbuf.st_size.o_blkno + statbuf.st_size.o_offset);

        if (statbuf.st_mtime.t_time == 0) {  /* st_mtime? */
            printf("                   ");
        } else {
            year = ((statbuf.st_mtime.t_date >> 9) & 0x7F) + 1900;
            if (year < 1978) year += 100;

            printf("%2s %02d %4d   ",
                   month[((statbuf.st_mtime.t_date >> 5) & 0x0F)-1], 
                   (statbuf.st_mtime.t_date & 0x1F),
                   year);

            printf("%2d:%02d",
                   ((statbuf.st_mtime.t_time >> 11) & 0x1F),
                   ((statbuf.st_mtime.t_time >> 5) & 0x3F)/*,
                   (statbuf.st_mtime.t_time & 0x1F)*/);
        }

        printf("  %-15s\n", dname);
    }
    _close (d);
}
