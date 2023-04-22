#ifndef __MJPEGD_SEMAPHORE_H__
#define __MJPEGD_SEMAPHORE_H__

#include "bsp/sys/semaphore.h"

typedef Semaphore_t Mjpegd_Semaphore_t;

__STATIC_INLINE void Mjpegd_Semaphore_Init(Mjpegd_Semaphore_t *sem,u8_t val)
{
    Semaphore_Init(sem,val);
}

__STATIC_INLINE u8_t Mjpegd_Semaphore_TryDown(Mjpegd_Semaphore_t *sem)
{
    return Semaphore_TryDown(sem);
}

__STATIC_INLINE void Mjpegd_Semaphore_Up(Mjpegd_Semaphore_t *sem)
{
    Semaphore_Up(sem);
}

__STATIC_INLINE u8_t Mjpegd_Semaphore_TryDownMulti(Mjpegd_Semaphore_t *sem,u8_t val)
{
    return Semaphore_TryDownMulti(sem,val);
}

__STATIC_INLINE void Mjpegd_Semaphore_UpMulti(Mjpegd_Semaphore_t *sem,u8_t val)
{
    Semaphore_UpMulti(sem,val);
}



#endif
