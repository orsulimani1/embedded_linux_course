#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

// Shared data structure
typedef struct {
    int counter_a;   // Process A's counter
    int counter_b;   // Process B's counter
    int active_pid;  // Currently active process PID
    int yielding;    // Whether yielding is enabled (0=no, 1=yes)
} shared_data_t;

// Global flag for signal handling
volatile int running = 1;

// Signal handler
void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

// Function to clear the terminal screen
void clear_screen() {
    printf("\033[H\033[J");  // ANSI escape sequence to clear screen
}

// Function to get current timestamp in milliseconds
long long get_timestamp_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Process A function (computation-intensive with optional yielding)
void process_a(shared_data_t *shared, char *process_name) {
    pid_t my_pid = getpid();
    
    // Set up signal handler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    while (running) {
        // Record start time for work cycle
        long long start_time = get_timestamp_ms();
        
        // Update shared data to show we're active
        shared->active_pid = my_pid;
        
        // Do computationally intensive work (simulate with a busy loop)
        for (int i = 0; i < 10000000; i++) {
            // Incrementing our counter
            shared->counter_a++;
            
            // Check if yielding is enabled (every 1,000,000 iterations)
            if (shared->yielding && (i % 1000000 == 0)) {
                sched_yield();  // Voluntarily yield the CPU
            }
        }
        
        // Calculate how long this work cycle took
        long long elapsed = get_timestamp_ms() - start_time;
        
        // Print status - only if we're still running (to avoid partial output)
        if (running) {
            printf("%s (PID %d): Completed work cycle in %lld ms - Counter A: %d, Counter B: %d %s\n",
                   process_name, my_pid, elapsed, shared->counter_a, shared->counter_b,
                   shared->yielding ? "[Yielding Enabled]" : "[Yielding Disabled]");
        }
        
        // Always sleep a tiny bit to prevent overwhelming the terminal
        usleep(10000);  // 10ms
    }
    
    printf("%s (PID %d): Exiting...\n", process_name, my_pid);
}

// Process B function (similar to A, but incrementing counter B)
void process_b(shared_data_t *shared, char *process_name) {
    pid_t my_pid = getpid();
    
    // Set up signal handler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    while (running) {
        // Record start time for work cycle
        long long start_time = get_timestamp_ms();
        
        // Update shared data to show we're active
        shared->active_pid = my_pid;
        
        // Do computationally intensive work (simulate with a busy loop)
        for (int i = 0; i < 10000000; i++) {
            // Incrementing our counter
            shared->counter_b++;
            
            // Check if yielding is enabled (every 1,000,000 iterations)
            if (shared->yielding && (i % 1000000 == 0)) {
                sched_yield();  // Voluntarily yield the CPU
            }
        }
        
        // Calculate how long this work cycle took
        long long elapsed = get_timestamp_ms() - start_time;
        
        // Print status - only if we're still running (to avoid partial output)
        if (running) {
            printf("%s (PID %d): Completed work cycle in %lld ms - Counter A: %d, Counter B: %d %s\n",
                   process_name, my_pid, elapsed, shared->counter_a, shared->counter_b,
                   shared->yielding ? "[Yielding Enabled]" : "[Yielding Disabled]");
        }
        
        // Always sleep a tiny bit to prevent overwhelming the terminal
        usleep(10000);  // 10ms
    }
    
    printf("%s (PID %d): Exiting...\n", process_name, my_pid);
}

