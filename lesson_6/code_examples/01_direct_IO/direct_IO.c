#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main() {
    const char *filename = "testfile.bin";
    // Open the file with O_DIRECT
    int fd = open(filename, O_RDWR | O_DIRECT);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Alignment requirement (commonly 512 or 4096)
    size_t alignment = 4096;
    size_t blockSize = 4096;

    void *buffer;
    if (posix_memalign(&buffer, alignment, blockSize) != 0) {
        perror("posix_memalign");
        close(fd);
        return 1;
    }

    // Zero out the buffer, write some data
    memset(buffer, 0, blockSize);
    strcpy((char *)buffer, "Hello Direct I/O!");

    // Perform a write
    ssize_t written = write(fd, buffer, blockSize);
    if (written < 0) {
        perror("write");
    } else {
        printf("Wrote %zd bytes via direct I/O.\n", written);
    }

    // Reset file offset and read back
    lseek(fd, 0, SEEK_SET);
    memset(buffer, 0, blockSize);
    ssize_t readBytes = read(fd, buffer, blockSize);
    if (readBytes < 0) {
        perror("read");
    } else {
        printf("Read %zd bytes: \"%s\"\n", readBytes, (char *)buffer);
    }

    free(buffer);
    close(fd);
    return 0;
}
