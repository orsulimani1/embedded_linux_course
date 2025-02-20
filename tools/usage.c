#include <sys/types.h>
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence);

lseek(fd, 0, SEEK_SET);   // Go to start
lseek(fd, 10, SEEK_CUR);  // Move 10 bytes ahead from current
lseek(fd, -5, SEEK_END);  // Move 5 bytes before the end


#include <unistd.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

#include <unistd.h>
#include <sys/types.h>
int ftruncate (int fd, off_t len);

#include <unistd.h>
#include <sys/types.h>
int truncate (const char *path, off_t len);

int main()
{
    int ret;
    ret = truncate ("./pirate.txt", 45);
    if (ret == -1) {
        perror ("truncate");
        return -1;
    }
    return 0;
}
results in a file of length 45 bytes with the following contents:


#include <sys/select.h>

int select(int nfds,
           fd_set *readfds,
           fd_set *writefds,
           fd_set *exceptfds,
           struct timeval *timeout);

struct timeval tv;
tv.tv_sec = 1;
tv.tv_usec = 0;
select(0, NULL, NULL, NULL, &tv); // Sleep for 1 second

#include <sys/select.h>
#include <signal.h>

int pselect(int nfds,
            fd_set *readfds,
            fd_set *writefds,
            fd_set *exceptfds,
            const struct timespec *timeout,
            const sigset_t *sigmask);



#include <poll.h>

struct pollfd {
    int   fd;         // The file descriptor to monitor
    short events;     // The requested events (input flags)
    short revents;    // The events that actually occurred (output flags)
};
int poll(struct pollfd *fds, nfds_t nfds, int timeout);


int ppoll(struct pollfd *fds, nfds_t nfds,
    const struct timespec *timeout_ts,
    const sigset_t *sigmask);