// Control process function (parent process)
void control_process(shared_data_t *shared, pid_t pid_a, pid_t pid_b) {
    char input;
    
    // Set up signal handler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Enable non-blocking stdin
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    // Initial help message
    printf("\n===== Interactive Process Yielding Demo =====\n");
    printf("Press 'y' to toggle yielding on/off\n");
    printf("Press 'c' to clear screen\n");
    printf("Press 'q' to quit\n");
    printf("===========================================\n\n");
    
    while (running) {
        // Check for user input
        if (read(STDIN_FILENO, &input, 1) > 0) {
            switch (input) {
                case 'y':
                case 'Y':
                    // Toggle yielding
                    shared->yielding = !shared->yielding;
                    printf("\n[CONTROL] Yielding %s\n\n", 
                           shared->yielding ? "ENABLED" : "DISABLED");
                    break;
                    
                case 'c':
                case 'C':
                    // Clear screen
                    clear_screen();
                    printf("===== Interactive Process Yielding Demo =====\n");
                    printf("Press 'y' to toggle yielding on/off\n");
                    printf("Press 'c' to clear screen\n");
                    printf("Press 'q' to quit\n");
                    printf("===========================================\n\n");
                    break;
                    
                case 'q':
                case 'Q':
                    // Quit program
                    printf("\n[CONTROL] Shutting down...\n");
                    running = 0;
                    break;
            }
        }
        
        // Show system status periodically
        static long long last_status = 0;
        long long now = get_timestamp_ms();
        
        if (now - last_status > 5000) {  // Every 5 seconds
            // Get CPU info for processes
            char cmd_a[100], cmd_b[100];
            sprintf(cmd_a, "ps -p %d -o %%cpu | tail -1", pid_a);
            sprintf(cmd_b, "ps -p %d -o %%cpu | tail -1", pid_b);
            
            // Use popen to capture command output
            FILE *fp;
            char cpu_a[10] = "?", cpu_b[10] = "?";
            
            if ((fp = popen(cmd_a, "r")) != NULL) {
                if (fgets(cpu_a, sizeof(cpu_a), fp) != NULL) {
                    // Remove newline
                    cpu_a[strcspn(cpu_a, "\n")] = 0;
                }
                pclose(fp);
            }
            
            if ((fp = popen(cmd_b, "r")) != NULL) {
                if (fgets(cpu_b, sizeof(cpu_b), fp) != NULL) {
                    // Remove newline
                    cpu_b[strcspn(cpu_b, "\n")] = 0;
                }
                pclose(fp);
            }
            
            printf("\n[SYSTEM STATUS] Yielding: %s | Process A CPU: %s%% | Process B CPU: %s%%\n\n",
                   shared->yielding ? "ON" : "OFF", cpu_a, cpu_b);
                   
            last_status = now;
        }
        
        usleep(100000);  // 100ms sleep to avoid high CPU usage in control process
    }
    
    // Send termination signals to child processes
    kill(pid_a, SIGTERM);
    kill(pid_b, SIGTERM);
}

int main() {
    // Create shared memory for inter-process communication
    shared_data_t *shared = mmap(NULL, sizeof(shared_data_t),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (shared == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    // Initialize shared data
    shared->counter_a = 0;
    shared->counter_b = 0;
    shared->active_pid = 0;
    shared->yielding = 0;  // Start with yielding disabled
    
    // Fork first child process (Process A)
    pid_t pid_a = fork();
    
    if (pid_a < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid_a == 0) {
        // We are in process A
        process_a(shared, "Process A");
        exit(EXIT_SUCCESS);
    }
    
    // Fork second child process (Process B)
    pid_t pid_b = fork();
    
    if (pid_b < 0) {
        perror("fork");
        kill(pid_a, SIGTERM);  // Clean up first child
        exit(EXIT_FAILURE);
    } else if (pid_b == 0) {
        // We are in process B
        process_b(shared, "Process B");
        exit(EXIT_SUCCESS);
    }
    
    // Parent process becomes the control process
    control_process(shared, pid_a, pid_b);
    
    // Wait for child processes to finish
    int status;
    waitpid(pid_a, &status, 0);
    waitpid(pid_b, &status, 0);
    
    // Clean up shared memory
    munmap(shared, sizeof(shared_data_t));
    
    printf("All processes terminated. Demo complete.\n");
    return 0;
}