#include "bsp/sys/concurrent_queue.h"
#include "bsp/sys/atomic.h"
#include "concurrent_queue.h"

//check if x is power of 2
#define __CONQUEUE_IS_POWER_OF_2(x) ((x)>0&&((x)&((x)-1))==0)

//equal to x%d, but faster when d is power of 2
#define __CONQUEUE_MOD_FAST(x,d) (x&((d)-1))

/*
If task A is preempted by another task B during pushing data,

  - With preempt disabled:
    Task B can't push data to queue directly, it'll have to wait 
    until task A finish pushing.
    
    Pop from queue is always allowed.

  - With preempt enabled:
    Task B can push data to queue directly without waiting task A.
    This mechanism allow task B to be a ISR, and can boost 
    performance in scenarios with multiple push tasks.

    But when preempt is enabled, it's not allowed to pop from 
    queue until all data is pushed. 
    We have to let task A finish pushing before we can pop any data.
    i.e. We can preempt a push task, but we can't starve it.
         It'll cause queue pop fail during starved time.

Conclusion:
    In single-consumer scenario, 
    enable preempt has better performance.
    
    In multi-consumer scenario, 
    disable preempt has better performance.
*/
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
        else if(Atomic_Cmpxchg_Type(CONCURRENT_QUEUE_INDEX_T,
                                &self->w_idx,(snap_w_idx+1),
                                snap_w_idx) ==snap_w_idx)
            break;
    } while (true);
    
    self->mem[__CONQUEUE_MOD_FAST(snap_w_idx,self->mem_len)] = val;

    if(self->allow_push_preempt)
        Atomic_Increment_Type(CONCURRENT_QUEUE_INDEX_T,&self->l_idx);
    else
    {
        do
        {
            if(Atomic_Cmpxchg_Type(CONCURRENT_QUEUE_INDEX_T,
                                &self->l_idx,(snap_w_idx+1),
                                snap_w_idx)==snap_w_idx)
                break;
        } while (true);
    }
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

        if (self->allow_push_preempt && (snap_l_idx)!=(snap_w_idx))//queue is writing
            return false;
        else if (snap_r_idx==snap_l_idx)//empty
            return false;
        else
        {
            *out = self->mem[__CONQUEUE_MOD_FAST(snap_r_idx,self->mem_len)];

            if (Atomic_Cmpxchg_Type(CONCURRENT_QUEUE_INDEX_T,
                                &self->r_idx,(snap_r_idx+1),
                                snap_r_idx)==snap_r_idx)
                return true;
        }
    } while (true);
}
