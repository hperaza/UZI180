#include <stdio.h>
#include <unix.h>

char *month[] = { "Jan", "Feb", "Mar", "Apr",
                  "May", "Jun", "Jul", "Aug",
                  "Sep", "Oct", "Nov", "Dec" };

time_t stime = { 0, 0 };

main(argc, argv)
int argc;
char *argv[];
{
    int retc, year, _time();

    retc = _time(&stime);
    if (retc != 0) {
        printf("_time: error %d\n", retc);
        return 1;
    }

    year = ((stime.t_date >> 9) & 0x7F) + 1900;
    if (year < 1978) year += 100;

    printf("%d-%s-%d ",
                (stime.t_date & 0x1F),
                month[((stime.t_date >> 5) & 0x0F)-1], 
                year);

    printf("%2d:%02d:%02d\n",
                ((stime.t_time >> 11) & 0x1F),
                ((stime.t_time >> 5) & 0x3F),
                (stime.t_time & 0x1F));

    return 0;
}
