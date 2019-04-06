#ifndef	_STDARG_H
#define	_STDARG_H

/*	Macros for accessing variable arguments */

typedef	char *	va_list;
extern	char *	__va_arg(char **, int);

#define	va_start(ap, parmn)	ap = (char *)&parmn + sizeof(parmn)

#define	va_arg(ap, type)	(type)(*__va_arg(&ap, sizeof(type)))

#define	va_end(ap)

#endif	_STDARG_H
