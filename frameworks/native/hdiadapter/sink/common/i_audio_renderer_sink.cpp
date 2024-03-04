/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "i_audio_renderer_sink.h"
#include "i_audio_renderer_sink_intf.h"

#include "audio_errors.h"
#include "audio_log.h"

#include "audio_renderer_sink.h"
#include "audio_renderer_file_sink.h"
#include "bluetooth_renderer_sink.h"
#include "remote_audio_renderer_sink.h"
#include "offload_audio_renderer_sink.h"
#include "multichannel_audio_renderer_sink.h"
#undef LOG_TAG
#define LOG_TAG "IAudioRendererSink"


namespace OHOS {
namespace AudioStandard {
IAudioRendererSink *IAudioRendererSink::GetInstance(const char *devceClass, const char *deviceNetworkId)
{
    CHECK_AND_RETURN_RET_LOG(devceClass != nullptr && deviceNetworkId != nullptr, nullptr,
        "GetInstance null class or networkid");
    AUDIO_DEBUG_LOG("%{public}s Sink:GetInstance[%{public}s]", devceClass, deviceNetworkId);
    const char *deviceClassPrimary = "primary";
    const char *deviceClassUsb = "usb";
    const char *deviceClassA2DP = "a2dp";
    const char *deviceClassFile = "file_io";
    const char *deviceClassRemote = "remote";
    const char *deviceClassOffload = "offload";
    const char *deviceClassMultiChannel = "multichannel";

    IAudioRendererSink *iAudioRendererSink = nullptr;
    if (!strcmp(devceClass, deviceClassPrimary)) {
        iAudioRendererSink = AudioRendererSink::GetInstance("primary");
    }
    if (!strcmp(devceClass, deviceClassUsb)) {
        iAudioRendererSink = AudioRendererSink::GetInstance("usb");
    }
    if (!strcmp(devceClass, deviceClassA2DP)) {
        iAudioRendererSink = BluetoothRendererSink::GetInstance();
    }
    if (!strcmp(devceClass, deviceClassFile)) {
        iAudioRendererSink = AudioRendererFileSink::GetInstance();
    }
    if (!strcmp(devceClass, deviceClassRemote)) {
        iAudioRendererSink = RemoteAudioRendererSink::GetInstance(deviceNetworkId);
    }
    if (!strcmp(devceClass, deviceClassOffload)) {
        iAudioRendererSink = OffloadRendererSink::GetInstance();
    }
    if (!strcmp(devceClass, deviceClassMultiChannel)) {
        iAudioRendererSink = MultiChannelRendererSink::GetInstance("multichannel");
    }

    if (iAudioRendererSink == nullptr) {
        AUDIO_ERR_LOG("GetInstance failed with device[%{public}s]:[%{private}s]", devceClass,
            deviceNetworkId);
    }
    return iAudioRendererSink;
}
}  // namespace AudioStandard
}  // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

int32_t FillinSinkWapper(const char *device, const char *deviceNetworkId, struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");
    IAudioRendererSink *instance = IAudioRendererSink::GetInstance(device, deviceNetworkId);
    if (instance != nullptr) {
        adapter->wapper = static_cast<void *>(instance);
    } else {
        adapter->wapper = nullptr;
        return ERROR;
    }

    return SUCCESS;
}

int32_t IAudioRendererSinkInit(struct RendererSinkAdapter *adapter, const SinkAttr *attr)
{
    if (adapter == nullptr || adapter->wapper == NULL || attr == NULL) {
        AUDIO_ERR_LOG("%{public}s: Invalid parameter", __func__);
        return ERROR;
    }
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    if (audioRendererSink->IsInited()) {
        return SUCCESS;
    }
    IAudioSinkAttr iAttr = {};
    iAttr.adapterName = attr->adapterName;
    iAttr.openMicSpeaker = attr->openMicSpeaker;
    iAttr.format = attr->format;
    iAttr.sampleRate = attr->sampleRate;
    iAttr.channel = attr->channel;
    iAttr.volume = attr->volume;
    iAttr.filePath = attr->filePath;
    iAttr.deviceNetworkId = attr->deviceNetworkId;
    iAttr.deviceType = attr->deviceType;
    iAttr.channelLayout = attr->channelLayout;

    return audioRendererSink->Init(iAttr);
}

