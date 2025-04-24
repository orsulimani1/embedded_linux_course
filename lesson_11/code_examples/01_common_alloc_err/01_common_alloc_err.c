#include <stdio.h>
#include <stdlib.h>

int main() {
    // 1. Memory leak - forgetting to free
    int *leak = malloc(sizeof(int) * 10);
    // No free() call
    
    // 2. Use after free
    int *ptr = malloc(sizeof(int));
    *ptr = 42;
    free(ptr);
    // BAD: *ptr = 100; // Using memory after it's been freed
    
    // 3. Double free
    int *ptr2 = malloc(sizeof(int));
    free(ptr2);
    // BAD: free(ptr2); // Freeing the same memory twice
    
    // 4. Invalid free
    int stack_var = 10;
    // BAD: free(&stack_var); // Trying to free memory that wasn't dynamically allocated
    
    // 5. Buffer overflow
    int *buffer = malloc(3 * sizeof(int));
    // BAD: for(int i = 0; i < 10; i++) buffer[i] = i; // Writing beyond allocated memory
    
    free(buffer);
    return 0;
}