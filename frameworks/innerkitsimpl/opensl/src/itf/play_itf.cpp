/* Play implementation */

#include<OpenSLES.h>
#include<common.h>
#include<iostream>

static SLresult SetPlayState(SLPlayItf self, SLuint32 state)
{
    std::cout << "SetPlayState in" << std::endl;
    IPlay *thiz = (IPlay *)self;
    thiz->mState = state;
    //invoke adapter
    std::cout << "SetPlayState out" << std::endl;
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPlayState(SLPlayItf self, SLuint32 *pState)
{
    std::cout << "GetPlayState in" << std::endl;
    //IPlay *thiz = (IPlay *)self;
    CAudioPlayer *thiz = (CAudioPlayer *)self;
    //SLuint32 id = thiz->mId;
    *pState = thiz->mPlay.mState;
    //invoke adapter(id, state);
    std::cout << "GetPlayState out" << std::endl;
    return SL_RESULT_FEATURE_UNSUPPORTED;
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

void IPlayInit(void *self)
{
    IPlay *thiz = (IPlay *)self;
    thiz->mItf = &PlayItf;
    thiz->mState = SL_PLAYSTATE_STOPPED;
}
