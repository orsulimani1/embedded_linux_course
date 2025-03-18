#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main() {
    // Open a file for reading
    int fd = open("data.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Prepare the aio control block
    struct aiocb cb;
    memset(&cb, 0, sizeof(cb));

    // Allocate a buffer for reading
    cb.aio_fildes = fd;
    cb.aio_buf = malloc(1024);
    cb.aio_nbytes = 1024;
    cb.aio_offset = 0; // Start reading at the beginning

    // Initiate asynchronous read
    if (aio_read(&cb) == -1) {
        perror("aio_read");
        close(fd);
        return 1;
    }

    // Do other work while the read is in progress...
    printf("Reading data asynchronously... Doing other tasks.\n");

    // Busy-wait loop (for demonstration only). 
    // In production, use signals or other event mechanisms to be notified.
    while (aio_error(&cb) == EINPROGRESS) {
        // We could do more useful work here
    }

    // Check the final status
    int status = aio_return(&cb);
    if (status == -1) {
        perror("aio_return");
    } else {
        printf("Asynchronous read finished, bytes read: %d\n", status);
        // Optionally process the data in cb.aio_buf here
    }

    // Cleanup
    free((void *)cb.aio_buf);
    close(fd);

    return 0;
}
