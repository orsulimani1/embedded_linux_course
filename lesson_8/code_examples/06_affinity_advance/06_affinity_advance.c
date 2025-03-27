#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

// Change this to match your system's CPU count
#define MAX_CPUS 8

// Shared data structure to track performance metrics
typedef struct {
    long long total_ops[MAX_CPUS];  // Total operations performed by each worker
    double    elapsed_time[MAX_CPUS]; // Time taken by each worker
    int       cpu_temp[MAX_CPUS];    // CPU temperature (simulated)
    int       cache_misses[MAX_CPUS]; // Cache misses (simulated)
    int       active_workers;        // Number of active workers
    int       test_running;          // Whether the test is still running
} shared_data_t;

// Worker function state
typedef struct {
    int worker_id;          // Worker ID
    int cpu_id;             // CPU core to run on
    int iterations;         // Number of iterations to run
    int work_type;          // Type of workload (0=CPU, 1=memory, 2=mixed)
    shared_data_t* shared;  // Shared data for reporting results
} worker_params_t;

// Global variables
volatile int running = 1;
const char* work_type_names[] = {"CPU-bound", "Memory-bound", "Mixed workload"};

// Signal handler
void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nReceived termination signal. Shutting down...\n");
        running = 0;
    }
}

// Get current timestamp in milliseconds
double get_timestamp_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (ts.tv_nsec / 1000000000.0);
}

// Read CPU temperature (simulated)
int read_cpu_temp(int cpu_id) {
    // Simulate temperature between 40-75°C with some randomness
    // In a real system, you would read from /sys/class/thermal/
    return 40 + (cpu_id * 2) + (rand() % 10);
}

// Worker function - does the actual work based on work_type
void* worker_function(void* arg) {
    worker_params_t* params = (worker_params_t*)arg;
    int cpu_id = params->cpu_id;
    int worker_id = params->worker_id;
    int work_type = params->work_type;
    shared_data_t* shared = params->shared;
    long long ops_count = 0;
    
    // Allocate memory for memory-intensive operations
    const size_t buffer_size = 64 * 1024 * 1024; // 64MB
    char* buffer = NULL;
    
    if (work_type == 1 || work_type == 2) {
        // For memory-bound or mixed workloads, allocate buffer
        buffer = (char*)malloc(buffer_size);
        if (!buffer) {
            fprintf(stderr, "Worker %d: Failed to allocate memory\n", worker_id);
            return NULL;
        }
        
        // Initialize buffer with some data
        for (size_t i = 0; i < buffer_size; i++) {
            buffer[i] = (char)(i & 0xFF);
        }
    }
    
    // Set the CPU affinity for this thread
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu_id, &cpu_set);
    
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set) != 0) {
        fprintf(stderr, "Worker %d: Failed to set CPU affinity to CPU %d\n", worker_id, cpu_id);
        free(buffer);
        return NULL;
    }
    
    // Verify our affinity was set correctly
    CPU_ZERO(&cpu_set);
    if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set) != 0) {
        fprintf(stderr, "Worker %d: Failed to get CPU affinity\n", worker_id);
        free(buffer);
        return NULL;
    }
    
    if (!CPU_ISSET(cpu_id, &cpu_set)) {
        fprintf(stderr, "Worker %d: Failed to set affinity to CPU %d\n", worker_id, cpu_id);
        free(buffer);
        return NULL;
    }
    
    printf("Worker %d: Started on CPU %d with %s workload\n", 
           worker_id, cpu_id, work_type_names[work_type]);
    
    // Record start time
    double start_time = get_timestamp_sec();
    
    // Main work loop
    while (running && shared->test_running && ops_count < params->iterations) {
        switch (work_type) {
            case 0: // CPU-bound workload - just do calculations
                for (int i = 0; i < 1000000; i++) {
                    // Do some meaningless but CPU-intensive calculations
                    volatile double result = 0.0;
                    for (int j = 1; j <= 100; j++) {
                        result += j / (result + 1.0);
                    }
                }
                ops_count++;
                break;
                
            case 1: // Memory-bound workload - scan and modify memory
                for (int i = 0; i < 3; i++) {
                    // Scan buffer with random access pattern
                    volatile int sum = 0;
                    for (size_t j = 0; j < buffer_size; j += 4096) {
                        sum += buffer[j];
                        buffer[j] = (char)(sum & 0xFF);
                    }
                }
                ops_count++;
                break;
                
            case 2: // Mixed workload - both CPU and memory
                for (int i = 0; i < 100000; i++) {
                    // Some CPU work
                    volatile double result = 0.0;
                    for (int j = 1; j <= 10; j++) {
                        result += j / (result + 1.0);
                    }
                }
                
                // Some memory work
                volatile int sum = 0;
                for (size_t j = 0; j < buffer_size; j += 16384) {
                    sum += buffer[j];
                    buffer[j] = (char)(sum & 0xFF);
                }
                ops_count++;
                break;
        }
        
        // Update shared data periodically
        if (ops_count % 10 == 0) {
            shared->total_ops[worker_id] = ops_count;
            shared->elapsed_time[worker_id] = get_timestamp_sec() - start_time;
            shared->cpu_temp[worker_id] = read_cpu_temp(cpu_id);
            
            // Simulate cache misses - higher for memory-bound workloads
            if (work_type == 0) {
                shared->cache_misses[worker_id] = rand() % 1000;  // Low cache misses
            } else if (work_type == 1) {
                shared->cache_misses[worker_id] = 5000 + (rand() % 5000);  // High cache misses
            } else {
                shared->cache_misses[worker_id] = 2000 + (rand() % 3000);  // Medium cache misses
            }
        }
    }
    
    // Record final statistics
    double end_time = get_timestamp_sec();
    shared->total_ops[worker_id] = ops_count;
    shared->elapsed_time[worker_id] = end_time - start_time;
    
    printf("Worker %d: Completed %lld operations in %.2f seconds on CPU %d\n",
           worker_id, ops_count, shared->elapsed_time[worker_id], cpu_id);
    
    // Clean up
    if (buffer) {
        free(buffer);
    }
    
    return NULL;
}

