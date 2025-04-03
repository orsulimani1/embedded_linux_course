#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Cleanup handler
void cleanup_handler(void* arg) {
    printf("Cleanup handler called with arg: %s\n", (char*)arg);
}

// Thread that returns naturally
void* thread_return(void* arg) {
    printf("Return thread running\n");
    
    // Allocate result
    int* result = malloc(sizeof(int));
    *result = 42;
    
    printf("Return thread exiting with value %d\n", *result);
    return result;  // Return dynamically allocated result
}

// Thread that calls pthread_exit
void* thread_exit(void* arg) {
    printf("Exit thread running\n");
    
    // Register cleanup handler
    pthread_cleanup_push(cleanup_handler, "Exit thread cleanup");
    
    // Allocate result
    int* result = malloc(sizeof(int));
    *result = 100;
    
    printf("Exit thread calling pthread_exit with value %d\n", *result);
    pthread_exit(result);  // Exit with result
    
    // This code is never reached
    printf("This will not be printed\n");
    
    // Must be balanced with push (even if never executed)
    pthread_cleanup_pop(0);
}

// Thread that will be cancelled
void* thread_to_cancel(void* arg) {
    printf("Cancellable thread running\n");
    
    // Register cleanup handler
    pthread_cleanup_push(cleanup_handler, "Cancelled thread cleanup");
    
    printf("Cancellable thread entering sleep loop\n");
    
    // Loop that contains cancellation points (sleep)
    while (1) {
        printf("Cancellable thread still running...\n");
        sleep(1);  // Cancellation point
    }
    
    pthread_cleanup_pop(0);  // Never reached but required
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;
    void* thread_result;
    
    // Create thread that will return normally
    pthread_create(&thread1, NULL, thread_return, NULL);
    
    // Create thread that will call pthread_exit
    pthread_create(&thread2, NULL, thread_exit, NULL);
    
    // Create thread that will be cancelled
    pthread_create(&thread3, NULL, thread_to_cancel, NULL);
    
    // Sleep briefly to let threads start
    sleep(2);
    
    // Cancel the third thread
    printf("Main thread cancelling thread3\n");
    pthread_cancel(thread3);
    
    // Join with thread1 and get return value
    pthread_join(thread1, &thread_result);
    printf("Thread1 returned with value: %d\n", *(int*)thread_result);
    free(thread_result);  // Free the allocated result
    
    // Join with thread2 and get exit value
    pthread_join(thread2, &thread_result);
    printf("Thread2 exited with value: %d\n", *(int*)thread_result);
    free(thread_result);  // Free the allocated result
    
    // Join with thread3 (cancelled)
    pthread_join(thread3, &thread_result);
    if (thread_result == PTHREAD_CANCELED) {
        printf("Thread3 was cancelled\n");
    }
    
    return 0;
}