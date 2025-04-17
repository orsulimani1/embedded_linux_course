/*
* bbb_direct_gpio.c - Control GPIO directly through memory-mapped registers
*
* This example demonstrates how to:
* - Access GPIO registers directly through memory mapping
* - Configure pin multiplexing through direct register access
* - Control GPIO pins without using SYSFS
* - Create blinking patterns with precise timing
*
* Note: This requires root privileges to access /dev/mem
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

/* AM335x Memory Map Definitions */
#define GPIO0_BASE      0x44E07000
#define GPIO1_BASE      0x4804C000
#define GPIO2_BASE      0x481AC000
#define GPIO3_BASE      0x481AE000
#define PINMUX_BASE     0x44E10000

/* GPIO Register Offsets */
#define GPIO_OE         0x134   // Output Enable
#define GPIO_DATAIN     0x138   // Data Input
#define GPIO_DATAOUT    0x13C   // Data Output
#define GPIO_CLEARDATAOUT 0x190 // Clear Data Output
#define GPIO_SETDATAOUT 0x194   // Set Data Output

/* Pin Definition */
#define P9_12_GPIO_BANK   1       // GPIO1
#define P9_12_GPIO_PIN    28      // Pin 28
#define P9_12_GPIO_MASK   (1 << P9_12_GPIO_PIN)
#define P9_12_PINMUX_OFFSET 0x878 // Offset from PINMUX_BASE

/* GPIO modes */
#define GPIO_INPUT      1
#define GPIO_OUTPUT     0

/* Memory mapping properties */
#define MAP_SIZE 4096
#define MAP_MASK (MAP_SIZE - 1)

// Structure to hold mapped memory regions
typedef struct {
    int fd;                        // /dev/mem file descriptor
    void *gpio_map[4];             // Mapped GPIO regions (GPIO0-3)
    void *pinmux_map;              // Mapped PINMUX region
} MemoryMap;

// Function to map physical memory regions
int map_memory(MemoryMap *map) {
    map->fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (map->fd < 0) {
        perror("Failed to open /dev/mem");
        return -1;
    }
    
    // Map GPIO regions
    map->gpio_map[0] = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                            map->fd, GPIO0_BASE);
    map->gpio_map[1] = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                            map->fd, GPIO1_BASE);
    map->gpio_map[2] = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                            map->fd, GPIO2_BASE);
    map->gpio_map[3] = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                            map->fd, GPIO3_BASE);
    
    // Map PINMUX region
    map->pinmux_map = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                        map->fd, PINMUX_BASE);
    
    // Check for mapping errors
    if (map->gpio_map[0] == MAP_FAILED || map->gpio_map[1] == MAP_FAILED || 
        map->gpio_map[2] == MAP_FAILED || map->gpio_map[3] == MAP_FAILED ||
        map->pinmux_map == MAP_FAILED) {
        perror("Memory mapping failed");
        close(map->fd);
        return -1;
    }
    
    return 0;
}

// Function to unmap memory regions
void unmap_memory(MemoryMap *map) {
    munmap(map->gpio_map[0], MAP_SIZE);
    munmap(map->gpio_map[1], MAP_SIZE);
    munmap(map->gpio_map[2], MAP_SIZE);
    munmap(map->gpio_map[3], MAP_SIZE);
    munmap(map->pinmux_map, MAP_SIZE);
    close(map->fd);
}

// Function to configure pin multiplexing
void configure_pin(MemoryMap *map, uint32_t offset, uint32_t mode) {
    uint32_t *pinmux_addr = (uint32_t *)((uint8_t *)map->pinmux_map + (offset & MAP_MASK));
    uint32_t pinmux_value = *pinmux_addr;
    
    printf("Pin at offset 0x%X: Current configuration: 0x%08X\n", offset, pinmux_value);
    
    // Clear mux mode (bits 0-2) and set new mode
    pinmux_value &= ~0x07;
    pinmux_value |= mode;
    
    // Ensure input buffer is enabled (bit 5)
    pinmux_value |= (1 << 5);
    
    // Disable pull-up/down (bit 4)
    pinmux_value |= (1 << 4);
    
    printf("Pin at offset 0x%X: New configuration: 0x%08X\n", offset, pinmux_value);
    
    // Write new configuration
    *pinmux_addr = pinmux_value;
}

// Function to configure GPIO direction
void configure_gpio_direction(MemoryMap *map, int bank, int pin, int direction) {
    uint32_t *gpio_oe = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_OE);
    uint32_t reg_value = *gpio_oe;
    
    if (direction == GPIO_INPUT) {
        // For input, set the bit in OE register
        reg_value |= (1 << pin);
    } else {
        // For output, clear the bit in OE register
        reg_value &= ~(1 << pin);
    }
    
    *gpio_oe = reg_value;
}

// Function to set GPIO output value
void set_gpio_value(MemoryMap *map, int bank, int pin, int value) {
    if (value) {
        // Set the pin high
        uint32_t *gpio_set = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_SETDATAOUT);
        *gpio_set = (1 << pin);
    } else {
        // Set the pin low
        uint32_t *gpio_clear = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_CLEARDATAOUT);
        *gpio_clear = (1 << pin);
    }
}

// Function to read GPIO input value
int get_gpio_value(MemoryMap *map, int bank, int pin) {
    uint32_t *gpio_datain = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_DATAIN);
    return (*gpio_datain & (1 << pin)) ? 1 : 0;
}

