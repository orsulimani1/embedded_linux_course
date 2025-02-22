#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("example", O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Write initial data
    write(fd, "Hello", 5);

    // Move the file offset forward by 100 bytes from current position
    // off_t newPos = lseek(fd, 0, SEEK_CUR );
    off_t newPos = lseek(fd, 100, SEEK_CUR );

    if (newPos == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    // Write again
    write(fd, " World", 6);

    close(fd);
    return 0;
}
