#include <common.h>

using namespace OHOS::AudioStandard;

static SLresult SetRecordState(SLRecordItf self, SLuint32 state)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IRecord *thiz = (IRecord *)self;
    AudioCapturerAdapter::GetInstance()->SetCapturerStateAdapter(thiz->mId, state);
    return SL_RESULT_SUCCESS;
}

static SLresult GetRecordState(SLRecordItf self, SLuint32 *pState)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    IRecord *thiz = (IRecord *)self;
    AudioCapturerAdapter::GetInstance()->GetCapturerStateAdapter(thiz->mId, state);
    return SL_RESULT_SUCCESS;
}

static SLresult SetDurationLimit(SLRecordItf self, SLmillisecond msec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPosition(SLRecordItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult RegisterCallback(SLRecordItf self, slRecordCallback callback, void *pContext)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetCallbackEventsMask(SLRecordItf self, SLuint32 eventFlags)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetCallbackEventsMask(SLRecordItf self, SLuint32 *pEventFlags)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetMarkerPosition(SLRecordItf self,SLmillisecond mSec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult ClearMarkerPosition(SLRecordItf self)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetMarkerPosition(SLRecordItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetPositionUpdatePeriod(SLRecordItf self, SLmillisecond mSec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPositionUpdatePeriod(SLRecordItf self, SLmillisecond *pMsec)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLRecordItf_ RecordItf = {
    SetRecordState,
    GetRecordState,
    SetDurationLimit,
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

void IRecordInit(void *self, SLuint32 id)
{
    IRecord *thiz = (IRecord *)self;
    thiz->mItf = &RecordItf;
    thiz->mState = SL_RECORDSTATE_STOPPED;
    thiz->mId = id;
}
