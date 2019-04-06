#include <stdarg.h>

char *__va_arg(char **ap, int size)
{
	register char *p = *ap;
	
	if (size == sizeof(char)) size = sizeof(int);
	*ap += size;

	return p;
}