// Display system information
void display_system_info() {
    printf("\n===== System Information =====\n");
    
    // Get number of available CPUs
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of available CPUs: %ld\n", num_cpus);
    
    // Get CPU information from /proc/cpuinfo
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (cpuinfo) {
        char line[256];
        int cpu_count = 0;
        char cpu_model[256] = "Unknown";
        
        while (fgets(line, sizeof(line), cpuinfo)) {
            if (strncmp(line, "processor", 9) == 0) {
                cpu_count++;
            } else if (strncmp(line, "model name", 10) == 0) {
                char* colon = strchr(line, ':');
                if (colon && *(colon + 1) == ' ' && cpu_count == 1) {
                    strncpy(cpu_model, colon + 2, sizeof(cpu_model) - 1);
                    // Remove newline
                    char* newline = strchr(cpu_model, '\n');
                    if (newline) *newline = '\0';
                }
            }
        }
        fclose(cpuinfo);
        printf("CPU model: %s\n", cpu_model);
    }
    
    // Get memory information
    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        char line[256];
        long total_mem = 0;
        
        while (fgets(line, sizeof(line), meminfo)) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                // Parse MemTotal line
                sscanf(line + 9, "%ld", &total_mem);
                break;
            }
        }
        fclose(meminfo);
        printf("Total memory: %ld KB (%.2f GB)\n", total_mem, total_mem / 1024.0 / 1024.0);
    }
    
    printf("==============================\n\n");
}

// Display test configuration
void display_test_config(int num_workers, worker_params_t* params) {
    printf("\n===== Test Configuration =====\n");
    printf("Number of workers: %d\n", num_workers);
    
    for (int i = 0; i < num_workers; i++) {
        printf("Worker %d: Running on CPU %d with %s workload\n", 
               params[i].worker_id, params[i].cpu_id, work_type_names[params[i].work_type]);
    }
    printf("==============================\n\n");
}

