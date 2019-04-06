/*
 * vscanf.c - read formatted input from the standard input stream
 */

#include	<stdio.h>
#include	<stdarg.h>

extern int	_doscan();

int
vscanf(fmt, args)
char *	fmt;
va_list	args;
{
	return _doscan(stdin, fmt, args);
}
