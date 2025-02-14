#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define FILENAME "../../../lesson_3/assignments/assignments_cpy.txt"
int get_file_permissions(const char *path);

// int open(const char *pathname, int flags, mode_t mode);
// fd = creat(filename, 0644);

int main(int argc, char const *argv[])
{
    int fd;
    fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC,
            S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP | S_IROTH);
    if (fd == -1) {
        /* handle error */
        perror("open()");
    }
    printf("Opened file for ");
    get_file_permissions(FILENAME);
    close(fd);

    fd = creat(FILENAME, 0644);
    if (fd == -1) {
        /* handle error */
        perror("creat()");
    }
    return 0;
}


/* int creat(const char *name, int mode)
{
    return open(name, O_WRONLY | O_CREAT | O_TRUNC, mode);
} */

int get_file_permissions(const char *path)
{
    struct stat file_stat;

    // Use stat() to retrieve file information
    if (stat(path, &file_stat) == -1) {
        perror("stat");
        return -1;
    }
    // The mode bits for user, group, others bits
    printf("Permissions for '%s': %o (octal)\n", path, file_stat.st_mode & 07777);

    return 0;
}