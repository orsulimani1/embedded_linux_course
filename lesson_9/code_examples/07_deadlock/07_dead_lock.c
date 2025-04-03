#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

// Two shared resources (mutexes)
pthread_mutex_t resource_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resource_B = PTHREAD_MUTEX_INITIALIZER;

// Global flag to control demonstration mode
bool use_deadlock_prevention = false;

// Thread function that causes deadlock
void* thread1_function(void* arg) {
    printf("Thread 1: Starting\n");
    
    // Lock resource A first
    printf("Thread 1: Attempting to lock resource A\n");
    pthread_mutex_lock(&resource_A);
    printf("Thread 1: Locked resource A\n");
    
    // Simulate some work
    printf("Thread 1: Working with resource A\n");
    sleep(1);
    
    if (use_deadlock_prevention) {
        // Deadlock prevention: Always acquire locks in the same order
        printf("Thread 1: Attempting to lock resource B (with deadlock prevention)\n");
        pthread_mutex_lock(&resource_B);
        printf("Thread 1: Locked resource B\n");
    } else {
        // This will cause deadlock
        printf("Thread 1: Attempting to lock resource B\n");
        pthread_mutex_lock(&resource_B);
        printf("Thread 1: Locked resource B - THIS LINE WON'T EXECUTE DUE TO DEADLOCK\n");
    }
    
    // Use both resources
    printf("Thread 1: Working with both resources\n");
    
    // Unlock in reverse order
    pthread_mutex_unlock(&resource_B);
    printf("Thread 1: Unlocked resource B\n");
    
    pthread_mutex_unlock(&resource_A);
    printf("Thread 1: Unlocked resource A\n");
    
    printf("Thread 1: Finished\n");
    return NULL;
}

// Thread function that also causes deadlock
void* thread2_function(void* arg) {
    printf("Thread 2: Starting\n");
    
    if (use_deadlock_prevention) {
        // Deadlock prevention: Always acquire locks in the same order (A then B)
        printf("Thread 2: Attempting to lock resource A (with deadlock prevention)\n");
        pthread_mutex_lock(&resource_A);
        printf("Thread 2: Locked resource A\n");
        
        // Simulate some work
        printf("Thread 2: Working with resource A\n");
        sleep(1);
        
        printf("Thread 2: Attempting to lock resource B\n");
        pthread_mutex_lock(&resource_B);
        printf("Thread 2: Locked resource B\n");
    } else {
        // Lock resource B first (opposite order from thread1)
        printf("Thread 2: Attempting to lock resource B\n");
        pthread_mutex_lock(&resource_B);
        printf("Thread 2: Locked resource B\n");
        
        // Simulate some work
        printf("Thread 2: Working with resource B\n");
        sleep(1);
        
        // This will cause deadlock
        printf("Thread 2: Attempting to lock resource A\n");
        pthread_mutex_lock(&resource_A);
        printf("Thread 2: Locked resource A - THIS LINE WON'T EXECUTE DUE TO DEADLOCK\n");
    }
    
    // Use both resources
    printf("Thread 2: Working with both resources\n");
    
    // Unlock in reverse order
    if (use_deadlock_prevention) {
        pthread_mutex_unlock(&resource_B);
        printf("Thread 2: Unlocked resource B\n");
        
        pthread_mutex_unlock(&resource_A);
        printf("Thread 2: Unlocked resource A\n");
    } else {
        pthread_mutex_unlock(&resource_A);
        printf("Thread 2: Unlocked resource A\n");
        
        pthread_mutex_unlock(&resource_B);
        printf("Thread 2: Unlocked resource B\n");
    }
    
    printf("Thread 2: Finished\n");
    return NULL;
}

// Function implementing trylock with timeout
bool mutex_lock_with_timeout(pthread_mutex_t* mutex, int timeout_seconds) {
    time_t start_time = time(NULL);
    
    while (pthread_mutex_trylock(mutex) != 0) {
        // Check if we've exceeded the timeout
        if (time(NULL) - start_time >= timeout_seconds) {
            return false; // Timeout
        }
        
        // Sleep a bit before trying again
        usleep(10000); // 10ms
    }
    
    return true; // Successfully acquired the lock
}

// Thread function using timeout to prevent deadlock
void* thread1_function_timeout(void* arg) {
    printf("Thread 1 (Timeout): Starting\n");
    
    // Lock resource A first
    printf("Thread 1 (Timeout): Attempting to lock resource A\n");
    pthread_mutex_lock(&resource_A);
    printf("Thread 1 (Timeout): Locked resource A\n");
    
    // Simulate some work
    printf("Thread 1 (Timeout): Working with resource A\n");
    sleep(1);
    
    // Try to lock resource B with timeout
    printf("Thread 1 (Timeout): Attempting to lock resource B (with timeout)\n");
    if (mutex_lock_with_timeout(&resource_B, 3)) {
        printf("Thread 1 (Timeout): Locked resource B\n");
        
        // Use both resources
        printf("Thread 1 (Timeout): Working with both resources\n");
        
        // Unlock resource B
        pthread_mutex_unlock(&resource_B);
        printf("Thread 1 (Timeout): Unlocked resource B\n");
    } else {
        printf("Thread 1 (Timeout): Failed to acquire resource B, avoiding deadlock\n");
        printf("Thread 1 (Timeout): Releasing resource A to try again later\n");
    }
    
    // Unlock resource A
    pthread_mutex_unlock(&resource_A);
    printf("Thread 1 (Timeout): Unlocked resource A\n");
    
    printf("Thread 1 (Timeout): Finished\n");
    return NULL;
}

