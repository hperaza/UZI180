char *sys_err[] =
{
	"Unknown error",		/*  0 */
	"Not owner",
	"No such file or directory",
	"No such process",
	"Interrupted System Call",
	"I/O Error",			/*  5 */
	"No such device or address",
	"Arg list too long",
	"Exec format error",
	"Bad file number",
	"No children",			/* 10 */
	"No more processes",
	"Not enough core",
	"Permission denied",
	"Bad address",
	"Block device required",	/* 15 */
	"Mount device busy",
	"File exists",
	"Cross-device link",
	"No such device",
	"Not a directory",		/* 20 */
	"Is a directory",
	"Invalid argument",
	"File table overflow",
	"Too many open files",
	"Not a typewriter",		/* 25 */
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",	/* 30 */
	"Too many links",
	"Broken pipe",
	"Argument too large",				/*280*/
	"Result too large"				/*280*/
};

short sys_ner = sizeof sys_err / sizeof sys_err[0];
