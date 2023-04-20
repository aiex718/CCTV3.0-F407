#include "eth/apps/mjpeg/mjpegd_framebuf.h"
#include "bsp/sys/callback.h"
#include "bsp/sys/systime.h"
#include "bsp/sys/atomic.h"
#include "eth/apps/mjpeg/mjpegd_debug.h"

//TODO: make a linklist.h
// #define LINKLIST_GET_UNTIL(p,list,end) do{\
//     for (p=list; p!=NULL && p->_next!=end; p=p->_next);\
// }while(0)
// #define LINKLIST_GET_LAST(p,list) LINKLIST_GET_UNTIL(p,list,NULL)

static void Mjpegd_FrameBuf_Sort(Mjpegd_FrameBuf_t* self,u8_t* order_out,SysTime_t time);

void Mjpegd_FrameBuf_Init(Mjpegd_FrameBuf_t* self)
{
    u8_t i;
    
    BSP_ARR_CLEAR(self->Mjpegd_FrameBuf_Callbacks);

    for (i = 0; i < self->_frames_len; i++)
        Mjpegd_Frame_Init(&self->_frames[i]);
    self->_pending_frame = NULL;
    self->_fps_counter = 0;
    self->_fps_timer = 0;
}

void Mjpegd_FrameBuf_SetCallback(Mjpegd_FrameBuf_t* self, Mjpegd_FrameBuf_CallbackIdx_t cb_idx, Callback_t* callback)
{
    if(cb_idx < __NOT_CALLBACK_FRAMEBUF_MAX)
        self->Mjpegd_FrameBuf_Callbacks[cb_idx] = callback;
}

void Mjpegd_FrameBuf_Service(Mjpegd_FrameBuf_t* self)
{
    SysTime_t now = SysTime_Get();
    //take pending frame to local and process it
    Mjpegd_Frame_t* pending_frame = (Mjpegd_Frame_t*)
            Atomic_Exchange((__IO u32_t *)&self->_pending_frame,NULL);
    if(pending_frame!=NULL)
    {
        //invoke callback
        Callback_Invoke_Idx(self, pending_frame,
            self->Mjpegd_FrameBuf_Callbacks, FRAMEBUF_CALLBACK_RX_RAWFRAME);
        //release frame
        Mjpegd_Frame_Unlock(pending_frame);
        Callback_Invoke_Idx(self, pending_frame,
            self->Mjpegd_FrameBuf_Callbacks, FRAMEBUF_CALLBACK_RX_NEWFRAME);
        
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE, 
            DBG_ARG("ProcessedFrame %p, _sem=%d\n",pending_frame,pending_frame->_sem));
        
        self->_fps_counter++;
    }

    if(now - self->_fps_timer > 8000)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_STATE, 
            DBG_ARG("Proc fps %d\n",self->_fps_counter>>3));
        self->_fps_counter = 0;
        self->_fps_timer = now;
    }
}

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetLatest(Mjpegd_FrameBuf_t* self,SysTime_t last_frame_time)
{
    u8_t i,sorted_idx[self->_frames_len];
    SysTime_t now = SysTime_Get();
    Mjpegd_Frame_t* frame;

    //Try to acquire newest frame.
    //Newest node should be the first node.
    Mjpegd_FrameBuf_Sort(self,sorted_idx,now);
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
                //how much time has passed since last frame
                SysTime_t last_frame_diff = now - last_frame_time;
                //how much time has passed since this frame
                SysTime_t new_frame_diff = now - frame->capture_time;
                //if this frame is newer than last frame
                if(last_frame_diff>new_frame_diff)
                {
                    if(Mjpegd_Frame_TryAcquire(frame))
                        break;
                }
                else
                {
                    //if newest frame is outdated, other frames are also outdated
                    //no need to check other frames
                    LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_SERIOUS , 
                        DBG_ARG("all frame outdated\n"));
                    frame = NULL;
                    break;
                }
            }
        }
        frame = NULL;
    }

    if(frame!=NULL)
    {
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE , 
            DBG_ARG("acquired frame %p\n",frame));
    }
    
    return frame;
}

void Mjpegd_FrameBuf_Release(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame)
{
    if(frame!=NULL)
    {
        Mjpegd_Frame_Release(frame);
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE , 
            DBG_ARG("release frame %p\n",frame));

        if(frame->_sem>MJPEGD_FRAME_SEMAPHORE_MAX)
        {
            LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_SEVERE , 
                DBG_ARG("frame %p ->_sem=%d > MAX %d\n",frame,frame->_sem,MJPEGD_FRAME_SEMAPHORE_MAX));
        }
    }
}

