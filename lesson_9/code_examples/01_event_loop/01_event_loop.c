#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>

typedef struct {
    int id;
    int interval;  // in seconds
    int last_run;  // timestamp
} Task;

void run_task(Task* task, int current_time) {
    printf("Running task %d at time %d\n", task->id, current_time);
    task->last_run = current_time;
}

int main() {
    // Set up some tasks
    Task tasks[3] = {
        {1, 2, 0},  // Task 1: run every 2 seconds
        {2, 5, 0},  // Task 2: run every 5 seconds
        {3, 3, 0}   // Task 3: run every 3 seconds
    };
    
    int time_elapsed = 0;
    
    printf("Starting event loop\n");
    
    // Simple event loop
    while (time_elapsed < 20) {  // Run for 20 seconds
        // Check for tasks to run
        for (int i = 0; i < 3; i++) {
            if (time_elapsed - tasks[i].last_run >= tasks[i].interval) {
                run_task(&tasks[i], time_elapsed);
            }
        }
        
        // Wait for 1 second
        sleep(1);
        time_elapsed++;
    }
    
    printf("Event loop finished\n");
    
    return 0;
}