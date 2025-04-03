#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MAX_QUEUE_SIZE 100
#define NUM_WORKERS 4
#define NUM_TASKS 20

// Task structure
typedef struct {
    int task_id;
    void (*function)(void* data);
    void* data;
} Task;

// Work queue structure
typedef struct {
    Task* tasks[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
    bool shutdown;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} WorkQueue;

// Task data structure
typedef struct {
    int value;
    int processing_time; // in milliseconds
} TaskData;

// Create a new work queue
WorkQueue* create_work_queue() {
    WorkQueue* queue = (WorkQueue*)malloc(sizeof(WorkQueue));
    if (queue == NULL) {
        return NULL;
    }
    
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
    queue->shutdown = false;
    
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->not_empty, NULL);
    pthread_cond_init(&queue->not_full, NULL);
    
    return queue;
}

// Clean up the work queue
void destroy_work_queue(WorkQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    // Free any remaining tasks in the queue
    while (queue->count > 0) {
        Task* task = queue->tasks[queue->front];
        queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
        queue->count--;
        
        free(task->data);
        free(task);
    }
    
    pthread_mutex_unlock(&queue->mutex);
    
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->not_empty);
    pthread_cond_destroy(&queue->not_full);
    
    free(queue);
}

// Add a task to the queue
bool enqueue_task(WorkQueue* queue, Task* task) {
    pthread_mutex_lock(&queue->mutex);
    
    // Wait until there's space or the queue is shutting down
    while (queue->count == MAX_QUEUE_SIZE && !queue->shutdown) {
        pthread_cond_wait(&queue->not_full, &queue->mutex);
    }
    
    // Don't add if we're shutting down
    if (queue->shutdown) {
        pthread_mutex_unlock(&queue->mutex);
        return false;
    }
    
    // Add the task to the queue
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
    queue->tasks[queue->rear] = task;
    queue->count++;
    
    // Signal that the queue is not empty
    pthread_cond_signal(&queue->not_empty);
    
    pthread_mutex_unlock(&queue->mutex);
    return true;
}

// Get a task from the queue
Task* dequeue_task(WorkQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    // Wait until there's a task or the queue is shutting down
    while (queue->count == 0 && !queue->shutdown) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    // If the queue is empty and shutting down, return NULL
    if (queue->count == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    
    // Get the task from the queue
    Task* task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    queue->count--;
    
    // Signal that the queue is not full
    pthread_cond_signal(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
    return task;
}

// Signal all threads to shut down
void shutdown_work_queue(WorkQueue* queue) {
    pthread_mutex_lock(&queue->mutex);
    
    queue->shutdown = true;
    
    // Wake up all waiting threads
    pthread_cond_broadcast(&queue->not_empty);
    pthread_cond_broadcast(&queue->not_full);
    
    pthread_mutex_unlock(&queue->mutex);
}

// Example task function - processes a value
void process_task(void* arg) {
    TaskData* data = (TaskData*)arg;
    
    // Get thread ID for demonstration
    pthread_t thread_id = pthread_self();
    
    printf("Thread %lu processing task with value %d (sleep: %d ms)\n", 
           (unsigned long)thread_id, data->value, data->processing_time);
    
    // Simulate processing time
    usleep(data->processing_time * 1000);
    
    printf("Thread %lu completed task with value %d (result: %d)\n", 
           (unsigned long)thread_id, data->value, data->value * data->value);
}

// Worker thread function
void* worker_thread(void* arg) {
    WorkQueue* queue = (WorkQueue*)arg;
    
    pthread_t thread_id = pthread_self();
    printf("Worker thread %lu started\n", (unsigned long)thread_id);
    
    while (true) {
        // Get the next task from the queue
        Task* task = dequeue_task(queue);
        
        if (task == NULL) {
            // Queue is being shut down
            printf("Worker thread %lu shutting down\n", (unsigned long)thread_id);
            break;
        }
        
        // Execute the task
        task->function(task->data);
        
        // Clean up
        free(task);
    }
    
    return NULL;
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Create the work queue
    WorkQueue* queue = create_work_queue();
    
    // Create worker threads
    pthread_t workers[NUM_WORKERS];
    printf("Creating %d worker threads\n", NUM_WORKERS);
    
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (pthread_create(&workers[i], NULL, worker_thread, queue) != 0) {
            fprintf(stderr, "Error creating worker thread %d\n", i);
            return 1;
        }
    }
    
    // Add tasks to the queue
    printf("Adding %d tasks to the queue\n", NUM_TASKS);
    
    for (int i = 0; i < NUM_TASKS; i++) {
        // Create task data with random processing time
        TaskData* data = (TaskData*)malloc(sizeof(TaskData));
        data->value = i + 1;
        data->processing_time = 100 + rand() % 900;  // 100-1000ms
        
        // Create the task
        Task* task = (Task*)malloc(sizeof(Task));
        task->task_id = i + 1;
        task->function = process_task;
        task->data = data;
        
        // Add to queue
        enqueue_task(queue, task);
        
        // Small delay between task creation
        usleep(50000);  // 50ms
    }
    
    // Wait for a bit to allow tasks to be processed
    printf("Waiting for tasks to complete...\n");
    sleep(5);
    
    // Shutdown the queue
    printf("Shutting down the work queue\n");
    shutdown_work_queue(queue);
    
    // Wait for all worker threads to finish
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(workers[i], NULL);
    }
    
    // Clean up
    destroy_work_queue(queue);
    
    printf("All done!\n");
    
    return 0;
}