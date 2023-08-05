/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstring>
#include <dlfcn.h>
#include <string>
#include <cinttypes>
#include "power_mgr_client.h"
#include "running_lock.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "audio_capturer_source.h"
#include "v1_0/iaudio_manager.h"
#include "audio_capturer_source_wakeup.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;

AudioCapturerSourceWakeup::AudioCapturerSourceWakeup()
    : capturerInited_(false), started_(false), paused_(false), leftVolume_(MAX_VOLUME_LEVEL),
      rightVolume_(MAX_VOLUME_LEVEL), openMic_(0), audioManager_(nullptr), audioAdapter_(nullptr),
      audioCapture_(nullptr)
{
    attr_ = {};
#ifdef CAPTURE_DUMP
    pfd = nullptr;
#endif // CAPTURE_DUMP
}

AudioCapturerSourceWakeup::~AudioCapturerSourceWakeup()
{
    AUDIO_ERR_LOG("~AudioCapturerSourceWakeup");
}

bool AudioCapturerSourceWakeup::IsInited(void)
{
    return capturerInited_;
}

void AudioCapturerSourceWakeup::DeInit()
{
    started_ = false;
    capturerInited_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioAdapter_, captureId_);
    }

    IAudioSourceCallback* callback = nullptr;
    {
        std::lock_guard<std::mutex> lck(callbackMutex_);
        callback = callback_;
    }
    if (callback != nullptr) {
        callback->OnWakeupClose();
    }

    audioCapture_ = nullptr;

    if (audioManager_ != nullptr) {
        audioManager_->UnloadAdapter(audioManager_, adapterDesc_.adapterName);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;
#ifdef CAPTURE_DUMP
    if (pfd) {
        fclose(pfd);
        pfd = nullptr;
    }
#endif // CAPTURE_DUMP
}

static void InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = INTERNAL_INPUT_STREAM_ID;
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    /* 16 * 1024 */
    attrs.silenceThreshold = AUDIO_BUFF_SIZE;
    attrs.sourceType = SOURCE_TYPE_MIC;
}

static int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size, const std::string &adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort &capturePort)
{
    if (descs == nullptr) {
        return ERROR;
    }

    for (uint32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        if (!adapterNameCase.compare(desc->adapterName)) {
            for (uint32_t port = 0; port < desc->portsLen; port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    capturePort = desc->ports[port];
                    return index;
                }
            }
        }
    }
    AUDIO_ERR_LOG("SwitchAdapterCapture Fail");

    return ERR_INVALID_INDEX;
}

