#include <stdio.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void strip_newline(char *str);

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if(pid==0){
        //------------------------------------------------
        // CHILD: writer
        //------------------------------------------------
        int fd = open("data.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            return 1;
        }
    
        char header[] = "HEAD1234\n";
        char body[]   = "This is the main content.\n";
        char footer[] = "FOOTER99\n";
    
        struct iovec iov[3];
        iov[0].iov_base = header; iov[0].iov_len = sizeof(header) - 1;
        iov[1].iov_base = body;   iov[1].iov_len = sizeof(body) - 1;
        iov[2].iov_base = footer; iov[2].iov_len = sizeof(footer) - 1;
    
        ssize_t bytes_written = writev(fd, iov, 3);
        if (bytes_written < 0) {
            perror("writev");
            close(fd);
            return 1;
        }
    
        printf("Wrote %zd bytes to file.\n", bytes_written);
    
        close(fd);
    } else {
        // Wait for child to finish
        wait(NULL);
        int fd = open("data.bin", O_RDONLY);
        if (fd < 0) {
            perror("open");
            return 1;
        }
    
        char header[9], body[26], footer[8];
        struct iovec iov[3];
    
        iov[0].iov_base = header; iov[0].iov_len = sizeof(header);
        iov[1].iov_base = body;   iov[1].iov_len = sizeof(body);
        iov[2].iov_base = footer; iov[2].iov_len = sizeof(footer);
    
        ssize_t bytes_read = readv(fd, iov, 3);
        if (bytes_read < 0) {
            perror("readv");
            close(fd);
            return 1;
        }
        strip_newline(header);
        strip_newline(body);
        strip_newline(footer);
        printf("Read %zd bytes: [%.*s] [%.*s] [%.*s]\n",
               bytes_read, (int)strlen(header), header, 
               (int)strlen(body), body, 
               (int)strlen(footer), footer);
    
        close(fd);
    }

    return 0;
}

// remove endlines
void strip_newline(char *str) {
    if (!str) return;  // handle NULL pointer safely

    while (*str != '\0') {
        if (*str == '\n') {
            *str = '\0';
            break;
        }
        str++;
    }
}
