#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child process (PID: %d) running...\n", getpid());
        sleep(15);
        printf("Child process exiting with status 42\n");
        exit(42); // Exit with status 42
    } else {
        // Parent process
        printf("Parent waiting for child (PID: %d) to terminate...\n", pid);
        
        int status;
        pid_t child_pid = wait(&status);
        
        if (WIFEXITED(status)) {
            printf("Child %d terminated normally with exit status %d\n", 
                   child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child %d terminated by signal %d\n", 
                   child_pid, WTERMSIG(status));
        }
        
        return 0;
    }
}