#define _GNU_SOURCE  /* <--- Make sure this is at the very top! */
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

#define TIMEOUT 2 /* Timeout in seconds */
#define USE_PPOLL
/* If you compile with -DUSE_PPOLL, we'll redefine poll(...) to call ppoll() internally. */
#ifdef USE_PPOLL
static int my_ppoll_wrapper(struct pollfd *fds, nfds_t nfds, int timeout_ms)
{
    /* Convert milliseconds to a struct timespec for ppoll(). */
    struct timespec ts;
    ts.tv_sec  = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000000L;

    /* Create a sigset_t to block SIGINT (Ctrl+C) while in ppoll(). */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    return ppoll(fds, nfds, &ts, &mask);
}

/* Replace calls to poll(...) with our ppoll wrapper */
#define poll(a, b, c) my_ppoll_wrapper((a), (b), (c))
#endif /* USE_PPOLL */

int main(void)
{
    struct pollfd fds[2];

    /* Set up poll structures */
    fds[0].fd     = STDIN_FILENO;   /* Watch stdin for input */
    fds[0].events = POLLIN;

    // fds[1].fd     = STDOUT_FILENO;  /* Watch stdout for ability to write */
    // fds[1].events = POLLOUT;

    while (1) {
        usleep(100000);
        int ret = poll(fds, 2, TIMEOUT * 1000);
        if (ret < 0) {
#ifdef USE_PPOLL
            perror("ppoll");
#else
            perror("poll");
#endif
            return 1;
        }

        if (ret == 0) {
            /* Timed out */
            printf("%d seconds elapsed with no input.\n", TIMEOUT);
            continue;
        }

        /* If stdin is readable, read and print the input */
        if (fds[0].revents & POLLIN) {
            char buf[1024];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("Received input: %s\n", buf);
            } else if (n == 0) {
                printf("EOF on stdin.\n");
                break; /* Stop if stdin is closed */
            } else {
                perror("read");
                break;
            }
        }

        /* If stdout is writable (usually true), you could do something here */
        if (fds[1].revents & POLLOUT) {
            printf("stdout is writable\n");
        }
    }

    return 0;
}
