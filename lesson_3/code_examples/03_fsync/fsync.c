#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // For close(), fsync()
#include <fcntl.h>      // For open(), O_WRONLY, O_CREAT, O_TRUNC
#include <string.h>     // For strlen()

int main(void)
{
    // 1) Open a file for writing
    int fd = open("fsync_example.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    // 2) Write some data
    const char *data = "Hello from fsync()!\n";
    ssize_t written = write(fd, data, strlen(data));
    if (written < 0) {
        perror("write");
        close(fd);
        return EXIT_FAILURE;
    }

    // 3) Force synchronization to disk
    if (fsync(fd) < 0) {
        perror("fsync");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Data has been (attempted) synced to disk via fsync().\n");

    // 4) Cleanup
    close(fd);
    return EXIT_SUCCESS;
}
