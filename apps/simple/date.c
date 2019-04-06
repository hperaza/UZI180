#include <stdio.h>
#include <time.h>

long convuzitime(time_t *);

int  time_zone = 0;

int main()
{
    time_t now;
    long lnow;

    time(&now);
    lnow = convuzitime(&now);
    fputs(ctime(&lnow), stdout);

    return 0;
}
