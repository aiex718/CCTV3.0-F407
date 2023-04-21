#ifndef __MJPEGD_SEMAPHORE_H__
#define __MJPEGD_SEMAPHORE_H__

#include "bsp/sys/semaphore.h"

#define Mjpegd_Semaphore_t Semaphore_t

#define Mjpegd_Semaphore_Init(sem,val) Semaphore_Init(sem,val)
#define Mjpegd_Semaphore_TryDown(sem) Semaphore_TryDown(sem)
#define Mjpegd_Semaphore_Up(sem) Semaphore_Up(sem);
#define Mjpegd_Semaphore_TryDownMulti(sem,val) Semaphore_TryDownMulti(sem,val)
#define Mjpegd_Semaphore_UpMulti(sem,val) Semaphore_UpMulti(sem,val)


#endif
