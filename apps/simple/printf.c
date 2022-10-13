/* printf - format and print data
   Copyright (C) 90, 91, 92, 93, 1994 Free Software Foundation, Inc.

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

/* Usage: printf format [argument...]

   A front end to the printf function that lets it be used from the shell.

   Backslash escapes:

   \" = double quote
   \\ = backslash
   \a = alert (bell)
   \b = backspace
   \c = produce no further output
   \f = form feed
   \n = new line
   \r = carriage return
   \t = horizontal tab
   \v = vertical tab
   \0ooo = octal number (ooo is 0 to 3 digits)
   \xhhh = hexadecimal number (hhh is 1 to 3 digits)

   Additional directive:

   %b = print an argument string, interpreting backslash escapes

   The `format' argument is re-used as many times as necessary
   to convert all of the given arguments.

   David MacKenzie <djm@gnu.ai.mit.edu> */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#if 0
#include <getopt.h>
#endif

#include "unix.h"

#ifndef STDC_HEADERS
double strtod(char *nptr, char **endptr);
long strtol(char *nptr, char **endptr, int base);
unsigned long strtoul(char *nptr, char **endptr, int base);
#endif

#define isodigit(c) ((c) >= '0' && (c) <= '7')
#define hextobin(c) ((c)>='a'&&(c)<='f' ? (c)-'a'+10 : (c)>='A'&&(c)<='F' ? (c)-'A'+10 : (c)-'0')
#define octtobin(c) ((c) - '0')

char *xmalloc(size_t size);
void error(int, int, char *, ...);

static double xstrtod(char *s);
static int print_esc(char *escstart);
static int print_formatted(char *format, int argc, char **argv);
static long xstrtol(char *s);
static unsigned long xstrtoul(char *s);
static void print_direc();
static void print_esc_char(char c);
static void print_esc_string(char *str);
static void verify(char *s, char *end);

/* The value to return to the calling program.  */
static int exit_status;

/* The name this program was run with. */
char *program_name;

static void usage(int status)
{
    if (status != 0)
	fprintf(stderr, "Try '%s -h' for more information.\n",
		program_name);
    else {
	printf("Usage: %s FORMAT [ARGUMENT]...\n"
               "  or:  %s -h\n", program_name, program_name);
	printf("\n"
	       "  -h      display this help and exit\n"
	       "\n"
	       "FORMAT controls the output as in C printf.  Interpreted sequences are:\n"
	       "\n"
	       "  \\\"      double quote\n"
	       "  \\0NNN   character with octal value NNN (0 to 3 digits)\n"
	       "  \\\\      backslash\n"
	       "  \\a      alert (BEL)\n"
	       "  \\b      backspace\n"
	       "  \\c      produce no further output\n"
	       "  \\f      form feed\n"
	       "  \\n      new line\n"
	       "  \\r      carriage return\n"
	       "  \\t      horizontal tab\n"
	       "  \\v      vertical tab\n"
	       "  \\xNNN   character with hexadecimal value NNN (1 to 3 digits)\n"
	       "\n"
	       "  %%%%      a single %%\n"
	       "  %%b      ARGUMENT as a string with '\\' escapes interpreted\n"
	       "\n"
	       "and all C format specifications ending with one of diouxXfeEgGcs, with\n"
	       "ARGUMENTs converted to proper type first.  Variable widths are handled.\n");
    }
    exit(status);
}

void main(int argc, char **argv)
{
    char *format;
    int args_used;

    program_name = argv[0];
    exit_status = 0;

    if (argc == 1) {
	fprintf(stderr, "Usage: %s format [argument...]\n", program_name);
	exit(1);
    }

    if (argv[1][0] == '-') {
      if (argv[1][1] == 'h') usage(0); else usage(1);
    }

    format = argv[1];
    argc -= 2;
    argv += 2;

    do {
	args_used = print_formatted(format, argc, argv);
	argc -= args_used;
	argv += args_used;
    }
    while (args_used > 0 && argc > 0);

    exit(exit_status);
}

