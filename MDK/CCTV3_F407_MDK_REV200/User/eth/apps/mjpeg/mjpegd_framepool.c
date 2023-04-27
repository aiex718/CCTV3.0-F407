#include "eth/apps/mjpeg/mjpegd_framepool.h"
#include "eth/apps/mjpeg/mjpegd_memutils.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"


//TODO: make a linklist.h?
// #define LINKLIST_GET_UNTIL(p,list,end) do{\
//     for (p=list; p!=NULL && p->_next!=end; p=p->_next);\
// }while(0)
// #define LINKLIST_GET_LAST(p,list) LINKLIST_GET_UNTIL(p,list,NULL)

//Private functions
static void Mjpegd_FramePool_Sort(
    Mjpegd_FramePool_t* self,u8_t* order_out,MJPEGD_SYSTIME_T time);

void Mjpegd_FramePool_Init(Mjpegd_FramePool_t* self)
{
    u8_t i;

    for (i = 0; i < __NOT_CALLBACK_FRAMEPOOL_MAX; i++)
        self->FramePool_Callbacks[i] = NULL;

    for (i = 0; i < self->_frames_len; i++)
        Mjpegd_Frame_Init(&self->_frames[i]);
}

//try to clear framepool, return faile if any frame acquired by other
bool Mjpegd_FramePool_TryClear(Mjpegd_FramePool_t* self)
{
    u8_t i;
    Mjpegd_Frame_t *frame;
    for (i = 0; i < self->_frames_len; i++)
    {
        frame = &self->_frames[i];

        if(Mjpegd_Frame_TryLock(frame))
        {
            Mjpegd_Frame_Clear(frame);
            Mjpegd_Frame_Unlock(frame);
        }
        else
            return false;
    }
    return true;
}

void Mjpegd_FramePool_SetCallback(
    Mjpegd_FramePool_t* self, Mjpegd_FramePool_CallbackIdx_t cb_idx, 
    Mjpegd_Callback_t *callback)
{
    if(cb_idx < __NOT_CALLBACK_FRAMEPOOL_MAX)
        self->FramePool_Callbacks[cb_idx] = callback;
}

/**
 * @brief Get the latest frame newer than last_frame_time, 0 to get the newest frame.
 * @return Mjpegd_Frame_t* 
 */
Mjpegd_Frame_t* Mjpegd_FramePool_GetLatest(Mjpegd_FramePool_t* self,
    MJPEGD_SYSTIME_T last_frame_time)
{
    //vla on armcc alloc using malloc (heap)
    u8_t i,sorted_idx[self->_frames_len];
    MJPEGD_SYSTIME_T now = sys_now();
    Mjpegd_Frame_t* frame;
    //how much time has passed since last frame
    MJPEGD_SYSTIME_T last_frame_diff = now - last_frame_time,new_frame_diff;

    //Try to acquire newest frame.
    //Newest node should be the first node.
    Mjpegd_FramePool_Sort(self,sorted_idx,now);
    for (i = 0; i < self->_frames_len; i++)
    {
        frame = &self->_frames[sorted_idx[i]];
        if( Mjpegd_Frame_IsValid(frame) && frame->capture_time!=0 )
        {   
            //no last frame, just get the newest frame
            if(last_frame_time==0)
            {
                if(Mjpegd_Frame_TryAcquire(frame))
                    break;
            }
            else
            {
                //how much time has passed since this frame
                new_frame_diff = now - frame->capture_time;
                //if this frame is newer than last frame
                if(last_frame_diff>new_frame_diff)
                {
                    LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                        MJPEGD_DBG_ARG("TryAcquire frame %p ,_sem:%d\n",frame,frame->_sem));

                    if(Mjpegd_Frame_TryAcquire(frame))
                    {
                        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                            MJPEGD_DBG_ARG("Acquired frame %p ,_sem:%d\n",frame,frame->_sem));
                        break;
                    }
                }
                else
                {
                    //if newest frame is outdated, other frames are also outdated
                    //no need to check other frames
                    LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                        MJPEGD_DBG_ARG("all frame outdated\n"));
                    frame = NULL;
                    break;
                }
            }
        }
        frame = NULL;
    }
    return frame;
}

