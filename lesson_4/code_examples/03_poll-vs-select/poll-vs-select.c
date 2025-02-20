#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define TIMEOUT_SEC 5
#define BUF_SIZE 1024

static volatile sig_atomic_t keep_running = 1;

/* Handle CTRL+C to gracefully exit the loop */
void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

int main(void) {
    int fd_file;
    int max_fd, ret;
    char buf[BUF_SIZE + 1];
    int eof_reached = 0;

    /* Install a signal handler for CTRL+C so we can break out of the loop */
    signal(SIGINT, handle_sigint);

    /* Open (or create) a file in read-only mode */
    fd_file = open("example.txt", O_RDONLY | O_CREAT, 0666);
    if (fd_file < 0) {
        perror("open");
        return 1;
    }

    /* Calculate the max_fd for select() once */
    max_fd = (STDIN_FILENO > fd_file ? STDIN_FILENO : fd_file) + 1;

    printf("Monitoring stdin and 'example.txt' for readability...\n");
    printf("Will wait up to %d seconds each loop. Press CTRL+C to exit.\n", TIMEOUT_SEC);

    while (keep_running) {
        /* Prepare file descriptor set for each iteration */
        fd_set readfds;
        FD_ZERO(&readfds);

        /* Always monitor stdin */
        FD_SET(STDIN_FILENO, &readfds);

        /* Monitor the file unless we reached EOF previously */
        if (!eof_reached) {
            FD_SET(fd_file, &readfds);
        }

        /* Set up a 5-second timeout each time through the loop */
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SEC;
        tv.tv_usec = 0;

        printf("\n-- Calling select() again, waiting up to %d seconds...\n", TIMEOUT_SEC);
        fflush(stdout);

        /* Call select() */
        ret = select(max_fd, &readfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) {
                /* Interrupted by signal (likely CTRL+C); just exit the loop */
                perror("select interrupted by signal");
                break;
            } else {
                perror("select");
                break;
            }
        } 
        else if (ret == 0) {
            /* Timed out with no data */
            printf("No data arrived on stdin or 'example.txt' within %d seconds.\n", TIMEOUT_SEC);
            continue;
        }

        /* If stdin is readable, read and print the input */
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int len = read(STDIN_FILENO, buf, BUF_SIZE);
            if (len < 0) {
                perror("read from stdin");
            } else if (len == 0) {
                /* EOF on stdin (unlikely for a terminal, but possible if piped) */
                printf("[stdin]: EOF encountered.\n");
                /* We won't break here; you could decide to exit or keep going */
            } else {
                buf[len] = '\0';
                printf("[stdin]: Read %d bytes -> \"%s\"\n", len, buf);
            }
        }

        /* If file fd is readable, read and print the data */
        if (!eof_reached && FD_ISSET(fd_file, &readfds)) {
            int len = read(fd_file, buf, BUF_SIZE);
            if (len < 0) {
                perror("read from example.txt");
            } else if (len == 0) {
                /* EOF on the file */
                printf("[example.txt]: End of file reached.\n");
                eof_reached = 1;
                /* After EOF, select() won't see new data unless you append to the file
                   and possibly re-seek or close/re-open. */
            } else {
                buf[len] = '\0';
                printf("[example.txt]: Read %d bytes -> \"%s\"\n", len, buf);
            }
        }
    }

    close(fd_file);
    printf("Exiting...\n");
    return 0;
}