/* Print the text in FORMAT, using ARGV (with ARGC elements) for
   arguments to any '%' directives.
   Return the number of elements of ARGV used.  */

static int print_formatted(char *format, int argc, char **argv)
{
    int save_argc = argc;	/* Preserve original value.  */
    char *f;			/* Pointer into 'format'.  */
    char *direc_start;		/* Start of % directive.  */
    size_t direc_length;	/* Length of % directive.  */
    int field_width;		/* Arg to first '*', or -1 if none.  */
    int precision;		/* Arg to second '*', or -1 if none.  */

    for (f = format; *f; ++f) {
	switch (*f) {
	case '%':
	    direc_start = f++;
	    direc_length = 1;
	    field_width = precision = -1;
	    if (*f == '%') {
		putchar('%');
		break;
	    }
	    if (*f == 'b') {
		if (argc > 0) {
		    print_esc_string(*argv);
		    ++argv;
		    --argc;
		}
		break;
	    }
	    if (index("-+ #", *f)) {
		++f;
		++direc_length;
	    }
	    if (*f == '*') {
		++f;
		++direc_length;
		if (argc > 0) {
		    field_width = xstrtoul(*argv);
		    ++argv;
		    --argc;
		} else
		    field_width = 0;
	    } else
		while (isdigit(*f)) {
		    ++f;
		    ++direc_length;
		}
	    if (*f == '.') {
		++f;
		++direc_length;
		if (*f == '*') {
		    ++f;
		    ++direc_length;
		    if (argc > 0) {
			precision = xstrtoul(*argv);
			++argv;
			--argc;
		    } else
			precision = 0;
		} else
		    while (isdigit(*f)) {
			++f;
			++direc_length;
		    }
	    }
	    if (*f == 'l' || *f == 'L' || *f == 'h') {
		++f;
		++direc_length;
	    }
	    if (!index("diouxXfeEgGcs", *f))
		error(1, 0, "%%%c: invalid directive", *f);
	    ++direc_length;
	    if (argc > 0) {
		print_direc(direc_start, direc_length, field_width,
			    precision, *argv);
		++argv;
		--argc;
	    } else
		print_direc(direc_start, direc_length, field_width,
			    precision, "");
	    break;

	case '\\':
	    f += print_esc(f);
	    break;

	default:
	    putchar(*f);
	}
    }

    return save_argc - argc;
}

/* Print a \ escape sequence starting at ESCSTART.
   Return the number of characters in the escape sequence
   besides the backslash. */

static int print_esc(char *escstart)
{
    register char *p = escstart + 1;
    int esc_value = 0;		/* Value of \nnn escape. */
    int esc_length;		/* Length of \nnn escape. */

    /* \0ooo and \xhhh escapes have maximum length of 3 chars. */
    if (*p == 'x') {
	for (esc_length = 0, ++p;
	     esc_length < 3 && isxdigit(*p); ++esc_length, ++p)
	    esc_value = esc_value * 16 + hextobin(*p);
	if (esc_length == 0)
	    error(1, 0, "missing hexadecimal number in escape");
	putchar(esc_value);
    } else if (*p == '0') {
	for (esc_length = 0, ++p;
	     esc_length < 3 && isodigit(*p); ++esc_length, ++p)
	    esc_value = esc_value * 8 + octtobin(*p);
	putchar(esc_value);
    } else if (index("\"\\abcfnrtv", *p))
	print_esc_char(*p++);
    else
	error(1, 0, "\\%c: invalid escape", *p);
    return p - escstart - 1;
}

/* Output a single-character \ escape.  */

static void print_esc_char(char c)
{
    switch (c) {
    case 'a':			/* Alert. */
	putchar(7);
	break;
    case 'b':			/* Backspace. */
	putchar(8);
	break;
    case 'c':			/* Cancel the rest of the output. */
	exit(0);
	break;
    case 'f':			/* Form feed. */
	putchar(12);
	break;
    case 'n':			/* New line. */
	putchar(10);
	break;
    case 'r':			/* Carriage return. */
	putchar(13);
	break;
    case 't':			/* Horizontal tab. */
	putchar(9);
	break;
    case 'v':			/* Vertical tab. */
	putchar(11);
	break;
    default:
	putchar(c);
	break;
    }
}

