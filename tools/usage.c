#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

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


setvbuf(stream, buffer, _IOFBF, custom_size);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

#include <stdio.h>
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *stream);
FILE * fdopen (int fd, const char *mode);


// Reading
int fgetc(FILE *stream);
char *fgets(char *str, int num, FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fscanf (FILE * stream,const char *__restrict __format, ...);


// Writing
int fputc(int c, FILE *stream);
int fputs(const char *str, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fprintf(FILE *stream, const char *format, ...);

int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
int fflush(FILE *stream);

int fcloseall (void);

void flockfile(FILE *stream);
int ftrylockfile(FILE *stream);
void funlockfile(FILE *stream);

int fgetc_unlocked(FILE *stream);
char *fgets_unlocked(char *str, int size, FILE *stream);
size_t fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fputc_unlocked(int c, FILE *stream);
int fputs_unlocked(const char *str, FILE *stream);
size_t fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *stream);


#include <sys/uio.h>
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);

struct iovec {
    void  *iov_base; // Pointer to buffer
    size_t iov_len;  // Length of buffer
};
ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset);
ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset);

#include <sys/epoll.h>
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
struct epoll_event
#include <sys/eventfd.h>
int efd = eventfd(unsigned int initval, int flags);

#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
#include <sys/mman.h>
int msync(void *addr, size_t length, int flags);
int madvise(void *addr, size_t length, int advice);


typedef struct image_header {
    uint32_t    ih_magic;       /* Image Header Magic Number (0x27051956) */
    uint32_t    ih_hcrc;        /* Image Header CRC Checksum */
    uint32_t    ih_time;        /* Image Creation Timestamp */
    uint32_t    ih_size;        /* Image Data Size */
    uint32_t    ih_load;        /* Data Load Address */
    uint32_t    ih_ep;          /* Entry Point Address */
    uint32_t    ih_dcrc;        /* Image Data CRC Checksum */
    uint8_t     ih_os;          /* Operating System */
    uint8_t     ih_arch;        /* CPU Architecture */
    uint8_t     ih_type;        /* Image Type */
    uint8_t     ih_comp;        /* Compression Type */
    uint8_t     ih_name[IH_NMLEN]; /* Image Name (32 bytes) */
} image_header_t;

#include <aio.h>


struct aiocb {
    int aio_fildes;            /* file descriptor */
    int aio_lio_opcode;        /* operation to perform (e.g., LIO_READ or LIO_WRITE) */
    int aio_reqprio;           /* request priority offset */
    volatile void *aio_buf;    /* pointer to data buffer */
    size_t aio_nbytes;         /* length of the operation */
    struct sigevent aio_sigevent; /* signal/event notification */
    /* internal, private members follow... */
};
int aio_read(struct aiocb *aiocbp);
int aio_write(struct aiocb *aiocbp);
int aio_error(const struct aiocb *aiocbp);
int aio_return(struct aiocb *aiocbp);


int aio_cancel(int fd, struct aiocb *aiocbp);
int aio_fsync(int op, struct aiocb *aiocbp);
int aio_suspend(const struct aiocb * const cblist[],
                int n,
                const struct timespec *timeout);

int execl(const char *path, const char *arg, ..., NULL);
int execv(const char *path, char *const argv[]);
int execle(const char *path, const char *arg, ..., NULL, char *const envp[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int execlp(const char *file, const char *arg, ..., NULL);
int execvp(const char *file, char *const argv[]);


pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

#include <sched.h>
#include <errno.h>
#include <string.h>
// struct sched_attr {
//     uint32_t size;              /* Size of this structure */
//     uint32_t sched_policy;      /* Policy (SCHED_DEADLINE) */
//     uint64_t sched_flags;       /* Flags */
//     int32_t  sched_nice;        /* Nice value (unused) */
//     uint32_t sched_priority;    /* Static priority (unused) */
//     uint64_t sched_runtime;     /* Runtime in nanoseconds */
//     uint64_t sched_deadline;    /* Deadline in nanoseconds */
//     uint64_t sched_period;      /* Period in nanoseconds */
// };

int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags);

int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags);

int sched_getscheduler(pid_t pid);

int sched_getparam(pid_t pid, struct sched_param *param);

int sched_get_priority_min(int policy);
int sched_get_priority_max(int policy);

int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);


int sched_setparam(pid_t pid, const struct sched_param *param);

#include <sys/time.h>
#include <sys/resource.h>

int getpriority(int which, id_t who);

int setpriority(int which, id_t who, int value);

int nice(int inc);

int sched_yield(void);
int nanosleep(const struct timespec *req, struct timespec *rem);
int clock_nanosleep(clockid_t clock_id, int flags,
    const struct timespec *request, struct timespec *remain);

    /* Yield the processor */
int sched_yield(void);

int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask);

int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);

void CPU_ZERO(cpu_set_t *set);      /* Clear CPU set */
void CPU_SET(int cpu, cpu_set_t *set);    /* Add CPU to set */
void CPU_CLR(int cpu, cpu_set_t *set);    /* Remove CPU from set */
int CPU_ISSET(int cpu, const cpu_set_t *set); /* Test if CPU in set */