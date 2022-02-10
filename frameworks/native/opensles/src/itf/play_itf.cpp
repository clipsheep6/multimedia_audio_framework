/* Play implementation */

#include <common.h>

static SLresult SetPlayState(SLPlayItf self, SLuint32 state)
{
    IPlay *thiz = (IPlay *)self;
    SetPlayStateAdapter(thiz->mId, state);
    return SL_RESULT_SUCCESS;
    //return setPlayStateAdapter(thiz->mId, state);
}

static SLresult GetPlayState(SLPlayItf self, SLuint32 *pState)
{
    IPlay *thiz = (IPlay *)self;
    GetPlayStateAdapter(thiz->mId, pState);
    return SL_RESULT_SUCCESS;
}

static SLresult GetDuration(SLPlayItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPosition(SLPlayItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult RegisterCallback(SLPlayItf self, slPlayCallback callback, void *pContext)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetCallbackEventsMask(SLPlayItf self, SLuint32 eventFlags)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}


static SLresult GetCallbackEventsMask(SLPlayItf self, SLuint32 *pEventFlags)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetMarkerPosition(SLPlayItf self, SLmillisecond mSec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult ClearMarkerPosition(SLPlayItf self)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetMarkerPosition(SLPlayItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetPositionUpdatePeriod(SLPlayItf self, SLmillisecond mSec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPositionUpdatePeriod(SLPlayItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLPlayItf_ PlayItf = {
    SetPlayState,
    GetPlayState,
    GetDuration,
    GetPosition,
    RegisterCallback,
    SetCallbackEventsMask,
    GetCallbackEventsMask,
    SetMarkerPosition,
    ClearMarkerPosition,
    GetMarkerPosition,
    SetPositionUpdatePeriod,
    GetPositionUpdatePeriod
};

void IPlayInit(void *self, SLuint32 id)
{
    IPlay *thiz = (IPlay *)self;
    thiz->mItf = &PlayItf;
    thiz->mState = SL_PLAYSTATE_STOPPED;
    thiz->mId = id;
}
