#include <stdio.h>
#include <stdlib.h>

// Global variables go in the data segment
int global_initialized = 42;
int global_uninitialized;

int main() {
    // Local variables go on the stack
    int stack_var = 100;
    
    // Dynamically allocated memory goes on the heap
    int *heap_var = malloc(sizeof(int));
    *heap_var = 200;
    
    printf("Text (code) segment: %p\n", (void*)main);
    printf("Data segment (initialized): %p\n", (void*)&global_initialized);
    printf("BSS segment (uninitialized): %p\n", (void*)&global_uninitialized);
    printf("Stack variable: %p\n", (void*)&stack_var);
    printf("Heap variable: %p\n", (void*)heap_var);
    
    // Don't forget to free allocated memory
    free(heap_var);
    
    return 0;
}