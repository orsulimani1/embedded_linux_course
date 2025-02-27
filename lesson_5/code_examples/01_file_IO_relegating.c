#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_IO_THREADS 2

void *io_worker(void *arg) {
    FILE *fp = (FILE *)arg;
    for (int i = 0; i < 5; i++) {
        fprintf(fp, "Thread %ld writing...\n", pthread_self());
        fflush(fp);
    }
    fclose(fp);
    return NULL;
}

int main() {
    pthread_t threads[NUM_IO_THREADS];
    FILE *streams[NUM_IO_THREADS];

    // Open one file per thread
    for (int i = 0; i < NUM_IO_THREADS; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "thread_%d.log", i);
        streams[i] = fopen(filename, "w");
    }

    // Assign one file per thread
    for (int i = 0; i < NUM_IO_THREADS; i++) {
        pthread_create(&threads[i], NULL, io_worker, streams[i]);
    }

    for (int i = 0; i < NUM_IO_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
