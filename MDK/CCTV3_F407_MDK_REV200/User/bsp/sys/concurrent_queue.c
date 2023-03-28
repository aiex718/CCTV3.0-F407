#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/atomic.h"
#include "concurrent_queue.h"

//check if x is power of 2
#define __CONQUEUE_IS_POWER_OF_2(x) ((x)>0&&((x)&((x)-1))==0)

//equal to x%d, but faster when d is power of 2
#define __CONQUEUE_MOD_FAST(x,d) (x&((d)-1))

void Concurrent_Queue_Init(__CONCURRENT_QUEUE_T *self, 
                            CONCURRENT_QUEUE_STORAGE_T* buf, 
                            CONCURRENT_QUEUE_STORAGE_LEN_T buf_len  )
{
    if(self==0 || buf ==0 || __CONQUEUE_IS_POWER_OF_2(buf_len)==0 )
        return;
    
    self->mem=buf;
    self->mem_len=buf_len;
    self->r_idx = self->w_idx = self->l_idx =0;
}

void Concurrent_Queue_Clear(__CONCURRENT_QUEUE_T *self)
{
    Atomic_EnterCritical();
    self->r_idx = self->w_idx = self->l_idx =0;
    Atomic_ExitCritical();
}

bool Concurrent_Queue_TryPush(__CONCURRENT_QUEUE_T *self, 
                                CONCURRENT_QUEUE_STORAGE_T val)
{
    __IO CONCURRENT_QUEUE_INDEX_T snap_w_idx,snap_r_idx;

    do
    {
        snap_w_idx = self->w_idx;
        snap_r_idx = self->r_idx;
        
        if (snap_w_idx-snap_r_idx>=self->mem_len)//is Full
            return false;
        else if(Atomic_Cmpxchg(CONCURRENT_QUEUE_INDEX_T,
                                &self->w_idx,(snap_w_idx+1),
                                snap_w_idx) ==snap_w_idx)
            break;
    } while (true);
    
    self->mem[__CONQUEUE_MOD_FAST(snap_w_idx,self->mem_len)] = val;

#if CONCURRENT_QUEUE_ALLOW_PREEMPT
    Atomic_Increment(CONCURRENT_QUEUE_INDEX_T,&self->l_idx);
#else
    do
    {
        if(Atomic_Cmpxchg(CONCURRENT_QUEUE_INDEX_T,
                            &self->l_idx,(snap_w_idx+1),
                            snap_w_idx)==snap_w_idx)
            break;
    } while (true);
#endif
    return true;
}

bool Concurrent_Queue_TryPop(__CONCURRENT_QUEUE_T *self,
                                CONCURRENT_QUEUE_STORAGE_T *out)
{
    __IO CONCURRENT_QUEUE_INDEX_T snap_r_idx,snap_l_idx,snap_w_idx;
    
    do
    {
        snap_r_idx = self->r_idx;
        snap_l_idx = self->l_idx;
        snap_w_idx = self->w_idx;

#if CONCURRENT_QUEUE_ALLOW_PREEMPT
        if ((snap_l_idx)!=(snap_w_idx))//queue is writing
            return false;
        else 
#endif
        if (snap_r_idx==snap_l_idx)//empty
            return false;
        else
        {
            *out = self->mem[__CONQUEUE_MOD_FAST(snap_r_idx,self->mem_len)];

            if (Atomic_Cmpxchg(CONCURRENT_QUEUE_INDEX_T,
                                &self->r_idx,(snap_r_idx+1),
                                snap_r_idx)==snap_r_idx)
                return true;
        }
    } while (true);
}
