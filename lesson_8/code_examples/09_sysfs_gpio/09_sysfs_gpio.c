#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define GPIO_PIN "60"  // P9_12

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

int main() {
    // Export GPIO pin
    if (write_to_file("/sys/class/gpio/export", GPIO_PIN) < 0) {
        printf("GPIO may already be exported\n");
    }
    
    // Set direction to output
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", GPIO_PIN);
    write_to_file(path, "out");
    
    // Set GPIO high
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", GPIO_PIN);
    write_to_file(path, "1");
    printf("GPIO %s set to HIGH\n", GPIO_PIN);
    sleep(1);
    
    // Set GPIO low
    write_to_file(path, "0");
    printf("GPIO %s set to LOW\n", GPIO_PIN);
    
    // Unexport GPIO
    write_to_file("/sys/class/gpio/unexport", GPIO_PIN);
    
    return 0;
}