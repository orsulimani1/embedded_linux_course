#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int policy;
    struct sched_param param;
    pid_t pid;
    
    // Use process ID from command line or current process
    if (argc > 1) {
        pid = atoi(argv[1]);
    } else {
        pid = getpid();
        printf("Using current process ID: %d\n", pid);
    }
    
    // Get process scheduling parameters
    if (sched_getparam(pid, &param) == -1) {
        fprintf(stderr, "Failed to get scheduling parameters for PID %d: %s\n", 
                pid, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Get process scheduling policy
    if ((policy = sched_getscheduler(pid)) == -1) {
        fprintf(stderr, "Failed to get scheduling policy for PID %d: %s\n", 
                pid, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    // Display policy name and priority
    printf("Process %d scheduling policy: ", pid);
    switch (policy) {
        case SCHED_FIFO:  printf("SCHED_FIFO (real-time, first-in-first-out)\n"); break;
        case SCHED_RR:    printf("SCHED_RR (real-time, round-robin)\n"); break;
        case SCHED_OTHER: printf("SCHED_OTHER (normal)\n"); break;
        case SCHED_BATCH: printf("SCHED_BATCH (batch)\n"); break;
        case SCHED_IDLE:  printf("SCHED_IDLE (idle)\n"); break;
#ifdef SCHED_DEADLINE
        case SCHED_DEADLINE: printf("SCHED_DEADLINE (real-time, deadline)\n"); break;
#endif
        default:          printf("Unknown (%d)\n", policy);
    }
    
    printf("Process %d priority: %d\n", pid, param.sched_priority);
    
    // Print valid priority range for this policy
    if (policy == SCHED_FIFO || policy == SCHED_RR) {
        int min = sched_get_priority_min(policy);
        int max = sched_get_priority_max(policy);
        if (min != -1 && max != -1) {
            printf("Valid priority range for this policy: %d to %d\n", min, max);
        }
    }
    
    return 0;
}