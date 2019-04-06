/*
 * vsprintf - print formatted output without ellipsis on an array
 * HiTechC version
 */

#include	<stdio.h>
#include	<stdarg.h>

extern int	_doprnt();

int
vsnprintf(wh, n, fmt, args)
char *	wh;
int	n;
char *	fmt;
va_list	args;
{
	static FILE spf;

	spf._file = -1;
	spf._flag = _IOWRT | _IOBINARY | _IOSTRG;
	spf._base = wh;
	spf._ptr  = wh;
	spf._cnt  = n-1;

	_doprnt(&spf, fmt, args);
	*spf._ptr = 0;

	return spf._ptr - wh;
}

int
vsprintf(wh, fmt, args)
char *	wh;
char *	fmt;
va_list	args;
{
	return vsnprintf(wh, 32767, fmt, args);
}
