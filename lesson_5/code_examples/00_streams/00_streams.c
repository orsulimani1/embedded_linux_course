#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>   // Includes prototype for strerror()

#define NS_PER_SEC 1000000000L
#define INPUT_FILE "../../../lesson_5/code_examples/00_streams/in_the_air_tonight"
#define OUTPUT_FILE_BFD "../../../lesson_5/code_examples/00_streams/in_the_air_tonight_out_bfd"
#define OUTPUT_FILE_UBFD "../../../lesson_5/code_examples/00_streams/in_the_air_tonight_out_ubfd"


#define NUM_ITERATIONS 1000



// Helper function to compute elapsed time in seconds
uint64_t time_diff_in_ns(struct timespec start, struct timespec end) {
    uint64_t start_ns = (uint64_t)start.tv_sec * NS_PER_SEC + (uint64_t)start.tv_nsec;
    uint64_t end_ns   = (uint64_t)end.tv_sec   * NS_PER_SEC + (uint64_t)end.tv_nsec;
    return end_ns - start_ns;
}

int main(int argc, char *argv[]) {

    const char *input_path = INPUT_FILE;
    const char *output_path = OUTPUT_FILE_BFD;
    // -------------------------------------------------------------
    // 1. Get the filesystem block size for the input file
    // -------------------------------------------------------------
    struct stat sb;
    if (stat(input_path, &sb) == -1) {
        perror("stat");
        return 1;
    }

    // Depending on your needs, you can also use statfs() or a fallback:
    // struct statfs fsb;
    // statfs(input_path, &fsb);
    // long block_size = (long)fsb.f_bsize;

    long block_size = sb.st_blksize; // Typical approach

    printf("Detected block size: %ld bytes\n", block_size);
    
    // we will use 3/4 of the block size to demostrate the buffered operations
    long chunk_size = (3 * block_size) / 4;
    printf("Using chunk size: %ld bytes\n", chunk_size);
    
    // -------------------------------------------------------------
    // 2. Timed Copy (Buffered I/O)
    // -------------------------------------------------------------
    FILE *fin = fopen(input_path, "rb");
    if (!fin) {
        perror("fopen(input)");
        return 1;
    }

    FILE *fout = fopen(output_path, "wb");
    if (!fout) {
        perror("fopen(output)");
        fclose(fin);
        return 1;
    }
    char *buffer = malloc(chunk_size);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer of size %ld\n", chunk_size);
        fclose(fin);
        fclose(fout);
        return 1;
    }

    // Start timing
    struct timespec start_b, end_b;
    clock_gettime(CLOCK_MONOTONIC, &start_b);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Go back to the start of the input file
        fseek(fin, 0, SEEK_SET);
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, chunk_size, fin)) > 0) {
            ssize_t bytes_written = fwrite(buffer, 1, bytes_read, fout);
            if (bytes_written < 0) {
                perror("write");
                free(buffer);
                fclose(fin);
                fclose(fout);
                return 1;
            }
        }

        if (bytes_read < 0) {
            // Error on read
            perror("read");
        }
    }
    // Flush to ensure all data is written
    fflush(fout);

    clock_gettime(CLOCK_MONOTONIC, &end_b);
    uint64_t buffered_time = time_diff_in_ns(start_b, end_b);

    fclose(fin);
    fclose(fout);

    // -------------------------------------------------------------
    // 3. Timed Copy (Unbuffered I/O)
    // -------------------------------------------------------------
    // We'll create a different output file to avoid overwriting.
    // Or you can reopen the same output file if desired.
    const char *output_path_unbuf = OUTPUT_FILE_UBFD;

    int fd_in = open(input_path, O_RDONLY);
    if (fd_in < 0) {
        perror("open(input)");
        return 1;
    }

    int fd_out = open(output_path_unbuf, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        fprintf(stderr, "open(%s),  %s\n",output_path_unbuf, strerror(errno));
        close(fd_in);
        return 1;
    }

    // Reuse the same block_size for fairness
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer of size %ld\n", block_size);
        close(fd_in);
        close(fd_out);
        return 1;
    }

    // Start timing
    struct timespec start_u, end_u;
    clock_gettime(CLOCK_MONOTONIC, &start_u);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Go back to the start of the input file
        lseek(fd_in, 0, SEEK_SET);

        // Read and write loop
        ssize_t raw_bytes_read;
        while ((raw_bytes_read = read(fd_in, buffer, chunk_size)) > 0) {
            // Could handle partial writes if needed, but for simplicity:
            ssize_t raw_bytes_written = write(fd_out, buffer, raw_bytes_read);
            if (raw_bytes_written < 0) {
                perror("write");
                free(buffer);
                close(fd_in);
                close(fd_out);
                return 1;
            }
        }

        if (raw_bytes_read < 0) {
            // Error on read
            perror("read");
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end_u);
    uint64_t unbuffered_time = time_diff_in_ns(start_u, end_u);

    close(fd_in);
    close(fd_out);
    free(buffer);

    // -------------------------------------------------------------
    // 4. Compare Results
    // -------------------------------------------------------------
    printf("\n=== Performance Results ===\n");
    printf("Buffered I/O  : %ld nano seconds\n", buffered_time);
    printf("Unbuffered I/O: %ld nano seconds\n", unbuffered_time);
    
    return 0;
}
