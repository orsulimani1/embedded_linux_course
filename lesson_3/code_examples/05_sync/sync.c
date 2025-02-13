#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // For sync()

int main(void)
{
    // 1) Initiate system-wide writeback of all dirty buffers
    sync();

    // 2) No return value to check—by POSIX definition, it just initiates the sync.
    // On Linux, it waits until the buffers are flushed (synchronous).

    printf("sync() called. All dirty buffers system-wide are being flushed.\n");
    return EXIT_SUCCESS;
}