/* Print string STR, evaluating \ escapes. */

static void print_esc_string(char *str)
{
    for (; *str; str++)
	if (*str == '\\')
	    str += print_esc(str);
	else
	    putchar(*str);
}

/* Output a % directive.  START is the start of the directive,
   LENGTH is its length, and ARGUMENT is its argument.
   If FIELD_WIDTH or PRECISION is non-negative, they are args for
   '*' values in those fields. */

static void print_direc(start, length, field_width, precision, argument)
char *start;
size_t length;
int field_width;
int precision;
char *argument;
{
    char *p;			/* Null-terminated copy of % directive. */

    p = xmalloc((unsigned) (length + 1));
    strncpy(p, start, length);
    p[length] = 0;

    switch (p[length - 1]) {
    case 'd':
    case 'i':
	if (field_width < 0) {
	    if (precision < 0)
		printf(p, xstrtol(argument));
	    else
		printf(p, precision, xstrtol(argument));
	} else {
	    if (precision < 0)
		printf(p, field_width, xstrtol(argument));
	    else
		printf(p, field_width, precision, xstrtol(argument));
	}
	break;

    case 'o':
    case 'u':
    case 'x':
    case 'X':
	if (field_width < 0) {
	    if (precision < 0)
		printf(p, xstrtoul(argument));
	    else
		printf(p, precision, xstrtoul(argument));
	} else {
	    if (precision < 0)
		printf(p, field_width, xstrtoul(argument));
	    else
		printf(p, field_width, precision, xstrtoul(argument));
	}
	break;

    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
	if (field_width < 0) {
	    if (precision < 0)
		printf(p, xstrtod(argument));
	    else
		printf(p, precision, xstrtod(argument));
	} else {
	    if (precision < 0)
		printf(p, field_width, xstrtod(argument));
	    else
		printf(p, field_width, precision, xstrtod(argument));
	}
	break;

    case 'c':
	printf(p, *argument);
	break;

    case 's':
	if (field_width < 0) {
	    if (precision < 0)
		printf(p, argument);
	    else
		printf(p, precision, argument);
	} else {
	    if (precision < 0)
		printf(p, field_width, argument);
	    else
		printf(p, field_width, precision, argument);
	}
	break;
    }

    free(p);
}

static unsigned long xstrtoul(char *s)
{
    char *end;
    unsigned long val;

    errno = 0;
    val = strtoul(s, &end, 0);
    verify(s, end);
    return val;
}

static long xstrtol(char *s)
{
    char *end;
    long val;

    errno = 0;
    val = strtol(s, &end, 0);
    verify(s, end);
    return val;
}

static double xstrtod(char *s)
{
    char *end;
    double val;

    errno = 0;
    val = strtod(s, &end);
    verify(s, end);
    return val;
}

static void verify(char *s, char *end)
{
    if (errno) {
	error(0, errno, "%s", s);
	exit_status = 1;
    } else if (*end) {
	if (s == end)
	    error(0, 0, "%s: expected a numeric value", s);
	else
	    error(0, 0, "%s: value not completely converted", s);
	exit_status = 1;
    }
}

/*-----------------------------------------------------------------*/

void error(int status, int errsys, char *fmt, ...) {
    va_list args;
        
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if (errsys) perror("");
    if (status) exit(status);
}

char *xmalloc(size_t size) {
    char *p = malloc(size);
    if (!p) {
	fprintf(stderr, "out of memory\n");
	exit(1);
    }
    return p;
}

static unsigned long
string2long(char *nptr, char **endptr, int base, int is_signed);

long int strtol(char *nptr, char **endptr, int base)
{
    return (long) string2long(nptr, endptr, base, 1);
}

