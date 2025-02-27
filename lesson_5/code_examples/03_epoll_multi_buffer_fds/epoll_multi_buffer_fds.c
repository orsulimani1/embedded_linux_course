/*****************************************************************************
 * demo_writer_epoll_after_prompt.c
 *
 * - Writer (child):
 *      * In each iteration:
 *          1) Prompt user
 *          2) Read input
 *          3) Write timestamp + message to fifo1 & fifo2 (using writev)
 *          4) epoll_wait for response on fifo1_resp & fifo2_resp
 *          5) read response (using readv) and print it
 *      * Repeat until "exit"
 *
 * - Receiver (parent):
 *      * Monitors fifo1 & fifo2 with epoll
 *      * readv two iovec buffers: timestamp, message
 *      * writes them to "received.log"
 *      * echoes them back to the Writer on fifo1_resp or fifo2_resp
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

/* Named FIFOs */
#define FIFO1       "fifo1"
#define FIFO2       "fifo2"
#define FIFO1_RESP  "fifo1_resp"
#define FIFO2_RESP  "fifo2_resp"

/* For epoll */
#define MAX_EVENTS  2
#define BUF_SIZE    256
 
/**
 * create_fifos - Creates four named FIFOs for your demo, ignoring EEXIST errors.
 *                Returns 0 on success, -1 on failure.
 */
