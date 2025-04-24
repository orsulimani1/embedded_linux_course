#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define ARRAY_SIZE 1000000
#define ITERATIONS 1000

// Function to get current time in microseconds
uint64_t get_time_microseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

void test_stack_allocation() {
    uint64_t start = get_time_microseconds();
    for (int i = 0; i < ITERATIONS; i++) {
        int array[ARRAY_SIZE];
        // Do something simple with the array to prevent optimization
        array[0] = 42;
    }
    uint64_t end = get_time_microseconds();
    double cpu_time_used = (double)(end - start) / 1000.0;
    printf("Stack allocation time: %4.4lg milli seconds\n", cpu_time_used);
}

void test_heap_allocation() {
    uint64_t start = get_time_microseconds();
    for (int i = 0; i < ITERATIONS; i++) {
        int *array = malloc(ARRAY_SIZE * sizeof(int));
        // Do something simple with the array to prevent optimization
        array[0] = 42;
        free(array);
    }
    uint64_t end = get_time_microseconds();
    double cpu_time_used = (double)(end - start) / 1000.0;
    printf("Heap allocation time: %4.4lg seconds\n", cpu_time_used);
}

int main() {
    // Use a smaller array size to avoid stack overflow
    printf("Testing with array size: %d and %d iterations\n", ARRAY_SIZE, ITERATIONS);
    
    test_stack_allocation();
    test_heap_allocation();
    
    return 0;
}