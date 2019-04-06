/*
 * vsscanf - read formatted output from a string
 */

#include	<stdio.h>
#include	<stdarg.h>
#include	<string.h>

extern int	_doscan();

int vsscanf(str, fmt, args)
char *	str;
char *	fmt;
va_list	args;
{
	static FILE stream;

	stream._file = -1;
	stream._base = str;
	stream._ptr  = str;
	stream._cnt  = strlen(str);
	stream._flag = _IOSTRG | _IOBINARY | _IOREAD;

	return _doscan(&stream, fmt, args);
}
