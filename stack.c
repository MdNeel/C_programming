/*
 * Compile this program with gcc -o stack stack.c -pthread -ggdb
 *
 * This program will create NTHREAD number of threads, each of which
 * will push the same number of items on the stack. The program is broken
 * though. When counting the number of items in the length, it does not
 * add up to NTHREAD * STACK_PUSH number of nodes.
 *
 * Current behavior:
 * Expecting a stack length of 8000000, and got a length of 3759402.
 * THE STACK IS INCORRECT.
 *
 * Expected behavior:
 * Expecting a stack length of 8000000, and got a length of 8000000.
 * THE STACK IS CORRECT.
 *
 * PLEASE FIX THIS PROGRAM SO IT HAS THE EXPECTED BEHAVIOR. YOU MUST DO
 * THIS BY ADDING A MUTEX TO `struct stack` ITSELF. stack_push and stack_pop
 * MUST USE THE MUTEX TO LOCK MUTATIONS OT THE STACK ITSELF FOR CORRECTNESS.
 * YOU CAN ONLY MODIFY "struct stack", stack_push AND stack_pop. IN stack_init
 * USE THE pthread_mutex_init TO INITIALIZE THE MUTEX. FOR EXAMPLE,
 * pthread_mutex_init(&stack->mutex, NULL);
 */

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define STACK_PUSH      (1000000)
#define NTHREAD 8

struct thread_input {
        int tid;
        struct stack *stack;
};

struct node {
        int value;
        int tid;
        struct node *next;
};

struct stack {
        struct node *head;
};

void
stack_init(struct stack *stack)
{

        stack->head = NULL;
        return;
}

void
stack_push(struct stack *stack, int tid, int value)
{
        struct node *node;

        node = malloc(sizeof *node);
        assert(node != NULL);
        node->value = value;
        node->tid = tid;
        node->next = stack->head;
        stack->head = node;
        return;
}

bool
stack_pop(struct stack *stack, int *output, int *tid)
{
        struct node *node;

        if (stack->head == NULL) {
                return false;
        }
        node = stack->head;
        *output = node->value;
        *tid = node->tid;
        stack->head = node->next;

        free(node);
        return true;
}

static void
test_function(struct thread_input *input)
{
        int i;

        for (int i = 0; i < STACK_PUSH; i++) {
                stack_push(input->stack, input->tid, i);
        }

        return;
}
static void *
thread_function(void *sp)
{
        struct thread_input *input = sp;

        test_function(input);
        return NULL;
}

static int
stack_length(struct stack *stack)
{
        int r = 0;

        for (struct node *cursor = stack->head; cursor != NULL; cursor = cursor->next)
                r++;

        return r;
}

int
main(void)
{
        struct stack stack;
        pthread_t threads[NTHREAD];
        struct thread_input input[NTHREAD];

        stack_init(&stack);

        /*
         * Create the threads but initialize the input structure first so every thread
         * gets the pointer to the same stack as well as a unique id, which we will use
         * for validation.
         */
        for (size_t i = 0; i < NTHREAD; i++) {
                input[i].tid = i;
                input[i].stack = &stack;

                if (pthread_create(&threads[i], NULL, thread_function, &input[i]) != 0)
                 abort();
        }

        /* Wait for threads to complete. */
        for (size_t i = 0; i < NTHREAD; i++)
                pthread_join(threads[i], NULL);

        /* Compare the length of the stack to the expected length. */
        fprintf(stderr, "Expecting a stack length of %d, and got a length of %d.\n",
            NTHREAD * STACK_PUSH, stack_length(&stack));

        if (stack_length(&stack) != NTHREAD * STACK_PUSH) {
                puts("THE STACK IS INCORRECT.");
        } else {
                puts("THE STACK IS CORRECT.");
        }
        return 0;
}

