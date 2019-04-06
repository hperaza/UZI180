#include "stdio.h"
#include "unix.h"

struct utimbuf {
    time_t actime;  /* access time */
    time_t modtime; /* modification time */
};

time_t stime = { 0, 0 };

int main(argc, argv)
int argc;
char **argv;
{
    int er, i, ncreate = 0;
    struct stat sbuf;
    struct utimbuf tbuf;
    int stat(), creat(), close(), time(), utime();

    if ((argv[1][0] == '-') && (argv[1][1] == 'c'))
	ncreate = 1;

    for (i = ncreate + 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (stat(argv[i], &sbuf)) {
		if (!ncreate)
		    er = close(creat(argv[i], 0666));
	    } else {
	        er = time(&stime);
	        if (!er) {
	            tbuf.actime = stime;
	            tbuf.modtime = stime;
		    er = utime(argv[i], &tbuf);
		}
	    }
	}
    }
    return er;
}
