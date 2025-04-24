#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

int main() {
    // Get page size
    long page_size = sysconf(_SC_PAGESIZE);
    printf("Page size: %ld bytes\n", page_size);
    
    // Allocate a memory buffer aligned to page boundary
    void* buffer;
    if (posix_memalign(&buffer, page_size, page_size) != 0) {
        perror("posix_memalign failed");
        return 1;
    }
    
    printf("Allocated buffer at address: %p\n", buffer);
    
    // Initialize the buffer
    memset(buffer, 0, page_size);
    
    // Lock the buffer in memory
    printf("Locking buffer in memory...\n");
    if (mlock(buffer, page_size) != 0) {
        if (errno == EPERM) {
            printf("Permission denied. Try running with sudo or increase RLIMIT_MEMLOCK.\n");
        } else {
            perror("mlock failed");
        }
        free(buffer);
        return 1;
    }
    
    printf("Buffer successfully locked.\n");
    
    // Use the buffer for something
    char* char_buffer = (char*)buffer;
    sprintf(char_buffer, "This data will stay in RAM and not be swapped out.");
    printf("Buffer content: %s\n", char_buffer);
    
    // Sleep for a while to allow observation of the memory state
    printf("Sleeping for 10 seconds. You can check the process memory status.\n");
    printf("Run in another terminal: cat /proc/%d/maps | grep -A 2 %p\n", getpid(), buffer);
    sleep(10);
    
    // Unlock the memory
    printf("Unlocking the buffer...\n");
    if (munlock(buffer, page_size) != 0) {
        perror("munlock failed");
    }
    
    // Free the buffer
    free(buffer);
    
    return 0;
}