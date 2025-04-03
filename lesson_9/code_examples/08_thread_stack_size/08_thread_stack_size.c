#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_function(void* arg) {
    printf("Thread with custom stack size running\n");
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_attr_t attr;
    size_t stack_size = 64 * 1024;  // 64KB stack
    int ret;
    
    // Initialize attributes
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        printf("Error initializing attributes\n");
        return 1;
    }
    
    // Set stack size
    ret = pthread_attr_setstacksize(&attr, stack_size);
    if (ret != 0) {
        printf("Error setting stack size\n");
        pthread_attr_destroy(&attr);
        return 1;
    }
    
    // Create thread with custom attributes
    ret = pthread_create(&thread, &attr, thread_function, NULL);
    if (ret != 0) {
        printf("Error creating thread\n");
        pthread_attr_destroy(&attr);
        return 1;
    }
    
    // Clean up attributes
    pthread_attr_destroy(&attr);
    
    // Wait for thread to complete
    pthread_join(thread, NULL);
    
    return 0;
}