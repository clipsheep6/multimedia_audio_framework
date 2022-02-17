/* Volume implementation */

#include <common.h>

static SLresult SetVolumeLevel(SLVolumeItf self, SLmillibel level) {
    IVolume *thiz = (IVolume *)self;
    SetVolumeLevelAdapter(thiz->mId, level);
    return SL_RESULT_SUCCESS;
}

static SLresult GetVolumeLevel(SLVolumeItf self, SLmillibel *pLevel) {
    IVolume *thiz = (IVolume *)self;
    GetVolumeLevelAdapter(thiz->mId, pLevel);
    return SL_RESULT_SUCCESS;
}

static SLresult GetMaxVolumeLevel(SLVolumeItf self, SLmillibel *pMaxLevel) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetMute(SLVolumeItf self, SLboolean state) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetMute(SLVolumeItf self, SLboolean *pState) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult EnableStereoPosition(SLVolumeItf self, SLboolean enable) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult IsEnabledStereoPosition(SLVolumeItf self, SLboolean *pEnable) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetStereoPosition(SLVolumeItf self, SLpermille stereoPosition) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetStereoPosition(SLVolumeItf self, SLpermille *pStereoPosition) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLVolumeItf_ VolumeItf = {
    SetVolumeLevel,
    GetVolumeLevel,
    GetMaxVolumeLevel,
    SetMute,
    GetMute,
    EnableStereoPosition,
    IsEnabledStereoPosition,
    SetStereoPosition,
    GetStereoPosition
};

void IVolumeInit(void *self, SLuint32 id)
{
    IVolume *thiz = (IVolume *) self;
    thiz->mItf = &VolumeItf;
    thiz->mId = id;
}