int32_t AudioCapturerSourceWakeup::InitAudioManager()
{
    AUDIO_INFO_LOG("AudioCapturerSourceWakeup: Initialize audio proxy manager");

    audioManager_ = IAudioManagerGet(false);
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

int32_t AudioCapturerSourceWakeup::CreateCapture(struct AudioPort &capturePort)
{
    int32_t ret;
    struct AudioSampleAttributes param;
    // User needs to set
    InitAttrsCapture(param);
    param.sampleRate = attr_.sampleRate;
    param.format = (AudioFormat)(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    param.sourceType = attr_.sourceType;

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = PIN_IN_MIC;
    deviceDesc.desc = (char *)"";

    ret = audioAdapter_->CreateCapture(audioAdapter_, &deviceDesc, &param, &audioCapture_, &captureId_);
    AUDIO_INFO_LOG("CreateCapture param.sourceType: %{public}d", param.sourceType);
    if (audioCapture_ == nullptr || ret < 0) {
        AUDIO_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }

    return 0;
}

int32_t AudioCapturerSourceWakeup::Init(IAudioSourceAttr &attr)
{
    if (InitAudioManager() != 0) {
        AUDIO_ERR_LOG("Init audio manager Fail");
        return ERR_INVALID_HANDLE;
    }
    attr_ = attr;
    int32_t ret;
    int32_t index;
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    if (size > MAX_AUDIO_ADAPTER_NUM || size == 0 || ret != 0) {
        AUDIO_ERR_LOG("Get adapters Fail");
        return ERR_NOT_STARTED;
    }
    // Get qualified sound card and port
    adapterNameCase_ = attr_.adapterName;
    openMic_ = attr_.open_mic_speaker;
    index = SwitchAdapterCapture((struct AudioAdapterDescriptor *)&descs, size, adapterNameCase_, PORT_IN, audioPort);
    if (index < 0) {
        AUDIO_ERR_LOG("Switch Adapter Capture Fail");
        return ERR_NOT_STARTED;
    }
    adapterDesc_ = descs[index];
    if (audioManager_->LoadAdapter(audioManager_, &adapterDesc_, &audioAdapter_) != 0) {
        AUDIO_ERR_LOG("Load Adapter Fail");
        return ERR_NOT_STARTED;
    }
    CHECK_AND_RETURN_RET_LOG(audioAdapter_ != nullptr, ERR_NOT_STARTED, "Load audio device failed");

    // Inittialization port information, can fill through mode and other parameters
    if (audioAdapter_->InitAllPorts(audioAdapter_) != 0) {
        AUDIO_ERR_LOG("InitAllPorts failed");
        return ERR_DEVICE_INIT;
    }
    if (CreateCapture(audioPort) != 0) {
        AUDIO_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }
    if (openMic_) {
        ret = SetInputRoute(DEVICE_TYPE_MIC);
        if (ret < 0) {
            AUDIO_ERR_LOG("AudioCapturerSourceWakeup:update route FAILED: %{public}d", ret);
        }
    }
    capturerInited_ = true;

    OpenDumpFile();
    return SUCCESS;
}

void AudioCapturerSourceWakeup::OpenDumpFile()
{
#ifdef CAPTURE_DUMP
    pfd = fopen(g_audioOutTestFilePath, "wb+");
    if (pfd == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
#endif // CAPTURE_DUMP
}

int32_t AudioCapturerSourceWakeup::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int64_t stamp = ClockTime::GetCurNano();
    int32_t ret;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("Audio capture Handle is nullptr!");
        return ERR_INVALID_HANDLE;
    }

    uint32_t frameLen = static_cast<uint32_t>(requestBytes);
    ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t*>(frame), &frameLen, &replyBytes);
    if (ret < 0) {
        AUDIO_ERR_LOG("Capture Frame Fail");
        return ERR_READ_FAILED;
    }

#ifdef CAPTURE_DUMP
    if (pfd) {
        size_t writeResult = fwrite(frame, 1, replyBytes, pfd);
        if (writeResult != replyBytes) {
            AUDIO_ERR_LOG("Failed to write the file.");
        }
    }
#endif // CAPTURE_DUMP

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", requestBytes, stamp);
    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::Start(void)
{
    AUDIO_INFO_LOG("Start.");
    if (mKeepRunningLock == nullptr) {
        switch (attr_.sourceType) {
            case SOURCE_TYPE_WAKEUP:
                mKeepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioWakeupCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
                break;
            case SOURCE_TYPE_MIC:
            default:
                mKeepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioPrimaryCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        }
    }
    if (mKeepRunningLock != nullptr) {
        AUDIO_INFO_LOG("AudioCapturerSourceWakeup call KeepRunningLock lock");
        mKeepRunningLock->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
    } else {
        AUDIO_ERR_LOG("mKeepRunningLock is null, start can not work well!");
    }

    int32_t ret;
    if (!started_) {
        ret = audioCapture_->Start(audioCapture_);
        if (ret < 0) {
            return ERR_NOT_STARTED;
        }
        started_ = true;
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::SetVolume(float left, float right)
{
    float volume;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::SetVolume failed audioCapture_ null");
        return ERR_INVALID_HANDLE;
    }

    rightVolume_ = right;
    leftVolume_ = left;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    audioCapture_->SetVolume(audioCapture_, volume);

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::GetVolume(float &left, float &right)
{
    float val = 0.0;
    audioCapture_->GetVolume(audioCapture_, &val);
    left = val;
    right = val;

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::SetMute(bool isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::SetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    ret = audioCapture_->SetMute(audioCapture_, isMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::SetMute failed from hdi");
    }

    if (audioAdapter_ != nullptr) {
        ret = audioAdapter_->SetMicMute(audioAdapter_, isMute);
        if (ret != 0) {
            AUDIO_ERR_LOG("AudioCapturerSourceWakeup::SetMicMute failed from hdi");
        }
    }

    AudioCapturerSource::micMuteState_ = isMute;

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::GetMute(bool &isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::GetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    bool isHdiMute = false;
    ret = audioCapture_->GetMute(audioCapture_, &isHdiMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::GetMute failed from hdi");
    }

    isMute = AudioCapturerSource::micMuteState_;

    return SUCCESS;
}

static AudioCategory GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AUDIO_IN_COMMUNICATION;
            break;
        case AUDIO_SCENE_RINGING:
            audioCategory = AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AUDIO_IN_MEDIA;
            break;
        default:
            audioCategory = AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("AudioCapturerSourceWakeup: Audio category returned is: %{public}d", audioCategory);

    return audioCategory;
}

static int32_t SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;

    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
            source.ext.device.type = PIN_IN_MIC;
            source.ext.device.desc = (char *)"pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = PIN_IN_HS_MIC;
            source.ext.device.desc = (char *)"pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = PIN_IN_USB_EXT;
            source.ext.device.desc = (char *)"pin_in_usb_ext";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            source.ext.device.type = PIN_IN_BLUETOOTH_SCO_HEADSET;
            source.ext.device.desc = (char *)"pin_in_bluetooth_sco_headset";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t AudioCapturerSourceWakeup::SetInputRoute(DeviceType inputDevice)
{
    AudioPortPin inputPortPin = PIN_IN_MIC;
    return SetInputRoute(inputDevice, inputPortPin);
}

int32_t AudioCapturerSourceWakeup::SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetInputPortPin(inputDevice, source);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup: SetOutputRoute FAILED: %{public}d", ret);
        return ret;
    }

    inputPortPin = source.ext.device.type;
    AUDIO_INFO_LOG("AudioCapturerSourceWakeup: Input PIN is: 0x%{public}X", inputPortPin);
    source.portId = static_cast<int32_t>(audioPort.portId);
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;
    source.ext.device.desc = (char *)"";

    sink.portId = 0;
    sink.role = AUDIO_PORT_SINK_ROLE;
    sink.type = AUDIO_PORT_MIX_TYPE;
    sink.ext.mix.moduleId = 0;
    sink.ext.mix.streamId = INTERNAL_INPUT_STREAM_ID;
    sink.ext.device.desc = (char *)"";

    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = &sink,
        .sinksLen = 1,
    };

    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup: AudioAdapter object is null.");
        return ERR_OPERATION_FAILED;
    }

    ret = audioAdapter_->UpdateAudioRoute(audioAdapter_, &route, &routeHandle_);
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup: UpdateAudioRoute failed");
        return ERR_OPERATION_FAILED;
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    AUDIO_INFO_LOG("AudioCapturerSourceWakeup::SetAudioScene scene: %{public}d, device: %{public}d",
        audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene <= AUDIO_SCENE_PHONE_CHAT,
        ERR_INVALID_PARAM, "invalid audioScene");
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerSourceWakeup::SetAudioScene failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }
    if (openMic_) {
        AudioPortPin audioSceneInPort = PIN_IN_MIC;
        int32_t ret = SetInputRoute(activeDevice, audioSceneInPort);
        if (ret < 0) {
            AUDIO_ERR_LOG("AudioCapturerSourceWakeup: Update route FAILED: %{public}d", ret);
        }
        struct AudioSceneDescriptor scene;
        scene.scene.id = GetAudioCategory(audioScene);
        scene.desc.pins = audioSceneInPort;
        scene.desc.desc = (char *)"";

        ret = audioCapture_->SelectScene(audioCapture_, &scene);
        if (ret < 0) {
            AUDIO_ERR_LOG("AudioCapturerSourceWakeup: Select scene FAILED: %{public}d", ret);
            return ERR_OPERATION_FAILED;
        }
    }
    AUDIO_INFO_LOG("AudioCapturerSourceWakeup::Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

uint64_t AudioCapturerSourceWakeup::GetTransactionId()
{
    AUDIO_INFO_LOG("AudioCapturerSourceWakeup::GetTransactionId in");
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t AudioCapturerSourceWakeup::Stop(void)
{
    AUDIO_INFO_LOG("Stop.");

    if (mKeepRunningLock != nullptr) {
        AUDIO_INFO_LOG("AudioCapturerSourceWakeup call KeepRunningLock UnLock");
        mKeepRunningLock->UnLock();
    } else {
        AUDIO_ERR_LOG("mKeepRunningLock is null, stop can not work well!");
    }

    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->Stop(audioCapture_);
        if (ret < 0) {
            AUDIO_ERR_LOG("Stop capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    started_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::Pause(void)
{
    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->Pause(audioCapture_);
        if (ret != 0) {
            AUDIO_ERR_LOG("pause capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = true;

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::Resume(void)
{
    int32_t ret;
    if (paused_ && audioCapture_ != nullptr) {
        ret = audioCapture_->Resume(audioCapture_);
        if (ret != 0) {
            AUDIO_ERR_LOG("resume capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::Reset(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceWakeup::Flush(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

void AudioCapturerSourceWakeup::RegisterWakeupCloseCallback(IAudioSourceCallback* callback)
{
    AUDIO_INFO_LOG("Register WakeupClose Callback");
    std::lock_guard<std::mutex> lck(callbackMutex_);
    callback_ = callback;
}

} // namespace AudioStandard
} // namesapce OHOS