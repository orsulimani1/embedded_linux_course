#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/hwrng", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/urandom");
        return 1;
    }
    
    unsigned int random_value;
    ssize_t result = read(fd, &random_value, sizeof(random_value));
    if (result < 0) {
        perror("Failed to read from /dev/urandom");
        close(fd);
        return 1;
    }
    
    printf("Random number: %u\n", random_value);
    close(fd);
    return 0;
}