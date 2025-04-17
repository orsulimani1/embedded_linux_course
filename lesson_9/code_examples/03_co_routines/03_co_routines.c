#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <time.h>
#include <sys/time.h>

#define STACK_SIZE 16384
#define NUM_FIBONACCI 28
#define NUM_ITERATIONS 10

// ----- Coroutine Implementation -----

typedef struct {
    ucontext_t caller;
    ucontext_t callee;
    char stack[STACK_SIZE];
    unsigned long value;
    int is_done;
} Coroutine;

// Generator function that yields Fibonacci numbers
void fibonacci_generator(Coroutine* co) {
    unsigned long a = 0;
    unsigned long b = 1;
    
    // Yield the first two Fibonacci numbers
    co->value = a;
    swapcontext(&co->callee, &co->caller);
    
    co->value = b;
    swapcontext(&co->callee, &co->caller);
    
    // Generate and yield the rest of the sequence
    while (1) {
        unsigned long next = a + b;
        a = b;
        b = next;
        
        co->value = next;
        swapcontext(&co->callee, &co->caller);
    }
    
    // Never reached, but good practice
    co->is_done = 1;
}

// Initialize a coroutine
void coroutine_init(Coroutine* co) {
    co->is_done = 0;
    
    getcontext(&co->callee);
    co->callee.uc_stack.ss_sp = co->stack;
    co->callee.uc_stack.ss_size = STACK_SIZE;
    co->callee.uc_link = NULL;
    
    makecontext(&co->callee, (void (*)())fibonacci_generator, 1, co);
}

// Get next value from the coroutine
unsigned long coroutine_next(Coroutine* co) {
    if (co->is_done) {
        return 0;  // Indicate that coroutine is done
    }
    
    // Resume the coroutine execution
    swapcontext(&co->caller, &co->callee);
    
    return co->value;
}

// ----- Traditional Recursive Implementation -----

unsigned long fibonacci_recursive(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    return fibonacci_recursive(n-1) + fibonacci_recursive(n-2);
}

// ----- Traditional Iterative Implementation -----

unsigned long fibonacci_iterative(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    unsigned long a = 0;
    unsigned long b = 1;
    unsigned long result = 0;
    
    for (int i = 2; i <= n; i++) {
        result = a + b;
        a = b;
        b = result;
    }
    
    return result;
}

// ----- Helper function to get current time in microseconds -----

double get_time_usec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000000 + (double)tv.tv_usec;
}

// ----- Main program with benchmarking -----

int main() {
    double start_time, end_time;
    unsigned long result;
    double coroutine_time = 0, recursive_time = 0, iterative_time = 0;
    
    printf("Comparing Fibonacci calculation methods (averaged over %d iterations)\n\n", NUM_ITERATIONS);
    
    // Run multiple iterations to get more reliable timing
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        
        // ----- Test Coroutine Implementation -----
        
        start_time = get_time_usec();
        
        Coroutine co;
        coroutine_init(&co);
        
        // Skip to the nth Fibonacci number
        for (int i = 0; i <= NUM_FIBONACCI; i++) {
            result = coroutine_next(&co);
        }
        
        end_time = get_time_usec();
        coroutine_time += (end_time - start_time);
        
        printf("Iteration %2d: Fibonacci(%d) using coroutine = %lu (%.2f μs)\n", 
               iter+1, NUM_FIBONACCI, result, end_time - start_time);
        
        // ----- Test Iterative Implementation -----
        
        start_time = get_time_usec();
        result = fibonacci_iterative(NUM_FIBONACCI);
        end_time = get_time_usec();
        iterative_time += (end_time - start_time);
        
        printf("Iteration %2d: Fibonacci(%d) using iterative = %lu (%.2f μs)\n", 
               iter+1, NUM_FIBONACCI, result, end_time - start_time);
        
        // ----- Test Recursive Implementation (only for small n) -----
        
        if (NUM_FIBONACCI <= 40) {  // Recursive is too slow for larger values
            start_time = get_time_usec();
            result = fibonacci_recursive(NUM_FIBONACCI);
            end_time = get_time_usec();
            recursive_time += (end_time - start_time);
            
            printf("Iteration %2d: Fibonacci(%d) using recursive = %lu (%.2f μs)\n\n", 
                   iter+1, NUM_FIBONACCI, result, end_time - start_time);
        }
    }
    
    // Calculate averages
    coroutine_time /= NUM_ITERATIONS;
    iterative_time /= NUM_ITERATIONS;
    recursive_time /= NUM_ITERATIONS;
    
    printf("\n===== RESULTS (average over %d iterations) =====\n", NUM_ITERATIONS);
    printf("Coroutine implementation: %.2f μs\n", coroutine_time);
    printf("Iterative implementation: %.2f μs\n", iterative_time);
    printf("Recursive implementation: %.2f μs\n", recursive_time);
    
    printf("\nPerformance comparison:\n");
    printf("- Coroutine vs Iterative: %.2fx %s\n", 
           coroutine_time > iterative_time ? coroutine_time / iterative_time : iterative_time / coroutine_time,
           coroutine_time > iterative_time ? "slower" : "faster");
    
    if (NUM_FIBONACCI <= 40) {
        printf("- Coroutine vs Recursive: %.2fx %s\n", 
               coroutine_time > recursive_time ? coroutine_time / recursive_time : recursive_time / coroutine_time,
               coroutine_time > recursive_time ? "slower" : "faster");
        
        printf("- Iterative vs Recursive: %.2fx %s\n", 
               iterative_time > recursive_time ? iterative_time / recursive_time : recursive_time / iterative_time,
               iterative_time > recursive_time ? "slower" : "faster");
    }
    
    return 0;
}