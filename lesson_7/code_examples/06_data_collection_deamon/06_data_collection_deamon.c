#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <time.h>

#define TEMP_FILE "/sys/class/thermal/thermal_zone0/temp"
#define LOG_FILE "/var/log/temp_daemon.log"

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    if (sig == SIGTERM) {
        syslog(LOG_INFO, "Temperature daemon terminating");
        running = 0;
    }
}

int read_cpu_temp() {
    FILE *file = fopen(TEMP_FILE, "r");
    if (!file) {
        syslog(LOG_ERR, "Failed to open temperature file");
        return -1;
    }
    
    int temp;
    fscanf(file, "%d", &temp);
    fclose(file);
    
    return temp / 1000; // Convert to degrees Celsius
}

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        printf("Temperature daemon started with PID: %d\n", pid);
        exit(EXIT_SUCCESS);
    }
    
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    
    signal(SIGTERM, signal_handler);
    
    // Change directory and set umask
    chdir("/");
    umask(0);
    
    // Close and redirect standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    
    // Set up logging
    openlog("temp_daemon", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Temperature monitoring daemon started");
    
    // Open log file
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        syslog(LOG_ERR, "Failed to open log file");
        closelog();
        exit(EXIT_FAILURE);
    }
    
    // Main daemon loop
    while (running) {
        int temp = read_cpu_temp();
        
        if (temp >= 0) {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[26];
            strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
            
            fprintf(log_file, "%s CPU Temperature: %d°C\n", time_str, temp);
            fflush(log_file);
            
            if (temp > 70) {
                syslog(LOG_WARNING, "High CPU temperature: %d°C", temp);
            }
        }
        
        sleep(60); // Record temperature every minute
    }
    
    fclose(log_file);
    closelog();
    return EXIT_SUCCESS;
}