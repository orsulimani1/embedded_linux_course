#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

// Function executed by thread
void* thread_function(void* arg) {
    int thread_num = *(int*)arg;
    
    // Get thread IDs at both user and kernel levels
    pthread_t user_tid = pthread_self();
    pid_t kernel_tid = syscall(SYS_gettid);
    
    printf("Thread %d: User-level TID = %lu, Kernel-level TID = %d\n", 
           thread_num, (unsigned long)user_tid, kernel_tid);
    
    sleep(1);
    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_nums[3] = {1, 2, 3};
    
    printf("Main process PID: %d\n", getpid());
    printf("Main thread kernel TID: %d\n", (int)syscall(SYS_gettid));
    
    // Create three threads
    for (int i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, &thread_nums[i])) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}