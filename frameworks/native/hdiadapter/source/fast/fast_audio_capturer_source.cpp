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

#include <string>
#include <cinttypes>
#include "power_mgr_client.h"
#include "running_lock.h"

#include "audio_errors.h"
#include "audio_log.h"

#include "audio_manager.h"
#include "fast_audio_capturer_source.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
class FastAudioCapturerSourceInner : public FastAudioCapturerSource {
public:
    int32_t Init(IAudioSourceAttr &attr) override;
    bool IsInited(void) override;
    void DeInit(void) override;

    int32_t Start(void) override;
    int32_t Stop(void) override;
    int32_t Flush(void) override;
    int32_t Reset(void) override;
    int32_t Pause(void) override;
    int32_t Resume(void) override;
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes) override;
    int32_t SetVolume(float left, float right) override;
    int32_t GetVolume(float &left, float &right) override;
    int32_t SetMute(bool isMute) override;
    int32_t GetMute(bool &isMute) override;

    int32_t SetAudioScene(AudioScene audioScene, DeviceType activeDevice) override;

    int32_t SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin);

    int32_t SetInputRoute(DeviceType inputDevice) override;

    uint64_t GetTransactionId() override;

    int32_t GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
        uint32_t &byteSizePerFrame) override;
    int32_t GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override;

    FastAudioCapturerSourceInner();
    ~FastAudioCapturerSourceInner();
private:
    static constexpr int32_t INVALID_FD = -1;
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    static constexpr int64_t SECOND_TO_NANOSECOND = 1000000000;
    static constexpr  uint32_t PCM_8_BIT = 8;
    static constexpr  uint32_t PCM_16_BIT = 16;
    static constexpr  uint32_t PCM_24_BIT = 24;
    static constexpr  uint32_t PCM_32_BIT = 32;
    static constexpr uint32_t AUDIO_CHANNELCOUNT = 2;
    static constexpr uint32_t AUDIO_SAMPLE_RATE_48K = 48000;
    static constexpr uint32_t INT_32_MAX = 0x7fffffff;

    IAudioSourceAttr attr_;
    bool capturerInited_;
    bool started_;
    bool paused_;

    uint32_t openMic_;
    std::string adapterNameCase_;
    struct AudioManager *audioManager_ = nullptr;
    struct AudioAdapter *audioAdapter_ = nullptr;
    struct AudioCapture *audioCapture_ = nullptr;
    struct AudioAdapterDescriptor adapterDesc_;
    struct AudioPort audioPort;

    size_t bufferSize_ = 0;
    uint32_t bufferTotalFrameSize_ = 0;

    int bufferFd_ = INVALID_FD;
    uint32_t eachReadFrameSize_ = 0;

    std::shared_ptr<PowerMgr::RunningLock> mKeepRunningLock;
    void InitAttrsCapture(struct AudioSampleAttributes &attrs);
    int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size,
    const std::string &adapterNameCase, enum AudioPortDirection portFlag, struct AudioPort &capturePort);
    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t PrepareMmapBuffer();
    int32_t InitAudioManager();
    uint32_t PcmFormatToBits(AudioSampleFormat format);
};

constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;

FastAudioCapturerSourceInner::FastAudioCapturerSourceInner()
    : capturerInited_(false), started_(false), paused_(false), openMic_(0), audioManager_(nullptr), audioAdapter_(nullptr),
      audioCapture_(nullptr)
{
    attr_ = {};
}

FastAudioCapturerSourceInner::~FastAudioCapturerSourceInner()
{
    AUDIO_ERR_LOG("~FastAudioCapturerSourceInner");
}
FastAudioCapturerSource *FastAudioCapturerSource::GetInstance()
{
    static FastAudioCapturerSourceInner audioCapturer;
    return &audioCapturer;
}

bool FastAudioCapturerSourceInner::IsInited(void)
{
    return capturerInited_;
}

void FastAudioCapturerSourceInner::DeInit()
{
    started_ = false;
    capturerInited_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioAdapter_, audioCapture_);
    }
    audioCapture_ = nullptr;

    if (audioManager_ != nullptr) {
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;
}

