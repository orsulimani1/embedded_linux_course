/* ssize_t read(int fd, void *buf, size_t count);


char buffer[128];
ssize_t nbytes = read(fd, buffer, sizeof(buffer));
if (nbytes < 0) {
    perror("read");
    // handle error
} else if (nbytes == 0) {
    // EOF reached
} else {
    // Successfully read nbytes bytes into buffer
}

// Opening in nonblocking mode
int fd = open("myfifo", O_RDONLY | O_NONBLOCK);
if (fd < 0) {
    perror("open");
    // handle error
}

char buffer[128];
ssize_t nbytes = read(fd, buffer, sizeof(buffer));
if (nbytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available right now, try again later
    } else {
        perror("read");
        // handle other errors
    }
} else if (nbytes == 0) {
    // EOF or the other side closed
} else {
    // Successfully read nbytes bytes
}
 */