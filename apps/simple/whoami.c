#include <stdio.h>
#include <pwd.h>

extern int geteuid();

int main(void)
{
    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    if (pw) {
	printf("%s\n", pw->pw_name);
	return 0;
    }

    return 1;
}
