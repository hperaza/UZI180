/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 */

#include <stdio.h>
#include <unix.h>

int more(FILE *fp, int morefiles);
int nextpage();

int main(argc, argv)
int argc; char **argv;
{
    FILE *fp;
    char *name, *getenv();
    int  next, dup(), open();
    
    if (argc == 1) {

        next = dup(0);
        close(0);
        name = getenv("CTTY");
        if (!name) {
            fprintf(stderr, "%s: CTTY not set\n", argv[0]);
            return -1;
        }
        if (open(name, O_RDWR) == 0) {
	    fp = fdopen(next, "r");
            if (fp) more(fp, 0);
        } else {
	    perror("more (CTTY)");
	}

    } else while (argc-- > 1) {

	name = *(++argv);

	fp = fopen(name, "r");
	if (fp == NULL) {
	    perror(name);
	    return 1;
	}
	
	printf("<< %s >>\n", name);
	next = more(fp, (argc > 0));
	fclose(fp);
	if (!next) break;
    }
    
    return 0;
}

int more(FILE *fp, int morefiles)
{
    int ch, line, col;

    line = 1;
    col = 0;

    while ((ch = fgetc(fp)) != EOF) {
	switch (ch) {
	case '\r':
	    col = 0;
	    break;

	case '\n':
	    line++;
	    col = 0;
	    break;

	case '\t':
	    col = ((col + 1) | 0x07) + 1;
	    break;

	case '\b':
	    if (col > 0) --col;
	    break;

	default:
	    col++;
	}

	putchar(ch);
	if (col >= 80) {
	    col -= 80;
	    ++line;
	}
	if (line < 24)
	    continue;

	if (col > 0)
	    putchar('\n');

	ch = nextpage();

	switch (ch) {
	case 'N':	/* next file */
	case 'n':
	    return 1;

	case 'Q':	/* quit */
	case 'q':
	    return 0;
	}

	col = 0;
	line = 1;
    }

    if (morefiles) {
	ch = nextpage();
	if (ch == 'q' || ch == 'Q') return 0;
    }

    return 1;
}

int nextpage()
{
    char buf[80];
    int  read();

    printf("--More--");
    fflush(stdout);

    if (read(0, buf, sizeof(buf)) < 0) return 0;
    return (buf[0] == ':' ? buf[1] : buf[0]);
}
