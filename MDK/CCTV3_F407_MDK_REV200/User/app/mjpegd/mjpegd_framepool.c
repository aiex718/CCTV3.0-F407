#include "app/mjpegd/mjpegd_framepool.h"
#include "app/mjpegd/mjpegd_memutils.h"
#include "app/mjpegd/trycatch.h"
#include "app/mjpegd/mjpegd_debug.h"


//TODO: make a linklist.h?
// #define LINKLIST_GET_UNTIL(p,list,end) do{\
//     for (p=list; p!=NULL && p->_next!=end; p=p->_next);\
// }while(0)
// #define LINKLIST_GET_LAST(p,list) LINKLIST_GET_UNTIL(p,list,NULL)

//Private functions
static bool Mjpegd_FramePool_Sort(
    Mjpegd_FramePool_t* self,u8_t* order_out,MJPEGD_SYSTIME_T time);

void Mjpegd_FramePool_Init(Mjpegd_FramePool_t* self)
{
    u8_t i;

    for (i = 0; i < __NOT_CALLBACK_FRAMEPOOL_MAX; i++)
        self->FramePool_Callbacks[i] = NULL;

    for (i = 0; i < self->_frames_len; i++)
        Mjpegd_Frame_Init(&self->_frames[i]);
}

//try to clear framepool, return false if any frame locked or acquired by others
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
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_WARNING , 
                        MJPEGD_DBG_ARG("Clear failed at frame 0x%p\n",frame));
            return false;
        }
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
 * @brief Get the latest frame newer than last_frame_time (0 to get the newest frame).
 * @return Mjpegd_Frame_t* 
 */
