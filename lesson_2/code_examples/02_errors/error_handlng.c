#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(void) {

    FILE *fp = fopen("non_existent_file.txt", "r");
    if (!fp) {


        perror("fopen");


        fprintf(stderr, "Using strerror: %s\n", strerror(errno));


        char buf[256];
        char *msg = strerror_r(errno, buf, sizeof(buf));
        fprintf(stderr, "Using strerror_r (GNU): %s\n", msg);

        exit(EXIT_FAILURE);
    }

    fclose(fp);
    return 0;
}
