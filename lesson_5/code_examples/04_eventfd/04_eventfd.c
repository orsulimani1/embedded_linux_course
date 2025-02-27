#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>   // gettimeofday
#include <stdarg.h>     // va_list etc

// Data structure in shared memory
#define MSG_SIZE 1024

typedef struct {
    // the message
    char  message[MSG_SIZE];
    // a timestamp for the message
    char  message_timestamp[64];

    // bitmask of which threads should receive the message (bits for each thread index)
    // e.g., if bit i is set => threadID=(2+i) is targeted
    uint32_t receivers_mask;

    // ack_mask to track which threads have responded
    // each thread sets its bit here before signaling T1
    uint32_t ack_mask;

    // the response is conceptually from the "last" responding thread,
    // but for demonstration we store only one. (In multi-target scenario,
    // it might be overwritten or each thread might build a combined response array.)
    char  response[MSG_SIZE];
    char  response_timestamp[64];
} shared_data_t;

static shared_data_t *g_shared = NULL;
static size_t g_shm_size = sizeof(shared_data_t);

// T1's eventfd: all receivers write here to ack
static int efdT1 = -1;

// The eventfds for each receiver thread
static int *efdReceivers = NULL; // array of length N

// Number of receiver threads
static int N = 2; // default


// safe_print() with flockfile/unlockfile to avoid interleaving
static void safe_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    flockfile(stdout); // lock
    vfprintf(stdout, fmt, args);
    funlockfile(stdout); // unlock

    va_end(args);
}


// Helper: increment an eventfd
static void signal_eventfd(int efd) {
    uint64_t inc = 1;
    if (write(efd, &inc, sizeof(inc)) < 0) {
        perror("write eventfd");
    }
}

// Helper: block on eventfd
static void wait_eventfd(int efd) {
    uint64_t val;
    if (read(efd, &val, sizeof(val)) < 0) {
        perror("read eventfd");
    }
}


// get_timestamp_ms() => "123456 ms"
static void get_timestamp_ms(char *buf, size_t buflen) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    snprintf(buf, buflen, "%ld ms", ms);
}


// Per-thread arg struct
typedef struct {
    int threadID;  // 2..N+1
    int efdMe;     // eventfd for this thread
} receiver_arg_t;


// Receiver thread function
// each thread sees if its bit is set in g_shared->receivers_mask
// If so, it processes the message, sets its bit in ack_mask, signals T1

static void* receiver_thread(void *arg) {
    receiver_arg_t *rarg = (receiver_arg_t*)arg;
    int myID  = rarg->threadID;    // e.g. 2..N+1
    int efdMe = rarg->efdMe;       // my eventfd

    // The index i in [0..N-1]
    int i = myID - 2;

    safe_print("[T%d] Started.\n", myID);

    while (1) {
        // 1) Wait until T1 signals me
        wait_eventfd(efdMe);

        // 2) msync read => get updated data
        if (msync(g_shared, g_shm_size, MS_SYNC | MS_INVALIDATE) < 0) {
            perror("[Rx] msync read");
        }

        // 3) Check if my bit is set in receivers_mask
        uint32_t mask = (1 << i);
        if ((g_shared->receivers_mask & mask) == 0) {
            // Not for me => ignore
            continue;
        }

        // We are targeted => read the message
        safe_print("[T%d] Received message: '%s'\n", myID, g_shared->message);
        safe_print("[T%d] Message timestamp: %s\n", myID, g_shared->message_timestamp);

        // Build a response (just a simple string)
        snprintf(g_shared->response, MSG_SIZE, 
                 "T%d acked the message (maskBit=%u)", myID, mask);

        // Set response timestamp
        get_timestamp_ms(g_shared->response_timestamp, sizeof(g_shared->response_timestamp));

        // 4) Set my bit in ack_mask
        g_shared->ack_mask |= mask;

        // 5) msync => so T1 sees it
        if (msync(g_shared, g_shm_size, MS_SYNC | MS_INVALIDATE) < 0) {
            perror("[Rx] msync write");
        }

        // 6) signal T1 => "I have acked"
        signal_eventfd(efdT1);
    }
    return NULL;
}


