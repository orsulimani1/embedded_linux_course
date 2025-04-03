#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Shared global counter
volatile long counter = 0;
volatile long iterations = 10000000; // 10 million iterations

// Thread function with race condition
void* increment_counter(void* arg) {
    int thread_id = *(int*)arg;
    printf("Thread %d starting\n", thread_id);
    
    // Increment the counter many times
    for (long i = 0; i < iterations; i++) {
        // RACE CONDITION: Read-modify-write sequence is not atomic
        counter = counter + 1;
    }
    
    printf("Thread %d finished\n", thread_id);
    return NULL;
}

// Thread function using mutex for synchronization
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile long safe_counter = 0;

void* increment_counter_safe(void* arg) {
    int thread_id = *(int*)arg;
    printf("Safe thread %d starting\n", thread_id);
    
    // Increment the counter many times with synchronization
    for (long i = 0; i < iterations; i++) {
        pthread_mutex_lock(&counter_mutex);
        safe_counter = safe_counter + 1;
        pthread_mutex_unlock(&counter_mutex);
    }
    
    printf("Safe thread %d finished\n", thread_id);
    return NULL;
}

// Main demonstration function
int main() {
    pthread_t thread1, thread2;
    int id1 = 1, id2 = 2;
    
    printf("=== Race Condition Demonstration ===\n");
    printf("Expected final count: %ld\n", iterations * 2);
    
    // Create two threads that increment the counter
    printf("Creating threads with race condition...\n");
    
    counter = 0; // Reset counter
    
    // Get the start time
    clock_t start_time = clock();
    
    pthread_create(&thread1, NULL, increment_counter, &id1);
    pthread_create(&thread2, NULL, increment_counter, &id2);
    
    // Wait for both threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Calculate elapsed time
    clock_t race_time = clock() - start_time;
    
    printf("Final counter value with race condition: %ld\n", counter);
    printf("Missing increments: %ld\n", (iterations * 2) - counter);
    printf("Time taken with race condition: %f seconds\n\n", 
           (double)race_time / CLOCKS_PER_SEC);
    
    // Now demonstrate with proper synchronization
    printf("=== Safe Synchronization Demonstration ===\n");
    printf("Creating threads with synchronization...\n");
    
    safe_counter = 0; // Reset safe counter
    
    // Get the start time
    start_time = clock();
    
    pthread_create(&thread1, NULL, increment_counter_safe, &id1);
    pthread_create(&thread2, NULL, increment_counter_safe, &id2);
    
    // Wait for both threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Calculate elapsed time
    clock_t safe_time = clock() - start_time;
    
    printf("Final counter value with synchronization: %ld\n", safe_counter);
    printf("Missing increments: %ld\n", (iterations * 2) - safe_counter);
    printf("Time taken with synchronization: %f seconds\n\n", 
           (double)safe_time / CLOCKS_PER_SEC);
    
    // Compare the performance
    printf("=== Performance Comparison ===\n");
    printf("Synchronization overhead: %f times slower\n", 
           (double)safe_time / race_time);
    
    return 0;
}