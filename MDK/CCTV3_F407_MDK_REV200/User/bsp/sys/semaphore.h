#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "bsp/platform/platform_defs.h"
#include "bsp/sys/atomic.h"

#ifdef atomic_init 
    #warning "using stdatomic.h"
#endif

#ifndef SEMAPHORE_VAL_TYPE
    #define SEMAPHORE_VAL_TYPE uint8_t
#endif

#ifndef CMPXCHG_RETURN_BOOL
    #define CMPXCHG_RETURN_BOOL 0
#endif

typedef __IO SEMAPHORE_VAL_TYPE Semaphore_t;

__STATIC_INLINE void Semaphore_Init(Semaphore_t* sem, SEMAPHORE_VAL_TYPE cnt)
{
    *sem = cnt;
}

__STATIC_INLINE bool Semaphore_TryDownMulti(Semaphore_t *sem,SEMAPHORE_VAL_TYPE num)
{
    SEMAPHORE_VAL_TYPE snap;

    snap = *sem; 
    if (snap < num) 
        return false;//no enough resource
#ifdef atomic_init //compatibility for stdatomic.h
    return Atomic_Cmpxchg(sem,&snap,snap - num);
#else
    return Atomic_Cmpxchg(sem,snap - num,snap)==snap;
#endif
    
}

__STATIC_INLINE void Semaphore_DownMulti(Semaphore_t *sem,SEMAPHORE_VAL_TYPE num)
{
    SEMAPHORE_VAL_TYPE snap;
    do {
        do{snap = *sem;} 
        while (snap < num);
    } while (
#ifdef atomic_init //compatibility for stdatomic.h
    Atomic_Cmpxchg(sem,&snap,snap - num)==false
#else
    Atomic_Cmpxchg(sem,snap - num,snap)!= snap
#endif
        );
}

__STATIC_INLINE bool Semaphore_TryDown(Semaphore_t *sem)
{
    return Semaphore_TryDownMulti(sem,1);
}

__STATIC_INLINE void Semaphore_Down(Semaphore_t *sem)
{
    Semaphore_DownMulti(sem,1);
}

__STATIC_INLINE void Semaphore_UpMulti(Semaphore_t* sem,SEMAPHORE_VAL_TYPE num)
{
    Atomic_Add(sem,num);
}

__STATIC_INLINE void Semaphore_Up(Semaphore_t* sem)
{
    Semaphore_UpMulti(sem,1);
}

#endif
