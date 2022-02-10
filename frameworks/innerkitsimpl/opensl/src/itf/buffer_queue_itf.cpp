/* BufferQueue implementation */

#include <OpenSLES.h>
#include <common.h>

SLresult BufferQueueEnqueue(SLBufferQueueItf self, const void *pBuffer, SLuint32 size)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

SLresult BufferQueueClear(SLBufferQueueItf self)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult BufferQueueGetState(SLBufferQueueItf self, SLBufferQueueState *pState)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

SLresult BufferQueueRegisterCallback(SLBufferQueueItf self,
    slBufferQueueCallback callback, void *pContext)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLBufferQueueItf_ IBufferQueueItf = {
    BufferQueueEnqueue,
    BufferQueueClear,
    BufferQueueGetState,
    BufferQueueRegisterCallback
};

void IBufferQueueInit(void *self)
{
    IBufferQueue *thiz = (IBufferQueue *) self;
    thiz->mItf = &IBufferQueueItf;
}