// main / T1
// usage: ./prog [N]
static int main_impl(void) {
    safe_print("[T1] Will create %d receiver threads (IDs=2..%d)\n", N, N+1);

    // 1) create shared memory
    g_shared = mmap(NULL, g_shm_size,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS,
                    -1, 0);
    if (g_shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    memset(g_shared, 0, g_shm_size);

    // 2) create T1's eventfd
    efdT1 = eventfd(0, 0);
    if (efdT1 < 0) {
        perror("eventfd T1");
        return 1;
    }

    // create array of eventfds for each receiver
    efdReceivers = calloc(N, sizeof(int));
    if (!efdReceivers) {
        perror("calloc efdReceivers");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        efdReceivers[i] = eventfd(0, 0);
        if (efdReceivers[i] < 0) {
            perror("eventfd for receiver");
            return 1;
        }
    }

    // 3) start N receiver threads
    pthread_t *rxThreads = calloc(N, sizeof(pthread_t));
    receiver_arg_t *rxArgs = calloc(N, sizeof(receiver_arg_t));
    if (!rxThreads || !rxArgs) {
        perror("calloc rx data");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        rxArgs[i].threadID = 2 + i;    // e.g. 2..(N+1)
        rxArgs[i].efdMe    = efdReceivers[i];
        pthread_create(&rxThreads[i], NULL, receiver_thread, &rxArgs[i]);
    }
    uint32_t max_mask = (1 << N) - 1;
    safe_print("[T1] All receivers started. Ready to send bitmask+message.\n");

    // 4) T1 loop
    while (1) {
        char input[MSG_SIZE];
        safe_print("[T1] Enter: 'bitmask decimal' + ' message' or 'exit':\n> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break; // EOF or error
        }
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0) {
            safe_print("[T1] Exiting main loop.\n");
            break;
        }

        // parse bitmask
        char *msg_ptr = NULL;
        uint32_t bitmask = strtoul(input, &msg_ptr, 10);
        while (*msg_ptr == ' ') msg_ptr++;

        // fill in shared memory
        strncpy(g_shared->message, msg_ptr, MSG_SIZE - 1);
        g_shared->message[MSG_SIZE - 1] = '\0';
        get_timestamp_ms(g_shared->message_timestamp, sizeof(g_shared->message_timestamp));

        if (bitmask > max_mask) {
            safe_print("[T1] Invalid bitmask 0x%X: it exceeds the number of threads (%d). Max=0x%X\n",
                       bitmask, N, max_mask);
            continue;
        }
        // set receivers_mask
        g_shared->receivers_mask = bitmask;
        // clear ack_mask
        g_shared->ack_mask = 0;

        // msync so receivers see it
        if (msync(g_shared, g_shm_size, MS_SYNC | MS_INVALIDATE) < 0) {
            perror("[T1] msync write");
        }

        // 5) signal each thread whose bit is set in bitmask
        for (int i = 0; i < N; i++) {
            uint32_t mask = (1 << i);
            if ((bitmask & mask) != 0) {
                signal_eventfd(efdReceivers[i]);
            }
        }

        // 6) Wait until ack_mask == receivers_mask
        while (1) {
            // wait for *some* ack
            wait_eventfd(efdT1);
            // read new ack_mask
            if (msync(g_shared, g_shm_size, MS_SYNC | MS_INVALIDATE) < 0) {
                perror("[T1] msync read ack");
            }

            uint32_t ack = g_shared->ack_mask;
            if (ack == bitmask) {
                // all targeted threads have acked
                break;
            }
            else {
                // not all have acked => keep waiting
                // but we might want to re-check or do partial log
                safe_print("[T1] Partial ack: ack_mask=0x%X, still waiting.\n", ack);
            }
        }

        // final response: we just store the last thread's response, but we don't know which set last
        safe_print("[T1] All threads in bitmask 0x%X acked. Last response:\n  '%s'\n",
                   bitmask, g_shared->response);
        safe_print("[T1] Response timestamp: %s\n", g_shared->response_timestamp);
    }

    // Cleanup
    munmap(g_shared, g_shm_size);
    close(efdT1);
    for (int i = 0; i < N; i++) {
        close(efdReceivers[i]);
    }
    free(efdReceivers);

    safe_print("[T1] Main done. (In real code, we'd stop threads & join them.)\n");
    return 0;
}


// main wrapper to parse N from argv
int main(int argc, char *argv[]) {
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N < 1) {
            safe_print("Usage: %s [NumReceivers >=1]\n", argv[0]);
            return 1;
        }
    }
    return main_impl();
}
