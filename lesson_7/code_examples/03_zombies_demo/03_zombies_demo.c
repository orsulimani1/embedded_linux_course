#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child (PID: %d) running\n", getpid());
        exit(0); // Child exits immediately
    } else {
        // Parent process
        printf("Parent (PID: %d) created child (PID: %d)\n", getpid(), pid);
        printf("Parent sleeping for 30 seconds without waiting for child\n");
        printf("Run 'ps -ef | grep defunct' in another terminal to see zombie\n");
        
        sleep(30); // Sleep without waiting for child
        
        printf("Parent exiting\n");
        return 0;
    }
}