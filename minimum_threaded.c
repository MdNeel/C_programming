#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include <sys/random.h>

#define GLOBAL_MINIMUM_LIMIT    10

static uint32_t global_minimum[GLOBAL_MINIMUM_LIMIT];
static bool global_minimum_used[GLOBAL_MINIMUM_LIMIT];

static void
usage(void)
{

        fprintf(stderr, "Usage: ./minimum <N> <files ...>\n");
        exit(EXIT_FAILURE);
}

static void
minimum_print(const char *path)
{
        FILE *fp;
        uint32_t minimum = UINT32_MAX;

        fp = fopen(path, "r");
        if (fp == NULL) {
                perror("error: failed to read file");
                return;
        }

        for (;;) {
                uint32_t value;
                 if (fread_unlocked(&value, sizeof(value), 1, fp) != 1) {
                        if (feof(fp) != 0)
                                break;

                        perror("error: failed to read entire file");
                        continue;
                }

                if (value < minimum)
                        minimum = value;

                /* Insert into top N minimum. */
        }
        fclose(fp);

        printf("[%s] %" PRIu32 "\n", path, minimum);
        return;
}

static void *
thread_function(void *path)
{

        minimum_print(path);
        return NULL;
}

int
main(int argc, const char *argv[])
{
        pthread_t *threads;
        int limit;

        if (argc < 3)
                usage();

        limit = atoi(argv[1]);
        if (limit <= 0 || limit > GLOBAL_MINIMUM_LIMIT) {
                fprintf(stderr, "error: number of integers must be > 0\n");
                exit(EXIT_FAILURE);
        }

        threads = malloc(sizeof(pthread_t) * (argc - 1));
        if (threads == NULL) {
                perror("error: failed to allocate threads");
                exit(EXIT_FAILURE);
        }

        for (int i = 2; i < argc; i++) {
                if (pthread_create(&threads[i - 2], NULL, thread_function,
                    (void *)argv[i]) != 0) {
                        perror("error: failed to create a thread");
                        exit(EXIT_FAILURE);
                }
        }

        for (int i = 0; i < argc - 2; i++) {
                if (pthread_join(threads[i], NULL) != 0)
                        perror("warning: failed to join a thread");
        }
        
        return 0;
}




