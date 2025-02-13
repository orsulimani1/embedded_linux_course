#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // fork(), read(), write(), close()
#include <string.h>     // strlen()
#include <fcntl.h>      // open(), O_RDONLY, O_WRONLY, O_NONBLOCK
#include <sys/stat.h>   // mkfifo() (if you want to create FIFO in code)
#include <errno.h>      // errno, EAGAIN, EWOULDBLOCK

#define MAX_MSG_SIZE 256
#define MSG_STOP "stop"
#define MY_FIFO "myfifo"
// #define BLOCKING_MODE "block"
#define BLOCKING_MODE "nonblock"

int make_fifo(const char *);

/*
 *
 * Demonstrates how the parent's read() call and child's write() call behave
 * in blocking vs. nonblocking scenarios within a single forked process.
 */
int main(int argc, char *argv[])
{

    const char *mode_str = BLOCKING_MODE;
    const char *fifo_path = MY_FIFO;

    int flags_reader = O_RDONLY;
    int flags_writer = O_WRONLY;
    
    
    if (0 < make_fifo(fifo_path)){
        perror("could not make fifo");
        return EXIT_FAILURE;
    }
    // If user requests nonblocking mode, set O_NONBLOCK for both reader & writer
    if (strcmp(mode_str, "nonblock") == 0) {
        flags_reader |= O_NONBLOCK;
        flags_writer |= O_NONBLOCK;
    }

    // Fork into parent (reader) and child (writer)
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        //------------------------------------------------
        // CHILD: Writer
        //------------------------------------------------
        // Delay child slightly so the parent can open for reading
        // (especially in blocking mode, this ensures parent is ready)
        sleep(1);
        char buffer[MAX_MSG_SIZE];

        printf("Child (PID=%d) opening FIFO in %s write mode...\n",
               getpid(), mode_str);

        int wfd = open(fifo_path, flags_writer);
        if (wfd < 0) {
            perror("open (writer)");
            return EXIT_FAILURE;
        }
        while(1){

            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                perror("fgets (writer)");
                return EXIT_FAILURE;
            }
            // Remove newline from fgets
            buffer[strcspn(buffer, "\n")] = '\0';

            ssize_t bytes_written = write(wfd, buffer, strlen(buffer));
            if (bytes_written < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    printf("[Child Nonblocking] write() would block. No data written.\n");
                } else {
                    perror("write (child)");
                }
            } else {
                printf("Child wrote %zd bytes: '%s'\n", bytes_written, buffer);
            }
            // Check for exit condition
            if (strcmp(buffer, MSG_STOP) == 0) {
                printf("[child] Exiting.\n");
                break;
            }
            sleep(2);
        }

        close(wfd);
        return 0;

    } else {
        //------------------------------------------------
        // PARENT: Reader
        //------------------------------------------------
        printf("Parent (PID=%d) opening FIFO in %s read mode...\n",
               getpid(), mode_str);

        int rfd = open(fifo_path, flags_reader);
        if (rfd < 0) {
            perror("open (reader)");
            return EXIT_FAILURE;
        }
        while (1)
        {
            printf("Parent: attempting to read...\n");

            char buffer[MAX_MSG_SIZE];
            ssize_t bytes_read = read(rfd, buffer, sizeof(buffer) - 1);
            if (bytes_read < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    printf("[Parent Nonblocking] No data available yet.\n");
                } else {
                    perror("read (parent)");
                }
            } else if (bytes_read == 0) {
                // 0 indicates EOF (child closed FIFO)
                printf("Parent: Reached EOF, child closed the FIFO?\n");
            } else {
                buffer[bytes_read] = '\0';
                printf("Parent read %zd bytes: '%s'\n", bytes_read, buffer);
            }
            // Check for exit condition
            if (strcmp(buffer, MSG_STOP) == 0) {
                printf("[child] Exiting.\n");
                break;
            }
            sleep(2);
        }
        close(rfd);

        // Wait for child to finish
        wait(NULL);

                // Remove the FIFO
        if (unlink(fifo_path) < 0) {
            perror("unlink");
        } else {
            printf("Removed FIFO: %s\n", fifo_path);
        }
        return 0;
    }
}



int make_fifo(const char * fifo_path){
        // Check if FIFO exists; if not, create it.
    struct stat st;
    if (stat(fifo_path, &st) == -1) {
        // If the file doesn't exist or stat fails for other reasons
        if (mkfifo(fifo_path, 0666) < 0) {
            perror("mkfifo");
            return EXIT_FAILURE;
        }
        printf("Created FIFO: %s\n", fifo_path);
    } else if (!S_ISFIFO(st.st_mode)) {
        // If the file exists but is not a FIFO, that's an error
        fprintf(stderr, "Error: %s exists and is not a FIFO.\n", fifo_path);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}