#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>

void signal_handler(int sig) {
    switch(sig) {
        case SIGHUP:
            syslog(LOG_INFO, "Daemon received SIGHUP. Reloading configuration.");
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Daemon received SIGTERM. Terminating.");
            closelog();
            exit(0);
            break;
    }
}

int main() {
    // Step 1: Fork from parent process
    pid_t pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Parent process exits
    if (pid > 0) {
        printf("Daemon started with PID: %d\n", pid);
        exit(EXIT_SUCCESS);
    }
    
    // Step 2: Create a new session
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Handle signals
    signal(SIGCHLD, SIG_IGN); // Ignore child
    signal(SIGHUP, signal_handler); // Catch hangup signal
    signal(SIGTERM, signal_handler); // Catch kill signal
    
    // Step 3: Fork again (recommended but optional)
    pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    // Step 4: Change working directory
    chdir("/");
    
    // Step 5: Reset file creation mask
    umask(0);
    
    // Step 6: Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Step 7: Redirect standard file descriptors to /dev/null
    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_WRONLY); // stdout
    open("/dev/null", O_WRONLY); // stderr
    
    // Set up logging
    openlog("example_daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon started");
    
    // Main daemon loop
    while (1) {
        syslog(LOG_INFO, "Daemon is running...");
        sleep(60); // Sleep for 1 minute
    }
    
    // Should never reach here
    closelog();
    return EXIT_SUCCESS;
}