Mjpegd_Frame_t* Mjpegd_FramePool_GetLatest(Mjpegd_FramePool_t* self,
    MJPEGD_SYSTIME_T last_frame_time)
{
    u8_t i, *sorted_idx=NULL;
    MJPEGD_SYSTIME_T now = sys_now(),last_frame_diff;
    Mjpegd_Frame_t *frame_out=NULL, *temp_frame;

    try
    {
        //alloc mem for sorted_idx
        sorted_idx = BSP_MALLOC(self->_frames_len*sizeof(*sorted_idx));
        throwif(sorted_idx==NULL,MALLOC_FAILED);

        throwif(Mjpegd_FramePool_Sort(self,sorted_idx,now)==false,SORT_FAILED);

        //how much time has passed since last frame
        last_frame_diff = now - last_frame_time;

        //Try to acquire newest frame.
        //Newest frame should be the first frame.
        for (i = 0; i < self->_frames_len; i++)
        {
            temp_frame = &self->_frames[sorted_idx[i]];

            //check if frame is valid
            if(Mjpegd_Frame_IsValid(temp_frame)==false || temp_frame->capture_time==0 )
                continue;

            //no last frame, just get the newest frame
            //snap client use this route
            if(last_frame_time==0)
            {
                if(Mjpegd_Frame_TryAcquire(temp_frame))
                {
                    frame_out = temp_frame;
                    break;
                }
            }
            else
            {
                //if this frame is newer than last frame
                if(last_frame_diff> now - temp_frame->capture_time)
                {
                    LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                        MJPEGD_DBG_ARG("TryAcquire frame %p ,_sem:%d\n",
                            temp_frame,temp_frame->_sem));

                    if(Mjpegd_Frame_TryAcquire(temp_frame))
                    {
                        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                            MJPEGD_DBG_ARG("Acquired frame %p ,_sem:%d\n",
                                temp_frame,temp_frame->_sem));

                        frame_out = temp_frame;
                        break;
                    }
                }
                else
                {
                    //if newest frame is outdated, other frames are also outdated
                    //no need to check other frames
                    LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                        MJPEGD_DBG_ARG("all frame outdated\n"));
                    frame_out = NULL;
                    break;
                }
            }
        }
    }
    catch(MALLOC_FAILED)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("GetLatest malloc failed for sorted_idx[]\n"));
        frame_out = NULL;
    }
    catch(SORT_FAILED)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("GetLatest sort failed\n"));
        frame_out = NULL;
    }
    finally
    {
        if(frame_out==NULL)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                MJPEGD_DBG_ARG("GetLatest frame failed\n"));
        }

        if(sorted_idx!=NULL)
            BSP_FREE(sorted_idx);

        return frame_out;
    }
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
    u8_t i,*sorted_idx=NULL;
    u16_t lock_cnt;
    MJPEGD_SYSTIME_T now = sys_now();
    Mjpegd_Frame_t *frame_out=NULL,*frame_temp;
    try
    {
        //frame pool should only have 1 locked pending frame
        //if more than 1 frame locked, we refuse to get new idle frame
        lock_cnt = Mjpegd_FramePool_GetLockedFrameCount(self);
        throwif(lock_cnt>1,MULTI_FRAME_LOCKED);

        //alloc mem for sorted_idx
        sorted_idx = BSP_MALLOC(self->_frames_len*sizeof(*sorted_idx));
        throwif(sorted_idx==NULL,MALLOC_FAILED);

        throwif(Mjpegd_FramePool_Sort(self,sorted_idx,now)==false,SORT_FAILED);

        //Try to get and lock oldest idle frame.
        //Oldest frame should be the last frame.
        for (i = self->_frames_len; i != 0; i--)
        {
            frame_temp = &self->_frames[sorted_idx[i-1]];

            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                MJPEGD_DBG_ARG("trylock frame %p ,_sem:%d\n",frame_temp,frame_temp->_sem));

            if(Mjpegd_Frame_TryLock(frame_temp))
            {
                LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_TRACE , 
                    MJPEGD_DBG_ARG("locked frame %p ,_sem:%d\n",frame_temp,frame_temp->_sem));
                frame_out = frame_temp;
                break;
            }
        }
    }
    catch(MULTI_FRAME_LOCKED)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SEVERE, 
            MJPEGD_DBG_ARG("Multi frame locked %d\n",lock_cnt));
        frame_out=NULL;
    }
    catch(MALLOC_FAILED)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("GetIdle malloc failed for sorted_idx[]\n"));
        frame_out = NULL;
    }
    catch(SORT_FAILED)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("GetIdle sort failed\n"));
        frame_out = NULL;
    }
    finally
    {
        if(frame_out==NULL)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS, 
                MJPEGD_DBG_ARG("no idle frame\n"));
        }

        if(sorted_idx!=NULL)
            BSP_FREE(sorted_idx);
            
        return frame_out;
    }
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
static bool Mjpegd_FramePool_Sort(Mjpegd_FramePool_t* self,u8_t* order_out,
    MJPEGD_SYSTIME_T time)
{
    MJPEGD_SYSTIME_T tmp,*time_diffs;
    u16_t time_diffs_len = self->_frames_len;
    u8_t tmp2,i,j;

    //alloc mem for time_diffs
    time_diffs = BSP_MALLOC(time_diffs_len*sizeof(*time_diffs));
    if(time_diffs==NULL)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEPOOL_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
            MJPEGD_DBG_ARG("Malloc failed for time_diffs[]\n"));
        return false;
    }

    for (i = 0; i < time_diffs_len; i++)
    {
        time_diffs[i] = time - self->_frames[i].capture_time;
        order_out[i] = i;
    }
#if MJPEGD_FRAMEPOOL_DEBUG_PRINT_SORT
    MJPEGD_DBG_PRINT("FramePool: [order,capture_time] before sort:");
    for (i = 0; i < time_diffs_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],self->_frames[i].capture_time);
    MJPEGD_DBG_PRINT(("\n"));

    MJPEGD_DBG_PRINT("FramePool: [order,time_diffs] before sort: ");
    for (i = 0; i < time_diffs_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    MJPEGD_DBG_PRINT("\n");
#endif
    //sort time_diffs and order_out
    for (i = 0; i < time_diffs_len; i++)
    {
        for (j = i+1; j < time_diffs_len; j++)
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
    for (i = 0; i < time_diffs_len; i++)
        MJPEGD_DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    MJPEGD_DBG_PRINT("\n");
#endif

    BSP_FREE(time_diffs);
    return true;
}
