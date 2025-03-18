#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        printf("Child PID: %d, Parent PID: %d\n", getpid(), getppid());
        // Simulate some work
        sleep(2);
        printf("Child done. Exiting...\n");
        exit(42); // child exit code
    } else {
        // Parent process
        printf("Parent PID: %d, created Child PID: %d\n", getpid(), pid);

        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0); // reap the child to avoid zombies
        if (WIFEXITED(status)) {
            printf("Child exited with code %d\n", WEXITSTATUS(status));
        }
        printf("Parent process continues...\n");
    }

    // Both parent and child (after child exit) eventually reach the end
    return 0;
}
