#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define NUM_WORKERS 4            // Number of worker threads
#define MQ_NAME "/log_mq"        // Name of POSIX message queue
#define LOG_MESSAGE_SIZE 256     // Max log message size
#define MQ_MAX_MESSAGES 10       // Max messages in the queue

// I/O thread function: receives messages and writes them to a file
void* io_thread_func(void *arg) {
    mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open (I/O thread)");
        return NULL;
    }

    FILE *fp = fopen("logs.txt", "a");
    if (!fp) {
        perror("fopen");
        mq_close(mq);
        return NULL;
    }

    flockfile(fp); // Lock file for exclusive access
    flockfile(stdout); // Lock file for exclusive access

    char message[LOG_MESSAGE_SIZE];
    while (1) {
        ssize_t bytes_read = mq_receive(mq, message, LOG_MESSAGE_SIZE, NULL);
        if (bytes_read > 0) {
            message[bytes_read] = '\0'; // Null-terminate message
            if (strcmp(message, "EXIT") == 0) break; // Termination signal

            fputs_unlocked(message, fp); // Efficient, unlocked I/O
            fflush(fp); // Ensure immediate write
            fputs_unlocked(message, stdout); // Efficient, unlocked I/O
            fflush(fp); // Ensure immediate write
        } else {
            perror("mq_receive");
        }
    }
    funlockfile(stdout); // Lock file for exclusive access
    funlockfile(fp);
    fclose(fp);
    mq_close(mq);
    mq_unlink(MQ_NAME); // Delete the message queue
    return NULL;
}

// Worker thread function: generates log messages and sends them via message queue
void* worker_thread_func(void *arg) {
    int id = *(int*)arg;
    char message[LOG_MESSAGE_SIZE];

    mqd_t mq = mq_open(MQ_NAME, O_WRONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open (Worker)");
        return NULL;
    }

    for (int i = 0; i < 5; i++) { // Each worker sends 5 messages
        snprintf(message, LOG_MESSAGE_SIZE, "Worker %d: Log Entry %d\n", id, i);
        
        if (mq_send(mq, message, strlen(message), 0) == -1) {
            perror("mq_send");
        }
        usleep(100000); // Simulate processing delay
    }

    mq_close(mq);
    return NULL;
}

int main() {
    pthread_t io_thread, worker_threads[NUM_WORKERS];
    int worker_ids[NUM_WORKERS];

    // Define message queue attributes
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MQ_MAX_MESSAGES;
    attr.mq_msgsize = LOG_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    // Create message queue
    mqd_t mq = mq_open(MQ_NAME, O_CREAT | O_RDWR , 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open (Main)");
        return 1;
    }
    mq_close(mq); // Close since each thread will open separately

    // Start the I/O thread
    pthread_create(&io_thread, NULL, io_thread_func, NULL);

    // Start worker threads
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_ids[i] = i + 1;
        pthread_create(&worker_threads[i], NULL, worker_thread_func, &worker_ids[i]);
    }

    // Wait for worker threads to finish
    for (int i = 0; i < NUM_WORKERS; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    // Signal I/O thread to terminate
    mq = mq_open(MQ_NAME, O_WRONLY);
    if (mq_send(mq, "EXIT", 4, 0) == -1) {
        perror("mq_send (EXIT)");
    }
    mq_close(mq);
    
    pthread_join(io_thread, NULL);

    printf("Logging complete. Check logs.txt\n");
    return 0;
}