// Display test results in a formatted table
void display_test_results(shared_data_t* shared, int num_workers, worker_params_t* params) {
    printf("\n============ Test Results ============\n");
    printf("%-8s %-8s %-15s %-12s %-10s %-12s %-10s\n", 
           "Worker", "CPU", "Workload", "Operations", "Time (s)", "Ops/sec", "Temp (°C)");
    printf("----------- ------------------------- --------------------------------\n");
    
    for (int i = 0; i < num_workers; i++) {
        double ops_per_sec = shared->total_ops[i] / shared->elapsed_time[i];
        printf("%-8d %-8d %-15s %-12lld %-10.2f %-12.2f %-10d\n",
               i, params[i].cpu_id, work_type_names[params[i].work_type],
               shared->total_ops[i], shared->elapsed_time[i], ops_per_sec, shared->cpu_temp[i]);
    }
    
    printf("========================================\n\n");
    
    // Print observations about CPU affinity effects
    printf("Observations:\n");
    
    // Check if same workload performs differently on different CPUs
    for (int i = 0; i < num_workers; i++) {
        for (int j = i + 1; j < num_workers; j++) {
            if (params[i].work_type == params[j].work_type && params[i].cpu_id != params[j].cpu_id) {
                double ops_per_sec_i = shared->total_ops[i] / shared->elapsed_time[i];
                double ops_per_sec_j = shared->total_ops[j] / shared->elapsed_time[j];
                double diff_percent = fabs(ops_per_sec_i - ops_per_sec_j) / 
                                     ((ops_per_sec_i + ops_per_sec_j) / 2.0) * 100.0;
                
                if (diff_percent > 5.0) {
                    printf("- Same workload (%s) performs %.1f%% %s on CPU %d compared to CPU %d\n",
                           work_type_names[params[i].work_type], diff_percent,
                           (ops_per_sec_i > ops_per_sec_j) ? "better" : "worse",
                           params[i].cpu_id, params[j].cpu_id);
                }
            }
        }
    }
    
    // Check for temperature differences
    int max_temp = -1, min_temp = 999, max_temp_cpu = -1, min_temp_cpu = -1;
    for (int i = 0; i < num_workers; i++) {
        if (shared->cpu_temp[i] > max_temp) {
            max_temp = shared->cpu_temp[i];
            max_temp_cpu = params[i].cpu_id;
        }
        if (shared->cpu_temp[i] < min_temp) {
            min_temp = shared->cpu_temp[i];
            min_temp_cpu = params[i].cpu_id;
        }
    }
    
    if (max_temp_cpu != min_temp_cpu) {
        printf("- CPU %d runs %.1f°C hotter than CPU %d under load\n",
               max_temp_cpu, (float)(max_temp - min_temp), min_temp_cpu);
    }
    
    // Check for cache behavior
    int max_cache_misses = -1, min_cache_misses = 999999, max_cm_worker = -1, min_cm_worker = -1;
    for (int i = 0; i < num_workers; i++) {
        if (shared->cache_misses[i] > max_cache_misses) {
            max_cache_misses = shared->cache_misses[i];
            max_cm_worker = i;
        }
        if (shared->cache_misses[i] < min_cache_misses) {
            min_cache_misses = shared->cache_misses[i];
            min_cm_worker = i;
        }
    }
    
    printf("- %s workload (Worker %d) experienced %.1fx more cache misses than %s workload (Worker %d)\n",
           work_type_names[params[max_cm_worker].work_type], max_cm_worker,
           (float)max_cache_misses / min_cache_misses,
           work_type_names[params[min_cm_worker].work_type], min_cm_worker);
           
    printf("\nRecommendations based on results:\n");
    printf("- CPU-bound workloads perform best on CPUs: ");
    for (int i = 0; i < num_workers; i++) {
        if (params[i].work_type == 0 && 
            shared->total_ops[i] / shared->elapsed_time[i] > 
            (shared->total_ops[0] / shared->elapsed_time[0]) * 0.9) {
            printf("%d ", params[i].cpu_id);
        }
    }
    printf("\n");
    
    printf("- Memory-bound workloads perform best on CPUs: ");
    for (int i = 0; i < num_workers; i++) {
        if (params[i].work_type == 1 && 
            shared->total_ops[i] / shared->elapsed_time[i] > 
            (shared->total_ops[1] / shared->elapsed_time[1]) * 0.9) {
            printf("%d ", params[i].cpu_id);
        }
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int num_workers = 0;
    int test_duration = 10; // Default test duration in seconds
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    
    // Validate CPU count
    if (num_cpus <= 0) {
        fprintf(stderr, "Error: Could not determine number of CPUs\n");
        return EXIT_FAILURE;
    }
    
    if (num_cpus > MAX_CPUS) {
        printf("Warning: System has %d CPUs but program is configured for max %d\n",
               num_cpus, MAX_CPUS);
        num_cpus = MAX_CPUS;
    }
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            test_duration = atoi(argv[i+1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [-d <duration>]\n", argv[0]);
            printf("Options:\n");
            printf("  -d <duration>  Set test duration in seconds (default: 10)\n");
            printf("  -h, --help     Show this help message\n");
            return EXIT_SUCCESS;
        }
    }
    
    // Set up signal handler
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Display system information
    display_system_info();
    
    // Create shared memory for workers to report results
    shared_data_t* shared = mmap(NULL, sizeof(shared_data_t),
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (shared == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    
    // Initialize shared data
    memset(shared, 0, sizeof(shared_data_t));
    shared->test_running = 1;
    
    // Determine number of workers based on CPU count
    // We'll create one worker of each type for each CPU, for comparison
    num_workers = num_cpus * 3; // 3 work types per CPU
    
    // If too many workers, reduce to a reasonable number
    if (num_workers > MAX_CPUS * 3) {
        num_workers = MAX_CPUS * 3;
    }
    
    // Create worker threads
    pthread_t* threads = (pthread_t*)malloc(num_workers * sizeof(pthread_t));
    worker_params_t* params = (worker_params_t*)malloc(num_workers * sizeof(worker_params_t));
    
    if (!threads || !params) {
        perror("malloc");
        free(threads);
        free(params);
        munmap(shared, sizeof(shared_data_t));
        return EXIT_FAILURE;
    }
    
    shared->active_workers = num_workers;
    
    // Initialize and create worker threads
    for (int i = 0; i < num_workers; i++) {
        int cpu_idx = i % num_cpus;
        int work_type = i / num_cpus;
        
        params[i].worker_id = i;
        params[i].cpu_id = cpu_idx;
        params[i].iterations = 1000000; // Large enough to run for the duration
        params[i].work_type = work_type % 3; // 0, 1, or 2
        params[i].shared = shared;
        
        if (pthread_create(&threads[i], NULL, worker_function, &params[i]) != 0) {
            fprintf(stderr, "Failed to create worker thread %d\n", i);
            shared->active_workers--;
        }
    }
    
    // Display test configuration
    display_test_config(num_workers, params);
    
    // Monitor and display progress for the test duration
    printf("Running test for %d seconds...\n", test_duration);
    
    // Main monitoring loop
    int progress_bar_width = 50;
    for (int t = 0; t < test_duration && running; t++) {
        // Display progress bar
        int filled_width = (t * progress_bar_width) / test_duration;
        printf("\rProgress: [");
        for (int i = 0; i < progress_bar_width; i++) {
            printf(i < filled_width ? "#" : " ");
        }
        printf("] %d%%", (t * 100) / test_duration);
        fflush(stdout);
        
        // Sleep for 1 second
        sleep(1);
    }
    printf("\rProgress: [");
    for (int i = 0; i < progress_bar_width; i++) {
        printf("#");
    }
    printf("] 100%%\n");
    
    // Stop the test
    shared->test_running = 0;
    
    // Wait for all worker threads to finish
    for (int i = 0; i < num_workers; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Display test results
    display_test_results(shared, num_workers, params);
    
    // Clean up
    free(threads);
    free(params);
    munmap(shared, sizeof(shared_data_t));
    
    printf("Test completed successfully.\n");
    return EXIT_SUCCESS;
}