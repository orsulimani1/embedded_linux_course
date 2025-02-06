#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

// Global variable shared by all threads and processes
int global_var = 100;

// Thread function: modifies the global variable
void *thread_function(void *arg) {
    printf("Inside thread: initial global_var = %d\n", global_var);
    global_var = 300;
    printf("Inside thread: modified global_var = %d\n", global_var);
    pthread_exit(NULL);
}

int main() {
    pid_t pid;
    int status;

    printf("Before fork: global_var = %d\n", global_var);
    
    pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Child process block
        global_var = 200;  // Modify the global variable
        printf("Child process: global_var modified to %d\n", global_var);
        printf("Child process: PID = %d, Parent PID = %d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    } else {
        wait(&status);  // Wait for the child process to finish
        printf("Parent process: global_var after child modification = %d\n", global_var);
    }

    // ===== Thread demonstration using pthread =====
    // Reset the global variable before thread demonstration.
    global_var = 100;
    printf("Before thread: global_var = %d\n", global_var);
    
    pthread_t tid;
    if (pthread_create(&tid, NULL, thread_function, NULL) != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
    
    // Wait for the thread to complete
    pthread_join(tid, NULL);
    printf("Main thread: global_var after thread modification = %d\n", global_var);
    // Note: The main thread sees the updated value (300) because threads share memory.

    return 0;
}
