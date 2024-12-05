#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include <sys/random.h>
#include <sys/stat.h>

#define GLOBAL_MINIMUM_LIMIT    10

static uint32_t global_minimum[GLOBAL_MINIMUM_LIMIT];
static bool global_minimum_used[GLOBAL_MINIMUM_LIMIT];

static void
usage(void)
{

        fprintf(stderr, "Usage: ./minimum <limit> <number of threads> <files ...>\n");
        exit(EXIT_FAILURE);
}

static void
minimum_print(const char *path, size_t start, size_t end)
{
        FILE *fp;
        uint32_t minimum = UINT32_MAX;
        size_t n_integers = end - start;

        fp = fopen(path, "r");
        if (fp == NULL) {
                perror("error: failed to read file");
                return;
        }
        if (fseek(fp, start, SEEK_SET) != 0) {
                perror("error: failed to set file location");
                exit(EXIT_FAILURE);
        }

        while (n_integers--) {
                uint32_t value;

                if (fread_unlocked(&value, sizeof(value), 1, fp) != 1) {
                        if (feof(fp) != 0)
                                break;

                        perror("error: failed to read entire file");
                        exit(EXIT_FAILURE);
                }

                if (value < minimum)
                        minimum = value;

                /* Insert into top N minimum. */
                 insert_into_global_minimum(...);
        }
        fclose(fp);

        printf("[%s] %" PRIu32 "\n", path, minimum);
        return;
}

static void *
thread_function(void *work_context)
{
        struct work *work = work_context;
        struct work *cursor;

        for (cursor = work; cursor != NULL; cursor = cursor->next) {
                minimum_print(cursor->file->path, cursor->start, cursor->end);
        }

        return NULL;
}
/*
 * XXX
 * An alternative to this is to have a global queue structure with fixed blocks
 * of data and have threads greedily process blocks of data from this.
 *
 * You'll want an efficient concurrent queue / list data structure and to learn
 * about "pthread condition variables" to utilize this.
 */

struct file {
        const char *path;
        size_t size;
        size_t claimed;
        struct file *next;
};

struct work {
        struct file *file;
        size_t start;
        size_t end;
        struct work *next;
};

/*
 * This functions will return a "struct work" linked list that
 * consumes up to total_bytes from the list of files provided
 * in the first argument.
 *
 * Multiple ways to achieve this. You need to save some state in
 * file elements themselves, you can use "claimed" to determine
 * whether a given file has already been completed claimed by
 * other threads.
 */
struct work *
work_unit_list_create(struct file *file, size_t total_bytes)
{
        struct work *work = NULL;

        return work;
}

struct thread {
        pthread_t thread;
        struct work *work;
};

int
main(int argc, const char *argv[])
{
        struct file *root = NULL;
        struct thread *threads;
        size_t total = 0;
        int limit, n_threads;
        size_t work_unit, work_slop;

        if (argc < 4)
                usage();

        limit = atoi(argv[1]);
        if (limit <= 0 || limit > GLOBAL_MINIMUM_LIMIT) {
                fprintf(stderr, "error: number of integers must be > 0\n");
                exit(EXIT_FAILURE);
        }

        n_threads = atoi(argv[2]);
        if (n_threads <= 0) {
                fprintf(stderr, "error: number of threads must be >= 1\n");
                exit(EXIT_FAILURE);
        }

        threads = calloc(sizeof(struct thread), n_threads);
        assert(threads != NULL);

#if 0
        threads = malloc(sizeof(pthread_t) * (argc - 1));
        if (threads == NULL) {
                perror("error: failed to allocate threads");
                exit(EXIT_FAILURE);
        }
#endif

        for (int i = 3; i < argc; i++) {
                struct stat st;
                struct file *file;

                if (stat(argv[i], &st) == -1) {
                        perror("error: failed to get file details");
                        exit(EXIT_FAILURE);
                }

                total += st.st_size;

                file = malloc(sizeof *file);
                assert(file != NULL);

                file->path = argv[i];
                file->size = st.st_size / 4;
                file->claimed = 0;
                 file->next = root;
                root = file;
        }

        fprintf(stderr, "Total size is: %zu integers\n", total);

        fprintf(stderr, "Every thread should process %zu integers\n",
            total / n_threads);

        work_unit = total / n_threads;

        fprintf(stderr, "Slop consists of %zu integers\n",
            total % n_threads);

        work_slop = total % n_threads;

        /* XXX: Create threads here. Every thread will have it's own list of work units. */
        /*
        for (int i = 0; i < n_threads; i++) {
                struct work *work = work_unit_list_create(root, work_unit);
                 pthread_create(&threads[i].thread, NULL, thread_function, work);
        }
        */

        /* XXX: Join threads. */

        /* XXX: Print out the global merged list of the 10 smallest integers
         *      across all files. */

        return 0;
}






