/**
 * timer_with_callback.c
 * 
 * This example demonstrates how to implement a timer system with callbacks
 * using POSIX timers and threads.
 * 
 * Compile with:
 *   gcc -o timer_with_callback timer_with_callback.c -lrt -pthread
 * 
 * Run with:
 *   ./timer_with_callback
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_TIMERS 10
#define CLOCKID CLOCK_REALTIME

// Callback function type
typedef void (*timer_callback_t)(void* user_data);

// Timer structure
typedef struct {
    timer_t timerid;                // POSIX timer ID
    timer_callback_t callback;      // Callback function
    void* user_data;                // User data to pass to callback
    struct sigevent sev;            // Timer event configuration
    struct itimerspec its;          // Timer specification
    int active;                     // Is this timer active?
    pthread_mutex_t mutex;          // Mutex for thread safety
    pthread_t thread;               // Thread for callback execution
    int thread_running;             // Is a thread running for this timer?
} callback_timer_t;

// Global timer array
static callback_timer_t timers[MAX_TIMERS];
static pthread_mutex_t timers_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to initialize the timer system
void timer_system_init() {
    pthread_mutex_lock(&timers_mutex);
    for (int i = 0; i < MAX_TIMERS; i++) {
        timers[i].active = 0;
        pthread_mutex_init(&timers[i].mutex, NULL);
        timers[i].thread_running = 0;
    }
    pthread_mutex_unlock(&timers_mutex);
}

// Function to clean up the timer system
void timer_system_cleanup() {
    pthread_mutex_lock(&timers_mutex);
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].active) {
            timer_delete(timers[i].timerid);
            timers[i].active = 0;
        }
        pthread_mutex_destroy(&timers[i].mutex);
    }
    pthread_mutex_unlock(&timers_mutex);
    pthread_mutex_destroy(&timers_mutex);
}

// Thread function to execute the callback
void* timer_thread(void* arg) {
    int timer_idx = *((int*)arg);
    free(arg);  // Free the allocated index memory

    pthread_mutex_lock(&timers[timer_idx].mutex);
    callback_timer_t timer = timers[timer_idx];
    pthread_mutex_unlock(&timers[timer_idx].mutex);

    // Call the callback function with the user data
    if (timer.callback) {
        timer.callback(timer.user_data);
    }

    // Mark thread as done
    pthread_mutex_lock(&timers[timer_idx].mutex);
    timers[timer_idx].thread_running = 0;
    pthread_mutex_unlock(&timers[timer_idx].mutex);

    return NULL;
}

// Signal handler for timer expiration
void timer_handler(int sig, siginfo_t *si, void *uc) {
    int timer_idx = -1;
    
    // Find which timer expired
    pthread_mutex_lock(&timers_mutex);
    for (int i = 0; i < MAX_TIMERS; i++) {
        pthread_mutex_lock(&timers[i].mutex);
        if (timers[i].active && si->si_value.sival_int == i) {
            timer_idx = i;
            // Don't start a new thread if one is already running
            if (!timers[i].thread_running) {
                timers[i].thread_running = 1;
                pthread_mutex_unlock(&timers[i].mutex);
                break;
            }
        }
        pthread_mutex_unlock(&timers[i].mutex);
    }
    pthread_mutex_unlock(&timers_mutex);

    // If this is a valid timer and no thread is running, create a thread to execute the callback
    if (timer_idx >= 0) {
        int* idx = malloc(sizeof(int));
        if (idx) {
            *idx = timer_idx;
            pthread_create(&timers[timer_idx].thread, NULL, timer_thread, idx);
            pthread_detach(timers[timer_idx].thread);
        }
    }
}

// Function to create a timer with a callback
int timer_create_with_callback(timer_callback_t callback, void* user_data, 
                            unsigned long initial_ms, unsigned long interval_ms) {
    static int initialized = 0;
    
    // Initialize signal handler only once
    if (!initialized) {
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = timer_handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
            perror("sigaction");
            return -1;
        }
        timer_system_init();
        initialized = 1;
    }
    
    // Find an available timer slot
    int timer_idx = -1;
    pthread_mutex_lock(&timers_mutex);
    for (int i = 0; i < MAX_TIMERS; i++) {
        pthread_mutex_lock(&timers[i].mutex);
        if (!timers[i].active) {
            timer_idx = i;
            timers[i].active = 1;
            timers[i].callback = callback;
            timers[i].user_data = user_data;
            pthread_mutex_unlock(&timers[i].mutex);
            break;
        }
        pthread_mutex_unlock(&timers[i].mutex);
    }
    pthread_mutex_unlock(&timers_mutex);
    
    if (timer_idx == -1) {
        fprintf(stderr, "No available timer slots\n");
        return -1;
    }
    
    // Configure the timer event
    pthread_mutex_lock(&timers[timer_idx].mutex);
    
    timers[timer_idx].sev.sigev_notify = SIGEV_SIGNAL;
    timers[timer_idx].sev.sigev_signo = SIGRTMIN;
    timers[timer_idx].sev.sigev_value.sival_int = timer_idx;
    
    // Create the timer
    if (timer_create(CLOCKID, &timers[timer_idx].sev, &timers[timer_idx].timerid) == -1) {
        perror("timer_create");
        timers[timer_idx].active = 0;
        pthread_mutex_unlock(&timers[timer_idx].mutex);
        return -1;
    }
    
    // Set up timer parameters
    timers[timer_idx].its.it_value.tv_sec = initial_ms / 1000;
    timers[timer_idx].its.it_value.tv_nsec = (initial_ms % 1000) * 1000000;
    timers[timer_idx].its.it_interval.tv_sec = interval_ms / 1000;
    timers[timer_idx].its.it_interval.tv_nsec = (interval_ms % 1000) * 1000000;
    
    // Start the timer
    if (timer_settime(timers[timer_idx].timerid, 0, &timers[timer_idx].its, NULL) == -1) {
        perror("timer_settime");
        timer_delete(timers[timer_idx].timerid);
        timers[timer_idx].active = 0;
        pthread_mutex_unlock(&timers[timer_idx].mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&timers[timer_idx].mutex);
    return timer_idx;
}

// Function to stop and delete a timer
int timer_delete_with_callback(int timer_id) {
    if (timer_id < 0 || timer_id >= MAX_TIMERS) {
        return -1;
    }
    
    pthread_mutex_lock(&timers[timer_id].mutex);
    
    if (!timers[timer_id].active) {
        pthread_mutex_unlock(&timers[timer_id].mutex);
        return -1;
    }
    
    // Stop the timer
    timers[timer_id].its.it_value.tv_sec = 0;
    timers[timer_id].its.it_value.tv_nsec = 0;
    timers[timer_id].its.it_interval.tv_sec = 0;
    timers[timer_id].its.it_interval.tv_nsec = 0;
    
    timer_settime(timers[timer_id].timerid, 0, &timers[timer_id].its, NULL);
    timer_delete(timers[timer_id].timerid);
    
    timers[timer_id].active = 0;
    pthread_mutex_unlock(&timers[timer_id].mutex);
    
    return 0;
}

// Example callbacks
void timer1_callback(void* user_data) {
    static int count = 0;
    count++;
    int timer_id = *((int*)user_data);
    
    printf("Timer %d callback executed %d times\n", timer_id, count);
    
    // Stop after 5 executions
    if (count >= 5) {
        printf("Stopping timer %d\n", timer_id);
        timer_delete_with_callback(timer_id);
    }
}

void timer2_callback(void* user_data) {
    char* message = (char*)user_data;
    printf("Timer 2 message: %s\n", message);
}

void timer3_callback(void* user_data) {
    // Simulate a long-running task
    printf("Timer 3: Starting long operation...\n");
    sleep(2); // Simulate work
    printf("Timer 3: Long operation completed\n");
}

int main() {
    printf("Timer with callback example\n");
    printf("===========================\n\n");
    
    // Create user data for timers
    int *timer1_id = malloc(sizeof(int));
    char *timer2_message = strdup("Hello from timer 2!");
    
    // Create timers with different intervals
    printf("Creating timers...\n");
    
    // Timer 1: Start after 1000ms, repeat every 1000ms
    *timer1_id = timer_create_with_callback(timer1_callback, timer1_id, 1000, 1000);
    printf("Timer 1 created with ID: %d\n", *timer1_id);
    
    // Timer 2: Start after 2000ms, repeat every 1500ms
    int timer2_id = timer_create_with_callback(timer2_callback, timer2_message, 2000, 1500);
    printf("Timer 2 created with ID: %d\n", timer2_id);
    
    // Timer 3: Start after 3000ms, repeat every 2000ms
    int timer3_id = timer_create_with_callback(timer3_callback, NULL, 3000, 2000);
    printf("Timer 3 created with ID: %d\n", timer3_id);
    
    printf("\nTimers running. Press Ctrl+C to exit.\n\n");
    
    // Wait for 15 seconds
    sleep(15);
    
    // Clean up remaining timers
    printf("\nCleaning up remaining timers...\n");
    timer_delete_with_callback(timer2_id);
    timer_delete_with_callback(timer3_id);
    
    // Free allocated memory
    free(timer2_message);
    
    // Final cleanup
    timer_system_cleanup();
    printf("Cleanup complete. Exiting.\n");
    
    return 0;
}