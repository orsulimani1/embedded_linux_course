#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

int main() {
    struct timespec req, rem;
    
    // Request 10.5 milliseconds sleep
    req.tv_sec = 0;
    req.tv_nsec = 10500000;  // 10.5 million nanoseconds
    
    printf("Sleeping for 10.5 ms...\n");
    
    // Call nanosleep, handling interruptions
    while (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("Sleep interrupted! Continuing for %ld.%09ld seconds\n", 
                   rem.tv_sec, rem.tv_nsec);
            req = rem;  // Continue with remaining time
        } else {
            perror("nanosleep");
            exit(EXIT_FAILURE);
        }
    }
    
    printf("Sleep completed\n");
    
    return 0;
}