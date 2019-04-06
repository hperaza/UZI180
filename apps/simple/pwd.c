#include <stdio.h>

char *getcwd(char *, int);

main()
{
    char buf[512];
    
    if (getcwd(buf, 512) == NULL) {
        fprintf(stderr, "pwd: cannot get current directory\n");
        return -1;
    }
    printf("%s\n", buf);
    return 0;
}
