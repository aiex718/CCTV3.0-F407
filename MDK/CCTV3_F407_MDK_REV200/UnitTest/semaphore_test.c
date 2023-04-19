#define SEMAPHORE_DEBUG 1
//override the atomic.h on pc platform
#if !defined(VSCODE_INTELLISENSE) && (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__))
#define __ATOMIC_CM4_H__ 
#define __DSB()
#define __ISB()
#define __DMB()
#include "stdatomic.h"
#include "stdbool.h"
#define SEMAPHORE_VAL_TYPE atomic_int
#define Atomic_Cmpxchg atomic_compare_exchange_weak
#define Atomic_Add atomic_fetch_add
#endif

#include "bsp/sys/semaphore.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 7
#define MAX_RESOURCES 3


Semaphore_t sem;

void *resource_task(void *arg)
{
    int id = *(int *)arg;
    // randomly allocate and release resources
    printf("Thread %d requesting...\n", id);
    while(Semaphore_TryDown(&sem) == false) {
        printf("Thread %d failed to acquire, retrying...\n", id);
        sleep(1);
    }
    printf("Thread %d acquired\n", id);
    sleep(rand() % 3);
    printf("Thread %d releasing...\n", id);
    Semaphore_Up(&sem);
    printf("Thread %d released\n", id);

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // initialize semaphore with maximum resources
    Semaphore_Init(&sem, MAX_RESOURCES);

    // create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, resource_task, &thread_ids[i])) {
            printf("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL)) {
            printf("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    printf("Finished, Semaphore:%d\n",sem);

    return 0;
}