#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIMEOUT 2      /* select timeout in seconds */
#define BUF_LEN 1024   /* read buffer in bytes */

int main(void)
{
    struct timeval tv;
    fd_set readfds;
    int ret;

    /* Initialize the FD set to all zero. */
    FD_ZERO(&readfds);

    /* Add our file descriptor (stdin) to the read set. */
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(STDERR_FILENO, &readfds);
    FD_CLR(STDERR_FILENO, &readfds);
    /* Configure the timeout. */
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 500000;

    /*
     * select() will monitor the descriptors up to STDIN_FILENO + 1
     * (the first parameter is highest_fd + 1).
     * We only care about readfds (since we want to read from stdin),
     * so we pass NULL for writefds and exceptfds.
     */
    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    if (ret == -1) {
        perror("select");
        return 1;
    } else if (!ret) {
        /* Timeout expired. */
        printf("%d seconds elapsed with no input.\n", TIMEOUT);
        return 0;
    }

    /*
     * If ret > 0, some file descriptor(s) is/are ready.
     * Let's check if stdin is in that set.
     */
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
        char buf[BUF_LEN + 1];
        int len;

        /* It's ready, so we can read without blocking. */
        len = read(STDIN_FILENO, buf, BUF_LEN);
        if (len == -1) {
            perror("read");
            return 1;
        }
        if (len) {
            buf[len] = '\0';  /* Null-terminate for printing. */
            printf("read: %s\n", buf);
        }
        return 0;
    }

    fprintf(stderr, "This should not happen!\n");
    return 1;
}