int create_fifos(void) {
    // Desired file modes for the FIFOs, e.g. 0666 => rw-rw-rw-
    mode_t mode = 0666;

    if (mkfifo("fifo1", mode) < 0 && errno != EEXIST) {
        fprintf(stderr, "mkfifo(\"fifo1\"): %s\n", strerror(errno));
        return -1;
    }
    if (mkfifo("fifo2", mode) < 0 && errno != EEXIST) {
        fprintf(stderr, "mkfifo(\"fifo2\"): %s\n", strerror(errno));
        return -1;
    }
    if (mkfifo("fifo1_resp", mode) < 0 && errno != EEXIST) {
        fprintf(stderr, "mkfifo(\"fifo1_resp\"): %s\n", strerror(errno));
        return -1;
    }
    if (mkfifo("fifo2_resp", mode) < 0 && errno != EEXIST) {
        fprintf(stderr, "mkfifo(\"fifo2_resp\"): %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

 /* Simple function to get a "[HH:MM:SS]" timestamp */
 static void get_timestamp(char *buf, size_t buflen) {
     time_t now = time(NULL);
     struct tm tmnow;
     localtime_r(&now, &tmnow);
     strftime(buf, buflen, "[%H:%M:%S]", &tmnow);
 }
 
int main(void)
{
    create_fifos();
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    /*************************************************************************
     * CHILD: Writer
     *************************************************************************/
    if (pid == 0) {
        // 1) Open the output FIFOs for writing (to send data)
        int fd1 = open(FIFO1, O_WRONLY);
        int fd2 = open(FIFO2, O_WRONLY);
        if (fd1 < 0 || fd2 < 0) {
            perror("[Writer] open(FIFO1/FIFO2)");
            return EXIT_FAILURE;
        }

        // 2) Open the response FIFOs for reading in non-blocking mode
        int fd1_resp = open(FIFO1_RESP, O_RDONLY | O_NONBLOCK);
        int fd2_resp = open(FIFO2_RESP, O_RDONLY | O_NONBLOCK);
        if (fd1_resp < 0 || fd2_resp < 0) {
            perror("[Writer] open(FIFOx_RESP)");
            return EXIT_FAILURE;
        }

        // 3) Create epoll to watch fd1_resp, fd2_resp
        int epfd = epoll_create1(0);
        if (epfd < 0) {
            perror("[Writer] epoll_create1");
            return EXIT_FAILURE;
        }

        // Add fd1_resp to epoll
        struct epoll_event ev1, ev2;
        memset(&ev1, 0, sizeof(ev1));
        ev1.events = EPOLLIN;
        ev1.data.fd = fd1_resp;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd1_resp, &ev1) < 0) {
            perror("[Writer] epoll_ctl fd1_resp");
            return EXIT_FAILURE;
        }

        // Add fd2_resp to epoll
        memset(&ev2, 0, sizeof(ev2));
        ev2.events = EPOLLIN;
        ev2.data.fd = fd2_resp;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd2_resp, &ev2) < 0) {
            perror("[Writer] epoll_ctl fd2_resp");
            return EXIT_FAILURE;
        }

        printf("[Writer] Type your message (or 'exit' to quit)\n");

        while (1) {
            // 4) Prompt user
            printf("[Writer] > ");
            fflush(stdout);

            // 5) Read user input
            char user_line[BUF_SIZE];
            if (!fgets_unlocked(user_line, sizeof(user_line), stdin)) {
                // EOF or error
                break;
            }
            // remove newline
            user_line[strcspn(user_line, "\n")] = '\0';

            if (strcmp(user_line, "exit") == 0) {
                printf("[Writer] Exiting...\n");
                break;
            }

            // 6) Build iov[2]: iov[0] = timestamp, iov[1] = user message
            char ts[32];
            get_timestamp(ts, sizeof(ts));

            struct iovec iov_msg[2];
            iov_msg[0].iov_base = ts;
            iov_msg[0].iov_len  = strlen(ts);
            iov_msg[1].iov_base = user_line;
            iov_msg[1].iov_len  = strlen(user_line);

            // 7) writev() to both FIFO1 & FIFO2
            ssize_t w1 = writev(fd1, iov_msg, 2);
            ssize_t w2 = writev(fd2, iov_msg, 2);
            if (w1 < 0 || w2 < 0) {
                perror("[Writer] writev");
                continue;
            }
            printf("[Writer] Sent timestamp+msg to FIFO1 & FIFO2.\n");

            // 8) Now epoll_wait for a response from either fd1_resp or fd2_resp
            //    We'll do a single epoll_wait. If you want to read from both FDs,
            //    you might do multiple. This code waits for exactly one event, then
            //    reads all responses that are available.
            struct epoll_event events[MAX_EVENTS];
            int ready = epoll_wait(epfd, events, MAX_EVENTS, 5000); // 5 sec timeout
            if (ready < 0 && errno != EINTR) {
                perror("[Writer] epoll_wait");
                break;
            }
            if (ready == 0) {
                printf("[Writer] No response within 5 seconds.\n");
                continue;
            }

            // read from each FD that has EPOLLIN
            for (int i = 0; i < ready; i++) {
                if (events[i].events & EPOLLIN) {
                    int rfd = events[i].data.fd;

                    // We'll do readv with iov[2]
                    char ts_buf[32], msg_buf[BUF_SIZE];
                    struct iovec iov_rd[2];
                    iov_rd[0].iov_base = ts_buf;
                    iov_rd[0].iov_len  = sizeof(ts_buf) - 1;
                    iov_rd[1].iov_base = msg_buf;
                    iov_rd[1].iov_len  = sizeof(msg_buf) - 1;

                    ssize_t n = readv(rfd, iov_rd, 2);
                    if (n > 0) {
                        ts_buf[sizeof(ts_buf) - 1] = '\0';
                        msg_buf[sizeof(msg_buf) - 1] = '\0';
                        printf("[Writer] Got response on fd=%d:\n  Timestamp: '%s'\n  Message:   '%s'\n",
                            rfd, ts_buf, msg_buf);
                    }
                    else if (n == 0) {
                        printf("[Writer] EOF on fd=%d\n", rfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, rfd, NULL);
                        close(rfd);
                    }
                    else if (n < 0 && errno != EAGAIN) {
                        perror("[Writer] readv");
                    }
                }
            }
        }

        close(fd1);
        close(fd2);
        close(fd1_resp);
        close(fd2_resp);
        close(epfd);
        return 0;
    }

    /*************************************************************************
     * PARENT: Receiver
     *************************************************************************/
    else {
        // Optionally wait 1 second so child opens all FIFOs first
        sleep(1);

        // Open input FIFOs (fifo1/fifo2) for reading (non-block)
        int fd1 = open(FIFO1, O_RDONLY | O_NONBLOCK);
        int fd2 = open(FIFO2, O_RDONLY | O_NONBLOCK);
        if (fd1 < 0 || fd2 < 0) {
            perror("[Receiver] open FIFO1/FIFO2");
            return EXIT_FAILURE;
        }

        // Open response FIFOs for writing
        int fd1_resp = open(FIFO1_RESP, O_WRONLY);
        int fd2_resp = open(FIFO2_RESP, O_WRONLY);
        if (fd1_resp < 0 || fd2_resp < 0) {
            perror("[Receiver] open FIFOx_RESP");
            return EXIT_FAILURE;
        }

        // Open a local file to store all messages
        FILE *fp_log = fopen("received.log", "a");
        if (!fp_log) {
            perror("[Receiver] fopen received.log");
            return EXIT_FAILURE;
        }

        // epoll for fd1, fd2
        int epfd = epoll_create1(0);
        if (epfd < 0) {
            perror("[Receiver] epoll_create1");
            return EXIT_FAILURE;
        }

        struct epoll_event ev1, ev2;
        memset(&ev1, 0, sizeof(ev1));
        ev1.events = EPOLLIN;
        ev1.data.fd = fd1;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd1, &ev1);

        memset(&ev2, 0, sizeof(ev2));
        ev2.events = EPOLLIN;
        ev2.data.fd = fd2;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd2, &ev2);

        printf("[Receiver] Monitoring FIFO1 & FIFO2 via epoll.\n");

        // Main loop
        while (1) {
            struct epoll_event events[MAX_EVENTS];
            int ready = epoll_wait(epfd, events, MAX_EVENTS, -1);
            if (ready < 0 && errno != EINTR) {
                perror("[Receiver] epoll_wait");
                break;
            }

            for (int i = 0; i < ready; i++) {
                if (events[i].events & EPOLLIN) {
                    int rfd = events[i].data.fd;
                    // read iovec[2]: ts_buf + msg_buf
                    char ts_buf[32], msg_buf[BUF_SIZE];
                    struct iovec iov_rd[2];
                    iov_rd[0].iov_base = ts_buf;
                    iov_rd[0].iov_len  = sizeof(ts_buf) - 1;
                    iov_rd[1].iov_base = msg_buf;
                    iov_rd[1].iov_len  = sizeof(msg_buf) - 1;

                    ssize_t n = readv(rfd, iov_rd, 2);
                    if (n > 0) {
                        ts_buf[sizeof(ts_buf) - 1] = '\0';
                        msg_buf[sizeof(msg_buf) - 1] = '\0';

                        printf("[Receiver] Received from fd=%d:\n  Timestamp:'%s'\n  Message:'%s'\n",
                            rfd, ts_buf, msg_buf);

                        // Store in local file
                        fprintf(fp_log, "%s %s\n", ts_buf, msg_buf);
                        fflush(fp_log);

                        // Echo back using writev()
                        struct iovec iov_wr[2];
                        iov_wr[0].iov_base = ts_buf;
                        iov_wr[0].iov_len  = strlen(ts_buf);
                        iov_wr[1].iov_base = msg_buf;
                        iov_wr[1].iov_len  = strlen(msg_buf);

                        if (rfd == fd1) {
                            writev(fd1_resp, iov_wr, 2);
                        } else {
                            writev(fd2_resp, iov_wr, 2);
                        }
                    }
                    else if (n == 0) {
                        // Possibly writer closed the FIFO
                        printf("[Receiver] EOF on fd=%d\n", rfd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, rfd, NULL);
                        close(rfd);
                    }
                    else if (n < 0 && errno != EAGAIN) {
                        perror("[Receiver] readv");
                    }
                }
            }
        }

        fclose(fp_log);
        close(fd1);
        close(fd2);
        close(fd1_resp);
        close(fd2_resp);
        close(epfd);

        // Wait for child (Writer) to exit
        wait(NULL);
        printf("[Receiver] Exiting.\n");
    }

    return 0;
}
 