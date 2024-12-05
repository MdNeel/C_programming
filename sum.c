/**
 * Compile with gcc -o sum sum.c -pthread
 *
 * This program times how long it takes to sum a large array
 * of randomly generated integers first with 1 thread and then
 * with 4 threads. The expectation is for 4 threads to generally
 * be a lot faster.
 *
 * Expected behavior:
 * $ ./sum
 * Generating random array...done
 * Summing integers (serially)...done (59287 microseconds)
 * Summing integers (4 threads)...done (14839 microseconds)
 *
 * We can see above that 4 threads doing the sum took 14839 microseconds
 * while it took one thread 59287 microseconds. However, something is now
 * broken in the program after adding locking. It looks like 4 threads is
 * just as fast or slower than the serial version.
 *
 * Current broken behavior:
 * $ ./sum
 * Generating random array...done
 * Summing integers (serially)...done (59556 microseconds)
 * Summing integers (4 threads)...done (59698 microseconds)
 *
 * Modify the locking being done in the program so that the program
 * is still correct but is able to realize performance improvement by
 * doing the sum across multiple threads. The problem is unnecessary
 * synchronization leading to long critical sections.
 *
 * LEAVE A COMMENT IN sum_thread WITH DETAILS ON WHAT YOU FIXED AND WHY
 * YOUR CHANGES DO NOT BREAK THE CORRECTNESS OF THE PROGRAM.
 */

#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N_THREADS               4
#define RANDOM_LENGTH   (16000000)
#define ITERATIONS      100

static unsigned int *
random_unsigned(size_t n)
{
        unsigned int *r;
        size_t i;

        r = malloc(sizeof(unsigned int) * n);
        if (r == NULL)
                return NULL;

        for (i = 0; i < n; i++)
                r[i] = (unsigned int)lrand48();

        return r;
}

static unsigned long long
sum_large_naive(unsigned int *v, size_t n)
{
        size_t i;
        unsigned long long s;

        for (i = 0; i < n; i++)
                s += v[i];

        return s;
}

static unsigned long long
elapsed_us(struct timespec *a, struct timespec *b)
{
        unsigned long long a_p = (a->tv_sec * 1000000ULL) + a->tv_nsec / 1000;
        unsigned long long b_p = (b->tv_sec * 1000000ULL) + b->tv_nsec / 1000;

        return b_p - a_p;
}

static unsigned long long total = 0;
static pthread_mutex_t mutex;

void *
sum_thread(void *v)
{
        unsigned int *array = v;
        unsigned long long sum;
        size_t i;

        pthread_mutex_lock(&mutex);
        for (i = 0; i < ITERATIONS; i++) {
                sum = sum_large_naive(array, RANDOM_LENGTH / N_THREADS);
        }

        total += sum;
       pthread_mutex_unlock(&mutex);
        return NULL;
}

int
main(void)
{
        struct timespec start, stop;
        unsigned long long sum_time, sum;
        time_t n = time(NULL);
        unsigned int *array;
        size_t i;
        pthread_t *threads;

        srand48((long int)n);

        fprintf(stderr, "Generating random array...");
        array = random_unsigned(RANDOM_LENGTH);
        if (array == NULL)
                abort();
                   fprintf(stderr, "done\n");

        fprintf(stderr, "Summing integers (serially)...");
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (i = 0; i < ITERATIONS; i++) {
                sum = sum_large_naive(array, RANDOM_LENGTH);
        }
        clock_gettime(CLOCK_MONOTONIC, &stop);
        fprintf(stderr, "done (%llu microseconds)\n",
            elapsed_us(&start, &stop) / ITERATIONS);

        pthread_mutex_init(&mutex, NULL);

        threads = malloc(sizeof(*threads) * N_THREADS);
        assert(threads != NULL);

        fprintf(stderr, "Summing integers (4 threads)...");
        clock_gettime(CLOCK_MONOTONIC, &start);

        for (i = 0; i < N_THREADS; i++) {
 pthread_create(&threads[i], NULL,
                    sum_thread, &array[RANDOM_LENGTH / N_THREADS * i]);
        }

        for (i = 0; i < N_THREADS; i++)
                pthread_join(threads[i], NULL);

        clock_gettime(CLOCK_MONOTONIC, &stop);
        fprintf(stderr, "done (%llu microseconds)\n",
            elapsed_us(&start, &stop) / ITERATIONS);
        return 0;
}
