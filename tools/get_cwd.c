#include <stdio.h>     // For printf, perror
#include <unistd.h>    // For getcwd
#include <limits.h>    // For PATH_MAX (optional, but convenient)


#define PATH_MAX 255
void print_pwd(void)
{
    char cwd[PATH_MAX];

    // Get the current working directory and store it in 'cwd'
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd() error");
    }
}