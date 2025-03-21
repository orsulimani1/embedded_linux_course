#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int main() {
    pid_t child_pid = fork();
    
    if (child_pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (child_pid == 0) {
        // Child process
        printf("Child process (PID: %d) created, parent PID: %d\n", 
               getpid(), getppid());
        
        // Sleep to give parent time to exit
        sleep(2);
        
        // After parent exits, check parent PID again
        printf("Child process (PID: %d) after parent exit, new parent PID: %d\n", 
               getpid(), getppid());
        
        // Keep running to allow inspection
        printf("Child still running. Check with 'ps -f %d' in another terminal\n", 
               getpid());
        sleep(30);
        exit(0);
    } else {
        // Parent process
        printf("Parent process (PID: %d) created child (PID: %d)\n", 
               getpid(), child_pid);
        printf("Parent exiting immediately without waiting\n");
        
        // Parent exits without waiting
        exit(0);
    }
}