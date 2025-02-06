#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Exiting gracefully...\n");
        exit(EXIT_SUCCESS);
    } else if (signum == SIGTERM) {
        printf("\nCaught SIGTERM. Exiting gracefully...\n");
        exit(EXIT_SUCCESS);
    }
}

int main() {
    struct sigaction sa;

    // Clear the memory for the sigaction struct
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;

    // Register the handler for SIGINT
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error registering SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // Register the handler for SIGTERM
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error registering SIGTERM handler");
        exit(EXIT_FAILURE);
    }

    printf("Signal handler is set up. Process ID: %d\n", getpid());
    // Loop forever, waiting for signals
    while (1) {
        pause();  // Suspend the process until a signal is caught
    }

    return 0;
}
