#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

#define GPIO_PIN "48"  // P9_15

// Helper function to write to sysfs files
int write_to_file(const char *filename, const char *value) {
    int fd = open(filename, O_WRONLY);
    if (fd < 0) {
        perror("Error opening file");
        return -1;
    }
    
    int ret = write(fd, value, strlen(value));
    close(fd);
    
    if (ret < 0) {
        perror("Error writing to file");
        return -1;
    }
    
    return 0;
}

// Read value from a sysfs file
int read_from_file(const char *filename) {
    char value[2];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file for reading");
        return -1;
    }
    
    if (read(fd, value, 1) != 1) {
        perror("Error reading from file");
        close(fd);
        return -1;
    }
    
    close(fd);
    return (value[0] == '1') ? 1 : 0;
}

int main() {
    // Export GPIO pin
    if (write_to_file("/sys/class/gpio/export", GPIO_PIN) < 0) {
        printf("GPIO may already be exported\n");
    }
    
    // Set direction to input
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", GPIO_PIN);
    write_to_file(path, "in");
    
    // Read GPIO value
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", GPIO_PIN);
    int value = read_from_file(path);
    printf("GPIO %s value: %d\n", GPIO_PIN, value);
    
    // Unexport GPIO
    write_to_file("/sys/class/gpio/unexport", GPIO_PIN);
    
    return 0;
}