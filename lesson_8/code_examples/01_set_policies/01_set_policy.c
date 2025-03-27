#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>

int main() {
    int policy = SCHED_FIFO;  // Real-time FIFO scheduling
    struct sched_param param;
    
    // Get valid priority range
    int max_prio = sched_get_priority_max(policy);
    int min_prio = sched_get_priority_min(policy);
    
    if (max_prio == -1 || min_prio == -1) {
        perror("sched_get_priority_max/min");
        exit(EXIT_FAILURE);
    }
    
    printf("Priority range for SCHED_FIFO: %d - %d\n", min_prio, max_prio);
    
    // Set to medium-high priority
    param.sched_priority = min_prio + (max_prio - min_prio) * 3/4;
    
    // Set the scheduling policy
    if (sched_setscheduler(0, policy, &param) == -1) {
        fprintf(stderr, "Failed to set scheduler: %s\n", strerror(errno));
        if (errno == EPERM)
            fprintf(stderr, "Permission denied. Try running with sudo.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Successfully set SCHED_FIFO with priority %d\n", param.sched_priority);
    
    // Run for a short while to demonstrate
    sleep(10);
    
    return 0;
}