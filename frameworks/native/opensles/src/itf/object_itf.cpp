/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <common.h>

using namespace OHOS::AudioStandard;

static SLresult Realize(SLObjectItf self, SLboolean async)
{
    return SL_RESULT_SUCCESS;
}

static SLresult Resume(SLObjectItf self, SLboolean async)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetState(SLObjectItf self, SLuint32 *state)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetInterface(SLObjectItf self, const SLInterfaceID iid, void *interface)
{
    if (self == nullptr) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    if (iid == SL_IID_ENGINE) {
        CEngine *cEngine = (CEngine *)self;
        *(void **)interface = (void *)&(cEngine->mEngine.mItf);
        return SL_RESULT_SUCCESS;
    } else if (iid == SL_IID_PLAY) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = (void *)&(cAudioPlayer->mPlay.mItf);
        return SL_RESULT_SUCCESS;
    } else if (iid == SL_IID_VOLUME) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = (void *)&(cAudioPlayer->mVolume.mItf);
        return SL_RESULT_SUCCESS;
    } else if (iid == SL_IID_OH_BUFFERQUEUE) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = (void *)&(cAudioPlayer->mBufferQueue.mItf);
        return SL_RESULT_SUCCESS;
    } else {
        MEDIA_ERR_LOG("GetInterface: SLInterfaceID not supported");
        return SL_RESULT_FEATURE_UNSUPPORTED;
    }
}

static SLresult RegisterCallback(SLObjectItf self, slObjectCallback callback, void *pContext)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static void AbortAsyncOperation(SLObjectItf self)
{
    return;
}

static SLresult SetPriority(SLObjectItf self, SLint32 priority, SLboolean preemptable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPriority(SLObjectItf self, SLint32 *pPriority, SLboolean *pPreemptable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetLossOfControlInterfaces(SLObjectItf self,
    SLint16 numInterfaces, SLInterfaceID *pInterfaceIDs, SLboolean enabled)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

void Destroy(SLObjectItf self)
{
    if (self == nullptr) {
        return;
    }
    IObject *iObject = (IObject *)self;
    SLuint32 objectId = iObject->mClass->mObjectId;
    switch (objectId) {
        case SL_OBJECTID_AUDIOPLAYER:
            AudioPlayerDestroy((void *)self);
            break;
        case SL_OBJECTID_ENGINE:
            EngineDestory((void *)self);
            break;
        case SL_OBJECTID_OUTPUTMIX:
            OutputMixDestroy((void *)self);
            break;
        default:
            MEDIA_ERR_LOG("objectId not supported");
            break;
    }
    return;
}

static const struct SLObjectItf_ ObjectItf = {
    Realize,
    Resume,
    GetState,
    GetInterface,
    RegisterCallback,
    AbortAsyncOperation,
    Destroy,
    SetPriority,
    GetPriority,
    SetLossOfControlInterfaces
};

void IObjectInit(void *self)
{
    IObject *thiz = (IObject *) self;
    thiz->mItf = &ObjectItf;
}
