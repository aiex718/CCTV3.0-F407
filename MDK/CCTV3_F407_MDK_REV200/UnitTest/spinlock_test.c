
#include "stdatomic.h"
#define SPINLOCK_DEBUG 1

//override the atomic.h on pc platform
#if !defined(VSCODE_INTELLISENSE) && (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__) || defined(__linux__))
#define __ATOMIC_CM4_H__ 
#define __DSB()
#define __ISB()
#define __DMB()
#include "stdatomic.h"
#include "stdbool.h"
#define SPINLOCK_LOCK_VAL_TYPE atomic_int
#define Atomic_Exchange atomic_exchange
#endif

#include "bsp/sys/spinlock.h"

// Test code 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define THREAD_COUNT 10

SpinLock_t spinlock = SPINLOCK_UNLOCK_VALUE;

void* thread_func(void* arg) {
    int* thread_id = (int*)arg;
    printf("Thread %d is trying to acquire the lock...\n", *thread_id);
    SpinLock_Lock(&spinlock);
    printf("Thread %d has acquired the lock.\n", *thread_id);
    sleep(1);
    printf("Thread %d is releaseing the lock.\n", *thread_id);
    SpinLock_Unlock(&spinlock);
    printf("Thread %d has released the lock.\n", *thread_id);
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];

    // Initialize thread IDs
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i + 1;
    }

    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}



