#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// Global termination flag
volatile sig_atomic_t running = 1;

// Reference count for active background threads
int active_threads = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_cond = PTHREAD_COND_INITIALIZER;

// Signal handler
void handle_signal(int sig) {
    running = 0;
}

// Register a background thread
void register_thread() {
    pthread_mutex_lock(&count_mutex);
    active_threads++;
    pthread_mutex_unlock(&count_mutex);
}

// Unregister a background thread
void unregister_thread() {
    pthread_mutex_lock(&count_mutex);
    active_threads--;
    // Signal if this was the last thread
    if (active_threads == 0) {
        pthread_cond_signal(&count_cond);
    }
    pthread_mutex_unlock(&count_mutex);
}

// Background thread function
void* background_thread(void* arg) {
    int thread_num = *(int*)arg;
    free(arg);
    
    register_thread();
    
    printf("Background thread %d started\n", thread_num);
    
    // Simulate periodic work
    int count = 0;
    while (running && count < 10) {
        printf("Background thread %d doing work\n", thread_num);
        sleep(1);
        count++;
        
        // Check termination flag periodically
        if (!running) {
            printf("Background thread %d received termination signal\n", thread_num);
            break;
        }
    }
    
    printf("Background thread %d exiting\n", thread_num);
    
    unregister_thread();
    return NULL;
}

int main() {
    pthread_t threads[3];
    int i;
    
    // Set up signal handling
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    printf("Starting background threads\n");
    
    // Create background threads
    for (i = 0; i < 3; i++) {
        int* thread_num = malloc(sizeof(int));
        *thread_num = i + 1;
        
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        
        pthread_create(&threads[i], &attr, background_thread, thread_num);
        
        pthread_attr_destroy(&attr);
    }
    
    printf("Main thread will continue with other work\n");
    
    // Simulate main thread work
    for (i = 0; i < 5; i++) {
        printf("Main thread doing work\n");
        sleep(1);
    }
    
    printf("Main thread work complete\n");
    
    // Option 1: Exit immediately, background threads continue
    if (0) {
        printf("Exiting without waiting for background threads\n");
        return 0;
    }
    
    // Option 2: Tell threads to terminate and wait for them
    printf("Signaling background threads to terminate\n");
    running = 0;
    
    // Wait for all background threads to finish
    pthread_mutex_lock(&count_mutex);
    while (active_threads > 0) {
        printf("Waiting for %d background threads to terminate\n", active_threads);
        pthread_cond_wait(&count_cond, &count_mutex);
    }
    pthread_mutex_unlock(&count_mutex);
    
    printf("All background threads have terminated\n");
    
    return 0;
}