unsigned long int strtoul(char *nptr, char **endptr, int base)
{
    return string2long(nptr, endptr, base, 0);
}

#define between(a, c, z)  ((unsigned) ((c) - (a)) <= (unsigned) ((z) - (a)))

static unsigned long
string2long(char *nptr, char **endptr, int base, int is_signed)
{
    register unsigned int v;
    register unsigned long val = 0;
    register int c;
    int ovfl = 0, sign = 1;
    char *startnptr = nptr, *nrstart;

    if (endptr) *endptr = (char *)nptr;
    while (isspace(*nptr)) nptr++;
    c = *nptr;

    if (c == '-' || c == '+') {
	if (c == '-') sign = -1;
	nptr++;
    }
    nrstart = nptr;			/* start of the number */

    /* When base is 0, the syntax determines the actual base */
    if (base == 0) {
	if (*nptr == '0') {
	    if (*++nptr == 'x' || *nptr == 'X') {
		base = 16;
		nptr++;
	    } else {
		base = 8;
	    }
	} else {
	    base = 10;
	}
    } else if (base == 16 && *nptr == '0' &&
	       (*++nptr =='x' || *nptr =='X')) {
	nptr++;
    }

    for (;;) {
	c = *nptr;
	if (between('0', c, '9')) {
	    v = c - '0';
	} else if (between('a', c, 'z')) {
	    v = c - 'a' + 0xa;
	} else if (between('A', c, 'Z')) {
	    v = c - 'A' + 0xA;
	} else {
	    break;
	}
	if (v >= base) break;
	if (val > (ULONG_MAX - v) / base) ovfl++;
	val = (val * base) + v;
	nptr++;
    }

    if (endptr) {
	if (nrstart == nptr)
	    *endptr = (char *) startnptr;
	else
	    *endptr = (char *) nptr;
    }

    if (!ovfl) {
	/* Overflow is only possible when converting a signed long. */
	if (is_signed && ((sign < 0 && val > - (unsigned long) LONG_MIN)
			|| (sign > 0 && val > LONG_MAX)))
	    ovfl++;
    }

    if (ovfl) {
	errno = ERANGE;
	if (is_signed) {
	    if (sign < 0)
		return LONG_MIN;
	    else
	    	return LONG_MAX;
	} else {
	    return ULONG_MAX;
	}
    }

    return (long) sign * val;
}


double strtod(char *nptr, char **endptr)
{
    char   c, sign, expsign;
    double val, ten;
    short  exp, eexp;
    char *startnptr = nptr, *nrstart;

    if (endptr) *endptr = (char *) nptr;
    while (isspace(*nptr)) nptr++;

    ten = 10.0;
    expsign = sign = 0;
    val = 0.0;
    exp = 0;
    
    c = *nptr;
    if ((c == '-') || (c == '+')) {
	if (c == '-') sign = 1;
	nptr++;
    }
    nrstart = nptr;			/* start of the number */

    while (isdigit(*nptr))
	val = ten * val + (double) (*nptr++ - '0');

    if (*nptr == '.') {
	for (;;) {
	    c = *++nptr;
	    if (isdigit(c)) {
		exp--;
		val = ten * val + (double) (c - '0');
	    } else
		break;
	}
    }
	
    eexp = 0;
    if (*nptr == 'e' || *nptr == 'E') {
	if (*++nptr == '-') {
	    expsign = 1;
	    nptr++;
	}
	if (*nptr == '+')
	    nptr++;
	while (isdigit(*nptr))
	    eexp = eexp * 10 + *nptr++ - '0';
	if (expsign)
	    eexp = -eexp;
    }

    exp += eexp;
    while (exp < 0) {
	val = val / ten;
	exp++;
    }
    while (exp > 0) {
	val = val * ten;
	exp--;
    }

    if (sign) val = -val;

    if (endptr) {
	if (nrstart == nptr)
	    *endptr = (char *) startnptr;
	else
	    *endptr = (char *) nptr;
    }

    return val;
}
