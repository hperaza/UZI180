/*
 * vprintf - formatted output without ellipsis to the standard output stream
 */

#include	<stdio.h>
#include	<stdarg.h>

extern int	_doprnt();

int
vprintf(fmt, args)
char *	fmt;
va_list	args;
{
	return _doprnt(stdout, fmt, args);
}
