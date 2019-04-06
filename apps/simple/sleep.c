/* sleep - delay for a specified amount of time.
   Copyright (C) 84, 91, 92, 93, 1994 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>

long argdecode();
int alarm(), pause();

/* The name by which this program was run. */
char *program_name;


static void usage()
{
    printf("Usage: %s number[d|h|m|s]\n", program_name);
    exit(1);
}

void main(argc, argv)
int argc;
char **argv;
{
    int i;
    unsigned long seconds = 0L;

    program_name = argv[0];

    if (argc == 1) {
	usage();
    }
    for (i = 1; i < argc; i++)
	seconds += argdecode(argv[i]);

#if 0
    sleep(seconds);
#else
    alarm(seconds);
    pause();
#endif

    exit(0);
}

long argdecode(s)
char *s;
{
    long value;
    register char *p = s;
    register char c;

    value = 0;
    while ((c = *p++) >= '0' && c <= '9')
	value = value * 10L + (long) (c - '0');

    switch (c) {
    case 's':
	break;
    case 'm':
	value *= 60L;
	break;
    case 'h':
	value *= 60L * 60L;
	break;
    case 'd':
	value *= 60L * 60L * 24L;
	break;
    default:
	p--;
    }

    if (*p) {
	fprintf(stderr, "%s: invalid time interval `%s'\n", program_name, s);
	exit(1);
    }

    return value;
}