Mjpegd_Frame_t* Mjpegd_FrameBuf_GetIdle(Mjpegd_FrameBuf_t* self)
{
    u8_t i,sorted_idx[self->_frames_len];
    SysTime_t now = SysTime_Get();
    Mjpegd_Frame_t* frame;

    //Try to get and lock oldest idle node.
    //Oldest node should be the last node.
    Mjpegd_FrameBuf_Sort(self,sorted_idx,now);
    for (i = self->_frames_len; i != 0; i--)
    {
        frame = &self->_frames[sorted_idx[i-1]];

        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE , 
            DBG_ARG("trylock frame %p ,_sem:%d\n",frame,frame->_sem));
        if(Mjpegd_Frame_TryLock(frame))
        {
            LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE , 
                DBG_ARG("locked frame %p ,_sem:%d\n",frame,frame->_sem));
            return frame;
        }
    }

    LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_SERIOUS, 
        DBG_ARG("no idle frame\n"));
    
    return NULL;
}

/**
 * @brief Return idle frame previous locked by calling Mjpegd_FrameBuf_GetIdle().
 * @details Returned frame will be assigned to _pending_frame, waiting service call
 *          to process the raw frame(by invoking FRAMEBUF_CALLBACK_RX_RAWFRAME).
 * @warning If original _pending_frame is not NULL, it will be released directly,
 *          which cause frame dropped, we dont buffer frame since all mjpegd client
 *          can reference to the same newest frame, buffer old frame make no sense.
 */
void Mjpegd_FrameBuf_ReturnIdle(Mjpegd_FrameBuf_t* self,Mjpegd_Frame_t* frame)
{
    if(frame!=NULL)
    {
        Mjpegd_Frame_t* drop_frame = NULL;

        if(Mjpegd_Frame_IsValid(frame)==false)
            drop_frame = frame;
        else 
        {
            drop_frame = (Mjpegd_Frame_t*)
                Atomic_Exchange((__IO u32_t *)&self->_pending_frame,(u32_t)frame);
        }
            
        if(drop_frame!=NULL)
        {
            //LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_WARNING, 
            //    DBG_ARG("Frame dropped %p, service too slow?\n",drop_frame));
            Mjpegd_Frame_Clear(drop_frame);
            Mjpegd_Frame_Unlock(drop_frame);
        }
        else
        {
            LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_TRACE, 
                DBG_ARG("ReturnIdle %p, _sem=%d\n",frame,frame->_sem));
        }
    }
    else
    {
        LWIP_DEBUGF(MJPEGD_FRAMEBUF_DEBUG | LWIP_DBG_LEVEL_WARNING, 
            DBG_ARG("ReturnIdle Null\n"));
    }
}

//sort by frame's time diff (capture_time-now),order from small to big(new to old)
static void Mjpegd_FrameBuf_Sort(Mjpegd_FrameBuf_t* self,u8_t* order_out,SysTime_t time)
{
    //notice:vla on armcc alloc at heap
    SysTime_t tmp,time_diffs[self->_frames_len];
    u8_t tmp2,i,j;
    for (i = 0; i < self->_frames_len; i++)
    {
        time_diffs[i] = time - self->_frames[i].capture_time;
        order_out[i] = i;
    }
#if MJPEGD_FRAMEBUF_DEBUG_PRINT_SORT
    DBG_PRINT("FrameBuf: [order,capture_time] before sort:");
    for (i = 0; i < self->_frames_len; i++)
        DBG_PRINT("[%d,%d] ",order_out[i],self->_frames[i].capture_time);
    DBG_PRINT(("\n"));

    DBG_PRINT("FrameBuf: [order,time_diffs] before sort: ");
    for (i = 0; i < self->_frames_len; i++)
        DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    DBG_PRINT("\n");
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

#if MJPEGD_FRAMEBUF_DEBUG_PRINT_SORT
    DBG_PRINT("FrameBuf: [order,time_diffs] after sort: ");
    for (i = 0; i < self->_frames_len; i++)
        DBG_PRINT("[%d,%d] ",order_out[i],time_diffs[i]);
    DBG_PRINT("\n");
#endif
}
