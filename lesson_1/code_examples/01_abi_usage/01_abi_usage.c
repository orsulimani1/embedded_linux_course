#include <unistd.h>

int main() {
    // write() is a system call that follows the ABI for system calls
    write(1, "Hello, ABI!\n", 12);  // 1 = stdout, 12 = length of the string
    return 0;
}