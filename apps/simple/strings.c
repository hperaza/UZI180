/* strings - print ASCII strings in a file	Author: Peter S. Housel */
/* UZI180 port by H. Peraza */

/*
  This is a version of the BSD "strings" program for UZI180. It is used
  to search a file for printable strings.

  Command:  strings - search file for printable strings

  Syntax:   strings [-o] [-len] file ...
  Flags:    -o   Print the offset (in octal) with each string.
            -len Use "len" as the minimum string length. The default is 4.

  Examples: strings /bin/ls
            strings -o /bin/cp > str

  Strings searches the specified file(s) for printable ASCII strings (four
  or more printable characters followed by a newline or a null) and writes
  them to the standard output. This can be used to find out, for example, to
  find out what program a "core" file came from, what kinds of error messages
  are in an executable, or to see ASCII data hidden in a "binary" data file.

*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Miscellaneous definitions */

#define	STRLEN		4	/* default minimum string length */
#define STRBUF		512	/* buffer length for strings */

int  main(int argc, char **argv);
void strings(char *filename);
void usage(void);

int strmin = STRLEN;		/* minimum string length */
int printoff = 0;		/* print octal offset of each str */

/* Just initialized data segment */

int main(int argc, char **argv)
{
    while ((++argv, --argc) && '-' == (*argv)[0]) {
	if (!strcmp(*argv, "-o"))
	    ++printoff;
	else if (isdigit((*argv)[1]))
	    strmin = atoi(&(*argv)[1]);
	else
	    usage();
    }

    if (argc == 0) usage();

    while (argc--) strings(*argv++);

    return 0;
}

void strings(char *filename)
{
    char *bufptr;		/* pointer into the strings buffer */
    FILE *input;		/* input file */
    long offset;		/* file offset */
    int c;			/* input character */
    char buf[STRBUF];		/* the strings buffer */

    input = fopen(filename, "r");
    if (input == NULL) {
	fprintf(stderr, "strings: ");
	perror(filename);
	exit(1);
    }

    offset = 0L;
    bufptr = buf;

    for (;;) {
	c = getc(input);
	if (c == EOF) break;
	if (((c == '\0' || c == '\n') && bufptr - buf >= strmin)
	    || (bufptr - buf == STRBUF - 1)) {
	    *bufptr = '\0';
	    if (printoff)
		printf("%lo:", offset - (bufptr - buf));
	    puts(buf);
	    bufptr = buf;
	} else if ((c >= ' ' && c < 0177) || c == '\t') {
	    *bufptr++ = c;
	} else {
	    bufptr = buf;
	}

	++offset;
    }

    fclose(input);
}

void usage()
{
    fprintf(stderr, "usage: strings [-o] [-num] file ...\n");
    exit(1);
}
