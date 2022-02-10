/* OpenHarmony BufferQueue implementation */

#include <common.h>

SLresult Enqueue(SLOHBufferQueueItf self, const void *pBuffer, SLuint32 size)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    EnqueueAdapter(thiz->mId, pBuffer, size);
    return SL_RESULT_SUCCESS;
}

SLresult Clear(SLOHBufferQueueItf self)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    ClearAdapter(thiz->mId);
    return SL_RESULT_SUCCESS;
}

SLresult GetState(SLOHBufferQueueItf self, SLOHBufferQueueState *pState)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    GetStateAdapter(thiz->mId, pState);
    return SL_RESULT_SUCCESS;
}

SLresult GetBuffer(SLOHBufferQueueItf self, SLuint8 **pBuffer, SLuint32 &pSize)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)self;
    GetBufferAdapter(thiz->mId, pBuffer, pSize);
    return SL_RESULT_SUCCESS;
}

SLresult RegisterCallback(SLOHBufferQueueItf self,
    slOHBufferQueueCallback callback, void *pContext)
{
    RegisterCallbackAdapter(self, callback, pContext);
    return SL_RESULT_SUCCESS;
}

static const struct SLOHBufferQueueItf_ IOHBufferQueueItf = {
    Enqueue,
    Clear,
    GetState,
    GetBuffer,
    RegisterCallback
};

void IOHBufferQueueInit(void *self, SLuint32 id)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *) self;
    thiz->mItf = &IOHBufferQueueItf;
    thiz->mId = id;
}