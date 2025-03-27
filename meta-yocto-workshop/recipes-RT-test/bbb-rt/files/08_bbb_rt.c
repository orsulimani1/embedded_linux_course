#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>

#define NANO_SECOND 1000000000

void check_rt_kernel() {
    FILE *fp;
    char config_line[256];
    int rt_kernel = 0;
    
    fp = fopen("/proc/version", "r");
    if (fp != NULL) {
        if (fgets(config_line, sizeof(config_line), fp) != NULL) {
            printf("Kernel version: %s", config_line);
            if (strstr(config_line, "PREEMPT_RT") != NULL) {
                printf("✓ PREEMPT_RT kernel detected\n");
                rt_kernel = 1;
            }
        }
        fclose(fp);
    }
    
    if (!rt_kernel) {
        printf("✗ Not running on a PREEMPT_RT kernel\n");
    }
    
    // Check for specific RT config options
    system("zcat /proc/config.gz | grep CONFIG_PREEMPT_RT > /dev/null && "
           "echo \"✓ CONFIG_PREEMPT_RT is enabled\" || "
           "echo \"✗ CONFIG_PREEMPT_RT is NOT enabled\"");
           
    system("zcat /proc/config.gz | grep CONFIG_HIGH_RES_TIMERS > /dev/null && "
           "echo \"✓ CONFIG_HIGH_RES_TIMERS is enabled\" || "
           "echo \"✗ CONFIG_HIGH_RES_TIMERS is NOT enabled\"");
}

void check_rt_scheduling() {
    struct sched_param param;
    int policy;
    int max_prio;
    
    policy = sched_getscheduler(0);
    
    printf("Current scheduling policy: ");
    switch(policy) {
        case SCHED_FIFO:  printf("SCHED_FIFO\n"); break;
        case SCHED_RR:    printf("SCHED_RR\n"); break;
        case SCHED_OTHER: printf("SCHED_OTHER (normal)\n"); break;
        case -1:          perror("sched_getscheduler"); break;
        default:          printf("Unknown (%d)\n", policy); break;
    }
    
    max_prio = sched_get_priority_max(SCHED_FIFO);
    printf("Maximum RT priority available: %d\n", max_prio);
    
    // Check if memory locking is available (crucial for RT)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == 0) {
        printf("✓ Memory locking (mlockall) successful\n");
        munlockall();
    } else {
        perror("✗ Memory locking (mlockall) failed");
    }
}

void measure_clock_resolution() {
    struct timespec res;
    
    printf("\nTimer resolution:\n");
    
    if (clock_getres(CLOCK_REALTIME, &res) == 0) {
        printf("CLOCK_REALTIME resolution: %ld.%09ld s\n", res.tv_sec, res.tv_nsec);
    } else {
        perror("clock_getres CLOCK_REALTIME");
    }
    
    if (clock_getres(CLOCK_MONOTONIC, &res) == 0) {
        printf("CLOCK_MONOTONIC resolution: %ld.%09ld s\n", res.tv_sec, res.tv_nsec);
    } else {
        perror("clock_getres CLOCK_MONOTONIC");
    }
    
    // Only available in RT kernels!
    if (clock_getres(CLOCK_MONOTONIC_RAW, &res) == 0) {
        printf("CLOCK_MONOTONIC_RAW resolution: %ld.%09ld s\n", res.tv_sec, res.tv_nsec);
    } else {
        perror("clock_getres CLOCK_MONOTONIC_RAW");
    }
}

int main() {
    printf("=== Beaglebone Black RT Kernel Verification ===\n\n");
    
    check_rt_kernel();
    printf("\n=== RT Scheduling Capabilities ===\n");
    check_rt_scheduling();
    measure_clock_resolution();
    
    printf("\n=== RT Runtime Parameters ===\n");
    system("echo -n \"RT throttling setting: \"; cat /proc/sys/kernel/sched_rt_runtime_us");
    printf("Note: -1 means no throttling, allowing 100%% CPU time for RT tasks\n");
    
    printf("\n=== CPU Core Information ===\n");
    system("cat /proc/cpuinfo | grep -m1 'model name'");
    system("cat /proc/cpuinfo | grep -m1 'cpu MHz'");
    
    return 0;
}