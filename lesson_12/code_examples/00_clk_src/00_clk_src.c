#include <stdio.h>
#include <time.h>

int main() {
    struct timespec res;
    
    // Check CLOCK_REALTIME resolution
    if (clock_getres(CLOCK_REALTIME, &res) == -1) {
        perror("clock_getres");
        return 1;
    }
    printf("CLOCK_REALTIME resolution: %ld seconds, %ld nanoseconds\n", 
           res.tv_sec, res.tv_nsec);
    
    // Check CLOCK_MONOTONIC resolution
    if (clock_getres(CLOCK_MONOTONIC, &res) == -1) {
        perror("clock_getres");
        return 1;
    }
    printf("CLOCK_MONOTONIC resolution: %ld seconds, %ld nanoseconds\n", 
           res.tv_sec, res.tv_nsec);
    
    // Check CLOCK_PROCESS_CPUTIME_ID resolution
    if (clock_getres(CLOCK_PROCESS_CPUTIME_ID, &res) == -1) {
        perror("clock_getres");
        return 1;
    }
    printf("CLOCK_PROCESS_CPUTIME_ID resolution: %ld seconds, %ld nanoseconds\n", 
           res.tv_sec, res.tv_nsec);
    
    return 0;
}