#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/random.h>

static void
usage(void)
{

        fprintf(stderr, "Usage: ./generator <output file> <number of integers>\n");
        exit(EXIT_FAILURE);
}

int
main(int argc, const char *argv[])
{
        FILE *fp;
        int count;

        if (argc != 3)
                usage();

        count = atoi(argv[2]);
        if (count <= 0) {
                fprintf(stderr, "error: number of integers must be > 0\n");
                exit(EXIT_FAILURE);
        }

        fp = fopen(argv[1], "a");

        for (int i = 0; i < count; i++) {
                uint32_t value;
                ssize_t r;

                r = getrandom(&value, sizeof(value), 0);
                if (r == -1) {
                 perror("warning: failed to generate random value");
                        i--;
                        continue;
                }

                if (fwrite(&value, sizeof(value), 1, fp) != 1) {
                        perror("error: failed to write value");
                        exit(EXIT_FAILURE);
                }
        }

        return 0;
}

