#define _POSIX_C_SOURCE 200809L  // or a similar POSIX version macro

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

// Signal handler for SIGCHLD
void handle_sigchld(int sig) {
    int saved_errno = errno; // Save errno
    
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        // Reap all terminated children
    }
    
    errno = saved_errno; // Restore errno
}

int main() {
    // Set up signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = &handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    // Create several child processes
    for (int i = 0; i < 5; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process
            printf("Child %d (PID: %d) running...\n", i, getpid());
            sleep(i + 1); // Each child runs for a different time
            printf("Child %d (PID: %d) exiting\n", i, getpid());
            exit(0);
        }
    }
    
    // Parent continues running without explicitly waiting
    printf("Parent (PID: %d) continuing to run...\n", getpid());
    
    // Parent does work while children terminate
    for (int i = 0; i < 10; i++) {
        printf("Parent working... (%d)\n", i);
        sleep(1);
    }
    
    printf("Parent exiting\n");
    return 0;
}