// Thread function using timeout to prevent deadlock
void* thread2_function_timeout(void* arg) {
    printf("Thread 2 (Timeout): Starting\n");
    
    // Lock resource B first
    printf("Thread 2 (Timeout): Attempting to lock resource B\n");
    pthread_mutex_lock(&resource_B);
    printf("Thread 2 (Timeout): Locked resource B\n");
    
    // Simulate some work
    printf("Thread 2 (Timeout): Working with resource B\n");
    sleep(1);
    
    // Try to lock resource A with timeout
    printf("Thread 2 (Timeout): Attempting to lock resource A (with timeout)\n");
    if (mutex_lock_with_timeout(&resource_A, 3)) {
        printf("Thread 2 (Timeout): Locked resource A\n");
        
        // Use both resources
        printf("Thread 2 (Timeout): Working with both resources\n");
        
        // Unlock resource A
        pthread_mutex_unlock(&resource_A);
        printf("Thread 2 (Timeout): Unlocked resource A\n");
    } else {
        printf("Thread 2 (Timeout): Failed to acquire resource A, avoiding deadlock\n");
        printf("Thread 2 (Timeout): Releasing resource B to try again later\n");
    }
    
    // Unlock resource B
    pthread_mutex_unlock(&resource_B);
    printf("Thread 2 (Timeout): Unlocked resource B\n");
    
    printf("Thread 2 (Timeout): Finished\n");
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    // Initialize mutexes
    pthread_mutex_init(&resource_A, NULL);
    pthread_mutex_init(&resource_B, NULL);
    
    printf("=== Deadlock Demonstration ===\n\n");
    printf("This program demonstrates different approaches to handling potential deadlocks:\n");
    printf("1. Creating a deadlock situation\n");
    printf("2. Using consistent lock ordering to prevent deadlock\n");
    printf("3. Using lock timeouts to recover from potential deadlocks\n\n");
    
    // Part 1: Demonstrate deadlock
    printf("=== Part 1: Demonstrating Deadlock ===\n");
    printf("In this part, Thread 1 locks resource A then tries to lock B,\n");
    printf("while Thread 2 locks resource B then tries to lock A.\n");
    printf("This will result in a deadlock.\n\n");
    
    use_deadlock_prevention = false;
    
    // Create threads that will deadlock
    pthread_create(&thread1, NULL, thread1_function, NULL);
    
    // Sleep briefly to ensure thread1 runs first
    usleep(100000);
    
    pthread_create(&thread2, NULL, thread2_function, NULL);
    
    // Wait a few seconds to observe the deadlock
    printf("Waiting to observe deadlock... (5 seconds)\n");
    sleep(5);
    
    printf("\nAs you can see, both threads are now stuck waiting for resources.\n");
    printf("Thread 1 has resource A and is waiting for resource B.\n");
    printf("Thread 2 has resource B and is waiting for resource A.\n");
    printf("This is a classic deadlock situation.\n\n");
    
    // Cancel the deadlocked threads
    printf("Cancelling deadlocked threads...\n");
    pthread_cancel(thread1);
    pthread_cancel(thread2);
    
    // Wait for threads to be cancelled
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Re-initialize mutexes
    pthread_mutex_destroy(&resource_A);
    pthread_mutex_destroy(&resource_B);
    pthread_mutex_init(&resource_A, NULL);
    pthread_mutex_init(&resource_B, NULL);
    
    printf("\n=== Part 2: Preventing Deadlock with Lock Ordering ===\n");
    printf("In this part, both threads acquire locks in the same order: A then B.\n");
    printf("This prevents deadlock through proper resource allocation ordering.\n\n");
    
    use_deadlock_prevention = true;
    
    // Create threads with deadlock prevention
    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);
    
    // Wait for threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nBoth threads completed successfully with no deadlock!\n\n");
    
    // Re-initialize mutexes for the third demonstration
    pthread_mutex_destroy(&resource_A);
    pthread_mutex_destroy(&resource_B);
    pthread_mutex_init(&resource_A, NULL);
    pthread_mutex_init(&resource_B, NULL);
    
    printf("\n=== Part 3: Preventing Deadlock with Lock Timeouts ===\n");
    printf("In this part, threads use trylock with timeout instead of blocking indefinitely.\n");
    printf("If a thread cannot acquire a lock within the timeout period,\n");
    printf("it releases any held locks and can retry later.\n\n");
    
    // Create threads with timeout-based deadlock prevention
    pthread_create(&thread1, NULL, thread1_function_timeout, NULL);
    pthread_create(&thread2, NULL, thread2_function_timeout, NULL);
    
    // Wait for threads to complete
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nBoth threads completed successfully with timeout-based deadlock prevention!\n");
    
    // Clean up
    pthread_mutex_destroy(&resource_A);
    pthread_mutex_destroy(&resource_B);
    
    return 0;
}