void Mjpegd_FramePool_Release(Mjpegd_FramePool_t* self,Mjpegd_Frame_t* frame)
{
    if(frame!=NULL)
    {
        Mjpegd_Frame_Release(frame);
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
            MJPEGD_DBG_ARG("release frame %p ,_sem:%d\n",frame,frame->_sem));

        if(frame->_sem>MJPEGD_FRAME_SEMAPHORE_MAX)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SEVERE , 
                MJPEGD_DBG_ARG("frame %p ->_sem=%d > MAX %d\n",
                    frame,frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX));
        }
    }
}

Mjpegd_Frame_t* Mjpegd_FramePool_GetIdle(Mjpegd_FramePool_t* self)
{
    u8_t i,sorted_idx[self->_frames_len];
    MJPEGD_SYSTIME_T now = sys_now();
    Mjpegd_Frame_t* frame;

    //Try to get and lock oldest idle node.
    //Oldest node should be the last node.
    Mjpegd_FramePool_Sort(self,sorted_idx,now);
    for (i = self->_frames_len; i != 0; i--)
    {
        frame = &self->_frames[sorted_idx[i-1]];

        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
            MJPEGD_DBG_ARG("trylock frame %p ,_sem:%d\n",frame,frame->_sem));

        if(Mjpegd_Frame_TryLock(frame))
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                MJPEGD_DBG_ARG("locked frame %p ,_sem:%d\n",frame,frame->_sem));
            return frame;
        }
    }

    LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS, 
        MJPEGD_DBG_ARG("no idle frame\n"));
    
    return NULL;
}

/**
 * @brief Return idle frame previous locked by calling Mjpegd_FramePool_GetIdle().
 */
void Mjpegd_FramePool_ReturnIdle(Mjpegd_FramePool_t* self,Mjpegd_Frame_t* frame)
{
    if(frame!=NULL)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
            MJPEGD_DBG_ARG("ReturnIdle frame %p ,_sem:%d\n",frame,frame->_sem));

        Mjpegd_Frame_Unlock(frame);
        Mjpegd_Callback_Invoke_Idx(self, frame, self->FramePool_Callbacks,
                FRAMEPOOL_CALLBACK_RX_NEWFRAME);

        if(frame->_sem>MJPEGD_FRAME_SEMAPHORE_MAX)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SEVERE , 
                MJPEGD_DBG_ARG("frame %p ->_sem=%d > MAX %d\n",
                    frame,frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX));
        }
    }
}

//sort by frame's time diff (capture_time-now),order from small to big(new to old)
static void Mjpegd_FramePool_Sort(Mjpegd_FramePool_t* self,u8_t* order_out,
    MJPEGD_SYSTIME_T time)
{
    //vla on armcc alloc using malloc (heap)
    MJPEGD_SYSTIME_T tmp,time_diffs[self->_frames_len];
    u8_t tmp2,i,j;
    for (i = 0; i < self->_frames_len; i++)
    {
        time_diffs[i] = time - self->_frames[i].capture_time;
        order_out[i] = i;
    }
#if MJPEGD_FRAMEPOOL_DEBUG_PRINT_SORT
    MJPEGD_DBG_PRINT("FramePool: [order,capture_time] before sort:");
    for (i = 0; i < self->_frames_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],self->_frames[i].capture_time);
    MJPEGD_DBG_PRINT(("\n"));

    MJPEGD_DBG_PRINT("FramePool: [order,time_diffs] before sort: ");
    for (i = 0; i < self->_frames_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    MJPEGD_DBG_PRINT("\n");
#endif
    //sort time_diffs and order_out
    for (i = 0; i < self->_frames_len; i++)
    {
        for (j = i+1; j < self->_frames_len; j++)
        {
            if(time_diffs[i] > time_diffs[j])
            {
                tmp = time_diffs[i];
                time_diffs[i] = time_diffs[j];
                time_diffs[j] = tmp;
                tmp2 = order_out[i];
                order_out[i] = order_out[j];
                order_out[j] = tmp2;
            }
        }
    }

#if MJPEGD_FRAMEPOOL_DEBUG_PRINT_SORT
    MJPEGD_DBG_PRINT("FramePool: [order,time_diffs] after sort: ");
    for (i = 0; i < self->_frames_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    MJPEGD_DBG_PRINT("\n");
#endif
}