void FastAudioCapturerSourceInner::InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = 0;
    attrs.type = AUDIO_MMAP_NOIRQ; // enable mmap!
    attrs.period = 0;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = 0;
    attrs.stopThreshold = INT_32_MAX;
    /* 16 * 1024 */
    attrs.silenceThreshold = 0;
}

int32_t FastAudioCapturerSourceInner::SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size,
    const std::string &adapterNameCase, enum AudioPortDirection portFlag, struct AudioPort &capturePort)
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
            for (uint32_t port = 0; port < desc->portNum; port++) {
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

int32_t FastAudioCapturerSourceInner::InitAudioManager()
{
    AUDIO_INFO_LOG("FastAudioCapturerSourceInner: Initialize audio proxy manager");

    audioManager_ = GetAudioManagerFuncs();
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

int32_t FastAudioCapturerSourceInner::CreateCapture(struct AudioPort &capturePort)
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
    param.startThreshold = 0;

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = PIN_IN_MIC;
    deviceDesc.desc = (char *)"";

    ret = audioAdapter_->CreateCapture(audioAdapter_, &deviceDesc, &param, &audioCapture_);
    if (audioCapture_ == nullptr || ret < 0) {
        AUDIO_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }

    return 0;
}

uint32_t FastAudioCapturerSourceInner::PcmFormatToBits(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return PCM_8_BIT;
        case SAMPLE_S16LE:
            return PCM_16_BIT;
        case SAMPLE_S24LE:
            return PCM_24_BIT;
        case SAMPLE_S32LE:
            return PCM_32_BIT;
        case SAMPLE_F32LE:
            return PCM_32_BIT;
        default:
            return PCM_24_BIT;
    }
}
int32_t FastAudioCapturerSourceInner::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame)
{
    if (bufferFd_ == INVALID_FD) {
        AUDIO_ERR_LOG("buffer fd has been released!");
        return ERR_INVALID_HANDLE;
    }
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("Audio render is null!");
        return ERR_INVALID_HANDLE;
    }

    struct AudioTimeStamp timestamp = {};
    int32_t ret = audioCapture_->attr.GetMmapPosition(audioCapture_, &frames, &timestamp);
    if (ret != 0) {
        AUDIO_ERR_LOG("Hdi GetMmapPosition filed, ret:%{public}d!", ret);
        return ERR_OPERATION_FAILED;
    }

    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it.
    if (timestamp.tvSec < 0 || timestamp.tvSec > maxSec || timestamp.tvNSec < 0 ||
        timestamp.tvNSec > SECOND_TO_NANOSECOND) {
        AUDIO_ERR_LOG("Hdi GetMmapPosition get invaild second:%{public}" PRId64 " or nanosecond:%{public}" PRId64 " !",
            timestamp.tvSec, timestamp.tvNSec);
        return ERR_OPERATION_FAILED;
    }
    timeSec = timestamp.tvSec;
    timeNanoSec = timestamp.tvNSec;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::PrepareMmapBuffer()
{
    int32_t totalBifferInMs = 40; // 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency.
    uint32_t frameSizeInByte_ = PcmFormatToBits(attr_.format) * attr_.channel / PCM_8_BIT;
    int32_t reqBufferFrameSize = totalBifferInMs * (attr_.sampleRate / 1000);

    struct AudioMmapBufferDescriptor desc = {0};
    int32_t ret = audioCapture_->attr.ReqMmapBuffer(audioCapture_, reqBufferFrameSize, &desc);
    if (ret != 0) {
        AUDIO_ERR_LOG("ReqMmapBuffer failed, ret:%{public}d", ret);
        return ERR_OPERATION_FAILED;
    }
    AUDIO_INFO_LOG("AudioMmapBufferDescriptor capturer memoryAddress[%{private}p] memoryFd[%{public}d] totalBufferFrames"
        "[%{public}d] transferFrameSize[%{public}d] isShareable[%{public}d] offset[%{public}d]", desc.memoryAddress,
        desc.memoryFd, desc.totalBufferFrames, desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030,3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    if (desc.totalBufferFrames < 0 || desc.transferFrameSize < 0 || desc.transferFrameSize > periodFrameMaxSize) {
        AUDIO_ERR_LOG("ReqMmapBuffer invalid values: totalBufferFrames[%{public}d] transferFrameSize[%{public}d]",
            desc.totalBufferFrames, desc.transferFrameSize);
        return ERR_OPERATION_FAILED;
    }
    bufferTotalFrameSize_ = desc.totalBufferFrames; // 1440 ~ 3840
    eachReadFrameSize_ = desc.transferFrameSize; // 240

    if (frameSizeInByte_ > ULLONG_MAX / bufferTotalFrameSize_) {
        AUDIO_ERR_LOG("BufferSize will overflow!");
        return ERR_OPERATION_FAILED;
    }
    bufferSize_ = bufferTotalFrameSize_ * frameSizeInByte_;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::Init(IAudioSourceAttr &attr)
{
    if (InitAudioManager() != 0) {
        AUDIO_ERR_LOG("Init audio manager Fail");
        return ERR_INVALID_HANDLE;
    }
    attr_ = attr;
    int32_t ret;
    int32_t index;
    int32_t size = MAX_AUDIO_ADAPTER_NUM;
    struct AudioAdapterDescriptor *descs = nullptr;
    ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);
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
    if (CreateCapture(audioPort) != SUCCESS || PrepareMmapBuffer() != SUCCESS) {
        AUDIO_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }
    if (openMic_) {
        ret = SetInputRoute(DEVICE_TYPE_MIC);
        if (ret < 0) {
            AUDIO_ERR_LOG("FastAudioCapturerSourceInner:update route FAILED: %{public}d", ret);
        }
    }
    capturerInited_ = true;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    AUDIO_ERR_LOG("CaptureFrame in fast mode is not supported!");
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::Start(void)
{
    AUDIO_INFO_LOG("Start.");
    if (mKeepRunningLock == nullptr) {
        mKeepRunningLock = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioFastCapturer",
            PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
    }
    if (mKeepRunningLock != nullptr) {
        AUDIO_INFO_LOG("FastAudioCapturerSourceInner call KeepRunningLock lock");
        mKeepRunningLock->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
    } else {
        AUDIO_ERR_LOG("mKeepRunningLock is null, start can not work well!");
    }

    int32_t ret;
    if (!started_) {
        ret = audioCapture_->control.Start(audioCapture_);
        if (ret < 0) {
            return ERR_NOT_STARTED;
        }
        started_ = true;
    }

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::SetVolume(float left, float right)
{
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::GetVolume(float &left, float &right)
{
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::SetMute(bool isMute)
{    AUDIO_ERR_LOG("SetMute in fast mode is not supported!");
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::GetMute(bool &isMute)
{
    AUDIO_ERR_LOG("SetMute in fast mode is not supported!");
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice)
{
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin)
{
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t FastAudioCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    return ERR_DEVICE_NOT_SUPPORTED;
}

uint64_t FastAudioCapturerSourceInner::GetTransactionId()
{
    AUDIO_INFO_LOG("FastAudioCapturerSourceInner::GetTransactionId in");
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t FastAudioCapturerSourceInner::Stop(void)
{
    AUDIO_INFO_LOG("Stop.");

    if (mKeepRunningLock != nullptr) {
        AUDIO_INFO_LOG("FastAudioCapturerSourceInner call KeepRunningLock UnLock");
        mKeepRunningLock->UnLock();
    } else {
        AUDIO_ERR_LOG("mKeepRunningLock is null, stop can not work well!");
    }

    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Stop(audioCapture_);
        if (ret < 0) {
            AUDIO_ERR_LOG("Stop capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    started_ = false;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::Pause(void)
{
    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Pause(audioCapture_);
        if (ret != 0) {
            AUDIO_ERR_LOG("pause capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = true;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::Resume(void)
{
    int32_t ret;
    if (paused_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Resume(audioCapture_);
        if (ret != 0) {
            AUDIO_ERR_LOG("resume capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = false;

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::Reset(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(audioCapture_);
    }

    return SUCCESS;
}

int32_t FastAudioCapturerSourceInner::Flush(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(audioCapture_);
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namesapce OHOS
