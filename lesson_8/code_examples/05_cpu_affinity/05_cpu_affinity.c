#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

int main() {
    int cpu_count, i;
    cpu_set_t cpu_set;
    
    // Get number of available CPUs
    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    printf("System has %d CPU(s)\n", cpu_count);
    
    // Get current affinity
    CPU_ZERO(&cpu_set);
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1) {
        perror("sched_getaffinity");
        exit(EXIT_FAILURE);
    }
    
    printf("Current CPU affinity: ");
    for (i = 0; i < cpu_count; i++) {
        printf("%d ", CPU_ISSET(i, &cpu_set));
    }
    printf("\n");
    
    // Set affinity to CPU 0 only (assuming at least one CPU)
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
    
    printf("Set affinity to CPU 0 only\n");
    
    // Verify new affinity
    CPU_ZERO(&cpu_set);
    if (sched_getaffinity(0, sizeof(cpu_set_t), &cpu_set) == -1) {
        perror("sched_getaffinity");
        exit(EXIT_FAILURE);
    }
    
    printf("New CPU affinity: ");
    for (i = 0; i < cpu_count; i++) {
        printf("%d ", CPU_ISSET(i, &cpu_set));
    }
    printf("\n");
    
    return 0;
}