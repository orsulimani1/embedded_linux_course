#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process (PID: %d) running...\n", getpid());
        pid_t parent = getppid();
        kill(parent, SIGTERM);

        sleep(10); // Child will run for 10 seconds unless terminated
        printf("Child process completed normally\n");
        exit(0);
    } else {
        // Parent process
        printf("Parent process (PID: %d) created child (PID: %d)\n", 
               getpid(), pid);
        
        sleep(10); // Give child time to start
        
        // Send SIGTERM to child process
        printf("Parent sending SIGTERM to child...\n");
        kill(pid, SIGTERM);
        
        printf("Parent process completed\n");
        return 0;
    }
}
