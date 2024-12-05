/**
 * Compile with: gcc -o hello hello.c -pthread -ggdb
 *
 * This program has a utility function called print_string. It
 * appears there is a bug in it, however. If multiple threads call
 * the function, output to the terminal gets clobbered. For example,
 * if one thread calls the function with the string "Hello World"
 * and another thread calls the function with the string "Hi there!"
 * the output gets clobbered!
 *
 * Calls to print_string must be atomic. In other words, print_string
 * must be mutually exclusive. If one thread is currently outputting to the
 * terminal in print_string, all other threads must wait their turn before
 * outputting to the terminal using print_string.
 *
 * If the program is behaving correctly, then every line must be one of:
 * AAAAAAAAAA
 * BBBBBBBBBB
 * CCCCCCCCCC
 * DDDDDDDDDD
 *
 * You can run ./validate.sh <path to the compiled executable> to validate
 * that your program is behaving correctly.
 *
 * PLEASE FIX THIS PROGRAM BY ADDING THE NECESSARY SYNCHRONIZATION USING
 * pthread_mutex_t. YOU MUST ONLY MODIFY thread_function AND MAY NOT MODIFY
 * ANY OTHER PART OF THE PROGRAM. YOU MUST USE print_string_mutex to enforce
 * the synchronization.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t print_string_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
print_string(const char *string)
{
        size_t i;

        for (i = 0; i < strlen(string); i++)
                putchar(string[i]);
        putchar('\n');
        return;
}

static void *
thread_function(void *string)
{

        for (size_t i = 0; i < 1000; i++)
                print_string(string);

        return NULL;
}

#define ARRAY_LENGTH(X) (sizeof(X) / sizeof(*(X)))

int
main(void)
{
        pthread_t t[4];
        char *strings[] = { "AAAAAAAAAA", "BBBBBBBBBB", "CCCCCCCCCC", "DDDDDDDDDD" };

        for (size_t i = 0; i < ARRAY_LENGTH(t); i++) {
                if (pthread_create(&t[i], NULL, thread_function,
                    strings[i % ARRAY_LENGTH(strings)]) != 0) {
                        abort();
                }
        }

        for (size_t i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
                pthread_join(t[i], NULL);
        }

        return 0;
}
