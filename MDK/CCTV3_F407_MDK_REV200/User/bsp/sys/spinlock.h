#ifndef SPINLOCK_H
#define SPINLOCK_H

//TODO:Add to bsp
#include "bsp/platform/platform_defs.h"
#include "bsp/sys/atomic.h"

#ifndef SPINLOCK_DEBUG
    #define SPINLOCK_DEBUG 0
#endif

#ifndef SPINLOCK_LOCK_VAL_TYPE 
    #define SPINLOCK_LOCK_VAL_TYPE uint8_t
#endif

#ifndef SPINLOCK_LOCK_VALUE
    #define SPINLOCK_LOCK_VALUE 0x01
#endif

#ifndef SPINLOCK_UNLOCK_VALUE
    #define SPINLOCK_UNLOCK_VALUE 0x00
#endif

typedef __IO SPINLOCK_LOCK_VAL_TYPE SpinLock_t;

/**
 * @brief Unlock a spinlock
 * @param lock Pointer to spinlock
 * @warning This function will not check if the spinlock is locked.
 * @note This operation never failed.
 */
__STATIC_INLINE void SpinLock_Unlock(SpinLock_t *sp_lock)
{
#if SPINLOCK_DEBUG
    if(*sp_lock!=SPINLOCK_LOCK_VALUE)
        printf("SpinLock_Unlock: lock is not locked\n");
#endif
    *sp_lock=SPINLOCK_UNLOCK_VALUE; 
    __DMB();
}

__STATIC_INLINE void SpinLock_Init(SpinLock_t *sp_lock)
{
    SpinLock_Unlock(sp_lock);
}

/**
 * @brief Try to lock a spinlock
 * @param lock Pointer to spinlock
 * @return true if lock was acquired, false if lock was already locked
 */
__STATIC_INLINE bool SpinLock_TryLock(SpinLock_t *sp_lock)
{
    return Atomic_Exchange(sp_lock,SPINLOCK_LOCK_VALUE)
        ==SPINLOCK_UNLOCK_VALUE;
}

/**
 * @brief Lock a spinlock
 * @param lock Pointer to spinlock
 * @note This function will block until the spinlock is acquired,
 * @note This operation never failed.
 */
__STATIC_INLINE void SpinLock_Lock(SpinLock_t *sp_lock)
{
    while(Atomic_Exchange(sp_lock,SPINLOCK_LOCK_VALUE)
        ==SPINLOCK_LOCK_VALUE);
}

#endif
