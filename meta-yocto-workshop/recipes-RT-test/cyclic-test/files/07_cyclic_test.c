#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>

#define ITERATIONS 100000
#define INTERVAL_US 1000  // 1ms interval

// Global for signal handler
volatile int keep_running = 1;

// Signal handler for clean termination
void signal_handler(int sig) {
    keep_running = 0;
}

void setup_rt_task() {
    struct sched_param param;
    
    // Use FIFO scheduling with high priority
    memset(&param, 0, sizeof(param));
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    
    if (sched_setscheduler(0, SCHED_FIFO, &param) < 0) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }
    
    // Lock memory to prevent page faults
    if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
        perror("mlockall");
        exit(EXIT_FAILURE);
    }
    
    // Set up signal handler for clean termination
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

int main() {
    struct timespec t_start, t_target, t_actual;
    long long latency, min_latency = 9999999, max_latency = 0, total_latency = 0;
    int i;
    
    // Initialize and set up the task for real-time execution
    setup_rt_task();
    
    printf("Starting latency test with %d iterations...\n", ITERATIONS);
    printf("Interval: %d microseconds\n", INTERVAL_US);
    
    // Pre-fault our stack to reduce latency spikes
    char dummy[8192];
    memset(dummy, 0, sizeof(dummy));
    
    // Initial sleep to let system settle
    usleep(10000);
    
    // Get initial time
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    
    for (i = 0; i < ITERATIONS && keep_running; i++) {
        // Calculate next target time
        t_target.tv_sec = t_start.tv_sec;
        t_target.tv_nsec = t_start.tv_nsec + (INTERVAL_US * 1000);
        if (t_target.tv_nsec >= 1000000000) {
            t_target.tv_nsec -= 1000000000;
            t_target.tv_sec++;
        }
        
        // Sleep until target time (absolute)
        if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_target, NULL) != 0) {
            perror("clock_nanosleep");
            break;
        }
        
        // Measure actual wake-up time
        clock_gettime(CLOCK_MONOTONIC, &t_actual);
        
        // Calculate latency (difference between target and actual)
        latency = ((t_actual.tv_sec - t_target.tv_sec) * 1000000000) + 
                  (t_actual.tv_nsec - t_target.tv_nsec);
        
        // Update statistics
        if (latency < min_latency) min_latency = latency;
        if (latency > max_latency) max_latency = latency;
        total_latency += latency;
        
        // Print progress for long runs
        if (i % 10000 == 0 && i > 0) {
            printf("Completed %d iterations, current max latency: %lld ns\n", 
                   i, max_latency);
        }
        
        // Use the actual time as the new start time
        t_start = t_actual;
    }
    
    // Print results
    printf("\nLatency Results (nanoseconds):\n");
    printf("Minimum: %lld ns\n", min_latency);
    printf("Maximum: %lld ns\n", max_latency);
    printf("Average: %lld ns\n", total_latency / i);
    
    // Release locked memory
    munlockall();
    
    return 0;
}