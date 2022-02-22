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

static SLuint32 audioplayerId = 0;

static SLresult CreateLEDDevice(
    SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateVibraDevice(
    SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
static SLresult CreateAudioPlayer(
    SLEngineItf self, SLObjectItf *pPlayer, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    ClassTable *audioPlayerClass = ObjectIdToClass(SL_OBJECTID_AUDIOPLAYER);
    CAudioPlayer *thiz = (CAudioPlayer *) Construct(audioPlayerClass, self);
    thiz->mId = audioplayerId;
    IObjectInit(&thiz->mObject);
    IPlayInit(&thiz->mPlay, audioplayerId);
    IVolumeInit(&thiz->mVolume, audioplayerId);
    IOHBufferQueueInit(&thiz->mBufferQueue, audioplayerId);
    *pPlayer = &thiz->mObject.mItf;
    AudioPlayerAdapter::GetInstance()->
        CreateAudioPlayerAdapter(audioplayerId, pAudioSrc, pAudioSnk, OHOS::AudioStandard::STREAM_MUSIC);
    audioplayerId++;
    return SL_RESULT_SUCCESS;
}
 
static SLresult CreateAudioRecorder(
    SLEngineItf self, SLObjectItf *pRecorder, SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateMidiPlayer(
    SLEngineItf self, SLObjectItf *pPlayer, SLDataSource *pMIDISrc, SLDataSource *pBankSrc, SLDataSink *pAudioOutput,
    SLDataSink *pVibra, SLDataSink *pLEDArray, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateListener(
    SLEngineItf self, SLObjectItf *pListener, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult Create3DGroup(
    SLEngineItf self, SLObjectItf *pGroup, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateOutputMix(
    SLEngineItf self, SLObjectItf *pMix, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    ClassTable *outputMixClass = ObjectIdToClass(SL_OBJECTID_OUTPUTMIX);
    COutputMix *thiz = (COutputMix *) Construct(outputMixClass, self);
    IObjectInit(&thiz->mObject);
    *pMix = &thiz->mObject.mItf;
    return SL_RESULT_SUCCESS;
}

static SLresult CreateMetadataExtractor(
    SLEngineItf self, SLObjectItf *pMetadataExtractor, SLDataSource *pDataSource, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult CreateExtensionObject(
    SLEngineItf self, SLObjectItf *pObject, void *pParameters, SLuint32 objectID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QueryNumSupportedInterfaces(SLEngineItf self, SLuint32 objectID, SLuint32 *pNumSupportedInterfaces)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QuerySupportedInterfaces(
    SLEngineItf self, SLuint32 objectID, SLuint32 index, SLInterfaceID *pInterfaceId)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QueryNumSupportedExtensions(SLEngineItf self, SLuint32 *pNumExtensions)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult QuerySupportedExtension(
    SLEngineItf self, SLuint32 index, SLchar *pExtensionName, SLint16 *pNameLength)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult IsExtensionSupported(SLEngineItf self, const SLchar *pExtensionName, SLboolean *pSupported)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLEngineItf_ EngineItf = {
    CreateLEDDevice,
    CreateVibraDevice,
    CreateAudioPlayer,
    CreateAudioRecorder,
    CreateMidiPlayer,
    CreateListener,
    Create3DGroup,
    CreateOutputMix,
    CreateMetadataExtractor,
    CreateExtensionObject,
    QueryNumSupportedInterfaces,
    QuerySupportedInterfaces,
    QueryNumSupportedExtensions,
    QuerySupportedExtension,
    IsExtensionSupported
};

void IEngineInit(void *self)
{
    IEngine *thiz = (IEngine *) self;
    thiz->mItf = &EngineItf;
}