// Function to read current GPIO output value
int get_gpio_output(MemoryMap *map, int bank, int pin) {
    uint32_t *gpio_dataout = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_DATAOUT);
    return (*gpio_dataout & (1 << pin)) ? 1 : 0;
}

// Function to toggle GPIO pin
void toggle_gpio(MemoryMap *map, int bank, int pin) {
    int current = get_gpio_output(map, bank, pin);
    set_gpio_value(map, bank, pin, !current);
}

// Function to create a blinking pattern with precise timing
void blink_pattern(MemoryMap *map, int bank, int pin, int count, int on_ms, int off_ms) {
    struct timespec on_time, off_time, rem;
    
    on_time.tv_sec = on_ms / 1000;
    on_time.tv_nsec = (on_ms % 1000) * 1000000;
    
    off_time.tv_sec = off_ms / 1000;
    off_time.tv_nsec = (off_ms % 1000) * 1000000;
    
    printf("Blinking GPIO%d_%d %d times (%dms on, %dms off)...\n", 
        bank, pin, count, on_ms, off_ms);
    
    for (int i = 0; i < count; i++) {
        // Turn on
        set_gpio_value(map, bank, pin, 1);
        printf("Cycle %d: ON\n", i + 1);
        
        // Sleep for on_time
        if (nanosleep(&on_time, &rem) == -1) {
            perror("nanosleep");
        }
        
        // Turn off
        set_gpio_value(map, bank, pin, 0);
        printf("Cycle %d: OFF\n", i + 1);
        
        // Sleep for off_time
        if (nanosleep(&off_time, &rem) == -1) {
            perror("nanosleep");
        }
    }
}

// Function to create SOS pattern (... --- ...)
void sos_pattern(MemoryMap *map, int bank, int pin) {
    printf("Generating SOS pattern on GPIO%d_%d...\n", bank, pin);
    
    // S: three short blinks
    for (int i = 0; i < 3; i++) {
        set_gpio_value(map, bank, pin, 1);
        usleep(200000);  // 200ms
        set_gpio_value(map, bank, pin, 0);
        usleep(200000);  // 200ms
    }
    
    // Pause between letters
    usleep(600000);  // 600ms
    
    // O: three long blinks
    for (int i = 0; i < 3; i++) {
        set_gpio_value(map, bank, pin, 1);
        usleep(600000);  // 600ms
        set_gpio_value(map, bank, pin, 0);
        usleep(200000);  // 200ms
    }
    
    // Pause between letters
    usleep(600000);  // 600ms
    
    // S: three short blinks
    for (int i = 0; i < 3; i++) {
        set_gpio_value(map, bank, pin, 1);
        usleep(200000);  // 200ms
        set_gpio_value(map, bank, pin, 0);
        usleep(200000);  // 200ms
    }
    
    // Final pause
    usleep(1000000);  // 1000ms
}

// Function to print GPIO register values
void print_gpio_registers(MemoryMap *map, int bank) {
    uint32_t *gpio_oe = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_OE);
    uint32_t *gpio_datain = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_DATAIN);
    uint32_t *gpio_dataout = (uint32_t *)((uint8_t *)map->gpio_map[bank] + GPIO_DATAOUT);
    
    printf("\n===== GPIO%d Register Values =====\n", bank);
    printf("GPIO_OE:      0x%08X\n", *gpio_oe);
    printf("GPIO_DATAIN:  0x%08X\n", *gpio_datain);
    printf("GPIO_DATAOUT: 0x%08X\n", *gpio_dataout);
}

int main(int argc, char *argv[]) {
    MemoryMap map;
    
    printf("===== BeagleBone Black Direct GPIO Control =====\n\n");
    
    // Check if running as root
    if (geteuid() != 0) {
        fprintf(stderr, "This program must be run as root to access /dev/mem\n");
        exit(EXIT_FAILURE);
    }
    
    // Map memory regions
    if (map_memory(&map) < 0) {
        exit(EXIT_FAILURE);
    }
    
    // Configure P9_12 pin for GPIO mode (mode 7)
    printf("Configuring P9_12 (GPIO1_28) for GPIO mode...\n");
    configure_pin(&map, P9_12_PINMUX_OFFSET, 7);  // 7 = GPIO mode
    
    // Print initial GPIO register values
    print_gpio_registers(&map, P9_12_GPIO_BANK);
    
    // Configure P9_12 as output
    printf("Configuring GPIO1_28 as output...\n");
    configure_gpio_direction(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN, GPIO_OUTPUT);
    
    // Print updated GPIO register values
    print_gpio_registers(&map, P9_12_GPIO_BANK);
    
    // Run different LED patterns
    
    // Pattern 1: Simple blink
    printf("\nPattern 1: Simple blink (5 cycles)...\n");
    blink_pattern(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN, 5, 500, 500);
    
    // Pattern 2: Fast blink
    printf("\nPattern 2: Fast blink (10 cycles)...\n");
    blink_pattern(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN, 10, 100, 100);
    
    // Pattern 3: Uneven blink
    printf("\nPattern 3: Uneven blink (long on, short off)...\n");
    blink_pattern(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN, 3, 800, 200);
    
    // Pattern 4: SOS pattern
    printf("\nPattern 4: SOS pattern (... --- ...)...\n");
    sos_pattern(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN);
    
    // Ensure LED is off before exiting
    set_gpio_value(&map, P9_12_GPIO_BANK, P9_12_GPIO_PIN, 0);
    
    // Unmap memory regions
    unmap_memory(&map);
    
    printf("\n===== Direct GPIO Control Example Completed =====\n");
    
    return 0;
}
 