void IAudioRendererSinkDeInit(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != nullptr, "null RendererSinkAdapter");
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_LOG(audioRendererSink != nullptr, "null audioRendererSink");
    // remove the sink in allsinks.
    if (audioRendererSink->IsInited()) {
        audioRendererSink->DeInit();
    }
}

int32_t IAudioRendererSinkStop(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    if (!audioRendererSink->IsInited()) {
        return SUCCESS;
    }

    return audioRendererSink->Stop();
}

int32_t IAudioRendererSinkStart(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    CHECK_AND_RETURN_RET_LOG(audioRendererSink->IsInited(), ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first\n");

    return audioRendererSink->Start();
}

int32_t IAudioRendererSinkPause(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "Renderer pause failed");

    return audioRendererSink->Pause();
}

int32_t IAudioRendererSinkResume(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");
    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "Renderer resume failed");

    return audioRendererSink->Resume();
}

int32_t IAudioRendererSinkRenderFrame(struct RendererSinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");

    int32_t ret = audioRendererSink->RenderFrame(*data, len, *writeLen);
    return ret;
}

int32_t IAudioRendererSinkSetVolume(struct RendererSinkAdapter *adapter, float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");

    int32_t ret = audioRendererSink->SetVolume(left, right);
    return ret;
}

int32_t IAudioRendererSinkGetVolume(struct RendererSinkAdapter *adapter, float *left, float *right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");

    int32_t ret = audioRendererSink->GetVolume(*left, *right);
    return ret;
}

int32_t IAudioRendererSinkGetLatency(struct RendererSinkAdapter *adapter, uint32_t *latency)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");

    CHECK_AND_RETURN_RET_LOG(latency, ERR_INVALID_PARAM, "IAudioRendererSinkGetLatency failed latency null");

    int32_t ret = audioRendererSink->GetLatency(latency);
    return ret;
}

int32_t IAudioRendererSinkRegCallback(struct RendererSinkAdapter *adapter, int8_t *cb, int8_t *userdata)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");
    auto *callback = reinterpret_cast<OnRenderCallback*>(cb);
    audioRendererSink->RegisterRenderCallback(callback, userdata);
    return SUCCESS;
}

int32_t IAudioRendererSinkGetPresentationPosition(struct RendererSinkAdapter *adapter, uint64_t *frames,
    int64_t *timeSec, int64_t *timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED, "audioRenderer Not Inited! Init the renderer first\n");

    return audioRendererSink->GetPresentationPosition(*frames, *timeSec, *timeNanoSec);
}

int32_t IAudioRendererSinkFlush(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer Flush failed\n");

    return audioRendererSink->Flush();
}

int32_t IAudioRendererSinkReset(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IAudioRendererSink *audioRendererSink = static_cast<IAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer Reset failed\n");

    return audioRendererSink->Reset();
}

int32_t IAudioRendererSinkSetBufferSize(struct RendererSinkAdapter *adapter, uint32_t sizeMs)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer SetBufferSize failed\n");

    return audioRendererSink->SetBufferSize(sizeMs);
}

int32_t IAudioRendererSinkOffloadRunningLockInit(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer Resume failed\n");

    return audioRendererSink->OffloadRunningLockInit();
}

int32_t IAudioRendererSinkOffloadRunningLockLock(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer Resume failed\n");

    return audioRendererSink->OffloadRunningLockLock();
}

int32_t IAudioRendererSinkOffloadRunningLockUnlock(struct RendererSinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null RendererSinkAdapter");

    IOffloadAudioRendererSink *audioRendererSink = static_cast<IOffloadAudioRendererSink *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioRendererSink != nullptr, ERR_INVALID_HANDLE, "null audioRendererSink");
    bool isInited = audioRendererSink->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_NOT_STARTED,
        "audioRenderer Not Inited! Init the renderer first, Renderer Resume failed\n");

    return audioRendererSink->OffloadRunningLockUnlock();
}
#ifdef __cplusplus
}
#endif