#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

int main() {
    int current_nice, new_nice;
    
    // Get current nice value
    current_nice = getpriority(PRIO_PROCESS, 0);
    printf("Current nice value: %d\n", current_nice);
    
    // Set a new nice value (higher number = lower priority)
    new_nice = 10;  // Lower priority
    if (setpriority(PRIO_PROCESS, 0, new_nice) == -1) {
        perror("setpriority");
        exit(EXIT_FAILURE);
    }
    
    // Verify the new nice value
    current_nice = getpriority(PRIO_PROCESS, 0);
    printf("New nice value: %d\n", current_nice);
    
    return 0;
}