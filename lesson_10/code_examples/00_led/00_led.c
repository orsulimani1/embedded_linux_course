#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define LED_GPIO_PIN "49"  // USR0 LED on BeagleBone Black, can be passed as argument

int gpio_export(const char* pin) {
    int fd;
    
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return -1;
    }
    
    if (write(fd, pin, strlen(pin)) != strlen(pin)) {
        // If error is EBUSY, the GPIO is already exported
        if (errno != EBUSY) {
            perror("Failed to export GPIO");
            close(fd);
            return -1;
        }
    }
    
    close(fd);
    return 0;
}

int gpio_set_direction(const char* pin, const char* direction) {
    int fd;
    char path[64];
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", pin);
    
    // Wait a bit for the sysfs entries to be created
    usleep(100000);
    
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open direction file");
        return -1;
    }
    
    if (write(fd, direction, strlen(direction)) != strlen(direction)) {
        perror("Failed to set direction");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

int gpio_set_value(const char* pin, const char* value) {
    int fd;
    char path[64];
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
    
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open value file");
        return -1;
    }
    
    if (write(fd, value, strlen(value)) != strlen(value)) {
        perror("Failed to set value");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

int gpio_get_value(const char* pin) {
    int fd;
    char path[64];
    char value[2];
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
    
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open value file for reading");
        return -1;
    }
    
    if (read(fd, value, 2) < 0) {
        perror("Failed to read value");
        close(fd);
        return -1;
    }
    
    close(fd);
    return atoi(value);
}

void print_usage(const char* prog_name) {
    printf("Usage: %s [GPIO_PIN] [on|off|toggle|blink]\n", prog_name);
    printf("  GPIO_PIN: GPIO pin number (default: 49 for USR0 LED)\n");
    printf("  Commands:\n");
    printf("    on     - Turn the LED on\n");
    printf("    off    - Turn the LED off\n");
    printf("    toggle - Toggle the LED state\n");
    printf("    blink  - Blink the LED 5 times\n");
    printf("Example: %s 49 blink\n", prog_name);
}

int main(int argc, char* argv[]) {
    const char* pin = LED_GPIO_PIN;
    const char* cmd = "toggle";
    int ret, current_value;
    
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        pin = argv[1];
    }
    
    if (argc > 2) {
        cmd = argv[2];
    }
    
    printf("Using GPIO %s\n", pin);
    
    // Export the GPIO pin
    ret = gpio_export(pin);
    if (ret < 0) {
        return 1;
    }
    
    // Set direction to output
    ret = gpio_set_direction(pin, "out");
    if (ret < 0) {
        return 1;
    }
    
    if (strcmp(cmd, "on") == 0) {
        ret = gpio_set_value(pin, "1");
        printf("LED turned ON\n");
    }
    else if (strcmp(cmd, "off") == 0) {
        ret = gpio_set_value(pin, "0");
        printf("LED turned OFF\n");
    }
    else if (strcmp(cmd, "toggle") == 0) {
        current_value = gpio_get_value(pin);
        if (current_value < 0) {
            return 1;
        }
        
        if (current_value == 0) {
            ret = gpio_set_value(pin, "1");
            printf("LED turned ON\n");
        } else {
            ret = gpio_set_value(pin, "0");
            printf("LED turned OFF\n");
        }
    }
    else if (strcmp(cmd, "blink") == 0) {
        printf("Blinking LED 5 times\n");
        for (int i = 0; i < 5; i++) {
            ret = gpio_set_value(pin, "1");
            if (ret < 0) return 1;
            printf("LED ON\n");
            sleep(1);
            
            ret = gpio_set_value(pin, "0");
            if (ret < 0) return 1;
            printf("LED OFF\n");
            sleep(1);
        }
    }
    else {
        printf("Unknown command: %s\n", cmd);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}