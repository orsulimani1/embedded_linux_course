// Process creation example
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();  // Create a new process
    
    if (pid < 0) {
        // Error occurred
        fprintf(stderr, "Fork failed\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
    } else {
        // Parent process
        printf("Parent process: PID = %d, child PID = %d\n", getpid(), pid);
    }
    
    return 0;
}