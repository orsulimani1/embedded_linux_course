/**
 * memory_alignment_benchmark.c
 * 
 * This program demonstrates the performance difference between
 * aligned and unaligned memory access using various methods.
 * 
 * Compile with:
 * 
 * Run with:
 *   ./memory_alignment_benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

// Size of data to process
#define ARRAY_SIZE (50 * 1024 * 1024)  // 50 million elements
#define NUM_ITERATIONS 5               // Run multiple times for better measurement
#define NUM_TESTS 3                    // Different tests

// Function to get current time in microseconds
uint64_t get_time_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

// Function to allocate aligned memory
void* aligned_malloc(size_t size, size_t alignment) {
    void* result;
    if (posix_memalign(&result, alignment, size) != 0) {
        return NULL;
    }
    return result;
}

// Function to allocate unaligned memory (offset by 1 byte from alignment)
void* unaligned_malloc(size_t size, size_t alignment) {
    // Allocate extra space for the offset and alignment
    unsigned char* buffer = (unsigned char*)malloc(size + alignment);
    if (!buffer) {
        return NULL;
    }
    
    // Calculate the offset to make it unaligned by 1 byte
    size_t offset = alignment - ((uintptr_t)(buffer) % alignment);
    if (offset == alignment) {
        offset = 1;  // Ensure it's not aligned
    } else {
        offset = offset + 1;  // Add 1 to make it unaligned
    }
    
    // Store the original pointer before the data for freeing later
    void** original_ptr = (void**)(buffer + offset - sizeof(void*));
    *original_ptr = buffer;
    
    return buffer + offset;
}

// Function to free memory allocated with unaligned_malloc
void unaligned_free(void* ptr) {
    if (ptr) {
        void** original_ptr = (void**)((unsigned char*)ptr - sizeof(void*));
        free(*original_ptr);
    }
}

// Check alignment of a pointer
int is_aligned(const void* ptr, size_t alignment) {
    return ((uintptr_t)ptr % alignment) == 0;
}

/**
 * Test 1: Simple read-modify-write benchmark
 * This test performs simple operations on double arrays
 */
double test_rmw(double* data, size_t count) {
    // Simple operation: multiply each element by 1.01
    for (size_t i = 0; i < count; i++) {
        data[i] *= 1.01;
    }
    
    // Compute a checksum to prevent optimization
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * Test 2: SIMD-friendly operation
 * This test performs operations that benefit from SIMD
 */
double test_simd_friendly(double* data, size_t count) {
    // Vector-friendly operation
    for (size_t i = 0; i < count - 4; i += 4) {
        data[i] = data[i] * 2.0;
        data[i+1] = data[i+1] * 2.0;
        data[i+2] = data[i+2] * 2.0;
        data[i+3] = data[i+3] * 2.0;
    }
    
    // Compute a checksum
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += data[i];
    }
    return sum;
}

/**
 * Test 3: Random access benchmark
 * This test accesses memory in a random pattern
 */
double test_random_access(double* data, size_t count) {
    // Create an array of random indices
    size_t* indices = (size_t*)malloc(count * sizeof(size_t));
    for (size_t i = 0; i < count; i++) {
        indices[i] = rand() % count;
    }
    
    // Random access pattern
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += data[indices[i]];
    }
    
    free(indices);
    return sum;
}

// Run a specific test and measure performance
double run_test(int test_num, double* data, size_t count) {
    double result = 0.0;
    
    switch (test_num) {
        case 0:
            result = test_rmw(data, count);
            break;
        case 1:
            result = test_simd_friendly(data, count);
            break;
        case 2:
            result = test_random_access(data, count);
            break;
    }
    
    return result;
}

// Initialize data array with values
void initialize_data(double* data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        data[i] = (double)i / count;
    }
}

// Print benchmark results
void print_results(const char* test_name, 
                double aligned_time, 
                double unaligned_time, 
                double aligned_checksum,
                double unaligned_checksum) {
    printf("%-20s | %12.6f ms | %12.6f ms | %10.2f%% | %10.2f\n",
        test_name,
        aligned_time,
        unaligned_time,
        (unaligned_time / aligned_time - 1.0) * 100.0,
        fabs(aligned_checksum - unaligned_checksum));
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Allocate memory for data
    size_t data_size = ARRAY_SIZE * sizeof(double);
    double* aligned_data = aligned_malloc(data_size, 64);   // 64-byte alignment (cache line)
    double* unaligned_data = unaligned_malloc(data_size, 64);
    
    if (!aligned_data || !unaligned_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Check alignment
    printf("Aligned data pointer:   %p (aligned to 64 bytes: %s)\n", 
        (void*)aligned_data, is_aligned(aligned_data, 64) ? "yes" : "no");
    printf("Unaligned data pointer: %p (aligned to 64 bytes: %s)\n", 
        (void*)unaligned_data, is_aligned(unaligned_data, 64) ? "yes" : "no");
    printf("\n");
    
    // Test names
    const char* test_names[] = {
        "Simple RMW",
        "SIMD-friendly",
        "Random Access"
    };
    
    // Print header
    printf("%-20s | %12s | %12s | %10s | %10s\n", 
        "Test", "Aligned", "Unaligned", "Overhead %", "Checksum Diff");
    printf("---------------------+----------------+----------------+------------+------------\n");
    
    // Run tests
    for (int test = 0; test < NUM_TESTS; test++) {
        double aligned_total_time = 0.0;
        double unaligned_total_time = 0.0;
        double aligned_checksum = 0.0;
        double unaligned_checksum = 0.0;
        
        for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
            // Reset data for both arrays
            initialize_data(aligned_data, ARRAY_SIZE);
            initialize_data(unaligned_data, ARRAY_SIZE);
            
            // Test aligned access
            uint64_t start_time = get_time_microseconds();
            aligned_checksum = run_test(test, aligned_data, ARRAY_SIZE);
            uint64_t end_time = get_time_microseconds();
            aligned_total_time += (end_time - start_time) / 1000.0;  // Convert to milliseconds
            
            // Reset unaligned data if needed
            if (test == 0 || test == 1) {
                initialize_data(unaligned_data, ARRAY_SIZE);
            }
            
            // Test unaligned access
            start_time = get_time_microseconds();
            unaligned_checksum = run_test(test, unaligned_data, ARRAY_SIZE);
            end_time = get_time_microseconds();
            unaligned_total_time += (end_time - start_time) / 1000.0;  // Convert to milliseconds
        }
        
        // Average times
        double aligned_avg_time = aligned_total_time / NUM_ITERATIONS;
        double unaligned_avg_time = unaligned_total_time / NUM_ITERATIONS;
        
        // Print results
        print_results(test_names[test], aligned_avg_time, unaligned_avg_time, 
                    aligned_checksum, unaligned_checksum);
    }
    
    // Clean up
    free(aligned_data);
    unaligned_free(unaligned_data);
    
    printf("\nNotes:\n");
    printf("1. Overhead %% shows how much slower unaligned access is compared to aligned\n");
    printf("2. Results may vary based on CPU architecture and compiler optimizations\n");
    printf("3. Modern x86/x64 CPUs handle unaligned access better than older or other architectures\n");
    printf("4. Checksum Diff should be very small (floating point precision differences)\n");
    
    return 0;
}