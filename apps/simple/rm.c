#include <string.h>
#include <stdio.h>
#include <unix.h>

#define lstat stat

int stat(), unlink();

char *basename(name)
char *name;
{
    char *base;

    base = rindex(name, '/');
    return base ? base + 1 : name;
}


int main(argc, argv)
int argc;
char **argv;
{
    int i /*, recurse = 0, interact =0 */ ;
    struct stat sbuf;

/*
    if (((argv[1][0] == '-') && (argv[1][1] == 'r')) ||
        ((argv[2][0] == '-') && (argv[2][1] == 'r'))) 
	recurse = 1;

    if (((argv[1][0] == '-') && (argv[1][1] == 'i')) ||
        ((argv[2][0] == '-') && (argv[2][1] == 'i')))
	interact = 1;        
*/

    for (i = /* recurse + interact + */ 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (!lstat(argv[i], &sbuf)) {
		if (unlink(argv[i])) {
		    fprintf(stderr, "rm: could not remove %s\n", argv[i]);
		}
	    }
	}
    }

    return 0;
}
