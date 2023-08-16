/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "securec.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
class AudioCapturerSourceInner : public AudioCapturerSource {
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

    void RegisterWakeupCloseCallback(IAudioSourceCallback* callback) override;
    void RegisterAudioCapturerSourceCallback(IAudioSourceCallback* callback) override;

    AudioCapturerSourceInner();
    ~AudioCapturerSourceInner();

private:
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    static constexpr uint32_t PRIMARY_INPUT_STREAM_ID = 14; // 14 + 0 * 8

    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t InitAudioManager();
    void InitAttrsCapture(struct AudioSampleAttributes &attrs);
    void OpenDumpFile();
    void SetAudioDumpBySysParam();

    IAudioSourceAttr attr_;
    bool capturerInited_;
    bool started_;
    bool paused_;
    float leftVolume_;
    float rightVolume_;

    int32_t routeHandle_ = -1;
    uint32_t openMic_;
    uint32_t captureId_ = 0;
    std::string adapterNameCase_;
    struct IAudioManager *audioManager_;
    struct IAudioAdapter *audioAdapter_;
    struct IAudioCapture *audioCapture_;
    struct AudioAdapterDescriptor adapterDesc_;
    struct AudioPort audioPort;

    std::shared_ptr<PowerMgr::RunningLock> keepRunningLock_;

    IAudioSourceCallback* wakeupCloseCallback_ = nullptr;
    std::mutex wakeupClosecallbackMutex_;

    IAudioSourceCallback* audioCapturerSourceCallback_ = nullptr;
    std::mutex audioCapturerSourceCallbackMutex_;
    bool enableDump_ = false;
    FILE *dumpFile_ = nullptr;
    uint32_t dumpCount = 0;
#ifdef CAPTURE_DUMP
    FILE *pfd_;
#endif
};

class AudioCapturerSourceWakeup : public AudioCapturerSource {
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

    int32_t SetInputRoute(DeviceType inputDevice) override;
    uint64_t GetTransactionId() override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback* callback) override;
    void RegisterAudioCapturerSourceCallback(IAudioSourceCallback* callback) override;

    AudioCapturerSourceWakeup() = default;
    ~AudioCapturerSourceWakeup() = default;

private:
    static inline void MemcpysAndCheck(void *dest, size_t destMax, const void *src, size_t count)
    {
        if (memcpy_s(dest, destMax, src, count)) {
            AUDIO_ERR_LOG("memcpy_s error");
        }
    }
    class WakeupBuffer {
    public:
        explicit WakeupBuffer(size_t sizeMax = BUFFER_SIZE_MAX)
            : sizeMax_(sizeMax),
              buffer_(std::make_unique<char[]>(sizeMax))
        {
        }

        ~WakeupBuffer() = default;

        int32_t Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &noStart)
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (noStart < headNum_) {
                noStart = headNum_;
            }

            if (noStart >= (headNum_ + size_)) {
                if (requestBytes > sizeMax_) {
                    requestBytes = sizeMax_;
                }

                int32_t res = audioCapturerSource_.CaptureFrame(frame, requestBytes, replyBytes);
                Offer(frame, replyBytes);

                return res;
            }

            if (requestBytes > size_) { // size_!=0
                replyBytes = size_;
            } else {
                replyBytes = requestBytes;
            }

            uint64_t tail = (head_ + size_) % sizeMax_;

            if (tail > head_) {
                MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, replyBytes);
                headNum_ += replyBytes;
                size_ -= replyBytes;
                head_ = (head_ + replyBytes) % sizeMax_;
            } else {
                uint64_t copySize = min((sizeMax_ - head_), replyBytes);
                if (copySize != 0) {
                    MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, copySize);
                    headNum_ += copySize;
                    size_ -= copySize;
                    head_ = (head_ + copySize) % sizeMax_;
                }

                uint64_t remainCopySize = replyBytes - copySize;
                if (remainCopySize != 0) {
                    MemcpysAndCheck(frame + copySize, remainCopySize, buffer_.get(), remainCopySize);
                    headNum_ += remainCopySize;
                    size_ -= remainCopySize;
                    head_ = (head_ + remainCopySize) % sizeMax_;
                }
            }

            return SUCCESS;
        }
    private:
        static constexpr size_t BUFFER_SIZE_MAX = 32000; // 2 seconds

        const size_t sizeMax_;
        size_t size_ = 0;

        std::unique_ptr<char[]> buffer_;
        std::mutex mutex_;

        uint64_t head_ = 0;

        uint64_t headNum_ = 0;

        void Offer(const char *frame, const uint64_t bufferBytes)
        {
            if ((size_ + bufferBytes) > sizeMax_) { // head_ need shift
                u_int64_t shift = (size_ + bufferBytes) - sizeMax_; // 1 to sizeMax_
                headNum_ += shift;
                if (size_ > shift) {
                    size_ -= shift;
                    head_ = ((head_ + shift) % sizeMax_);
                } else {
                    size_ = 0;
                    head_ = 0;
                }
            }

            uint64_t tail = (head_ + size_) % sizeMax_;
            if (tail < head_) {
                MemcpysAndCheck((buffer_.get() + tail), bufferBytes, frame, bufferBytes);
            } else {
                uint64_t copySize = min(sizeMax_ - tail, bufferBytes);
                MemcpysAndCheck((buffer_.get() + tail), sizeMax_ - tail, frame, copySize);

                if (copySize < bufferBytes) {
                    MemcpysAndCheck((buffer_.get()), bufferBytes - copySize, frame + copySize, bufferBytes - copySize);
                }
            }
            size_ += bufferBytes;
        }
    };

    uint64_t noStart_ = 0;
    std::atomic<bool> isInited = false;
    static inline int initCount = 0;

    std::atomic<bool> isStarted = false;
    static inline int startCount = 0;

    static inline std::unique_ptr<WakeupBuffer> wakeupBuffer_;
    static inline std::mutex wakeupMutex_;

    static inline AudioCapturerSourceInner audioCapturerSource_;
};

#ifdef CAPTURE_DUMP
const char *g_audioOutTestFilePath = "/data/data/.pulse_dir/dump_audiosource.pcm";
#endif // CAPTURE_DUMP
bool AudioCapturerSource::micMuteState_ = false;
constexpr int32_t RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING = -1;

AudioCapturerSourceInner::AudioCapturerSourceInner()
    : capturerInited_(false), started_(false), paused_(false), leftVolume_(MAX_VOLUME_LEVEL),
      rightVolume_(MAX_VOLUME_LEVEL), openMic_(0), audioManager_(nullptr), audioAdapter_(nullptr),
      audioCapture_(nullptr)
{
    attr_ = {};
#ifdef CAPTURE_DUMP
    pfd_ = nullptr;
#endif // CAPTURE_DUMP
}

AudioCapturerSourceInner::~AudioCapturerSourceInner()
{
    AUDIO_ERR_LOG("~AudioCapturerSourceInner");
}

AudioCapturerSource *AudioCapturerSource::GetInstance(const SourceType sourceType, const char *sourceName)
{
    switch (sourceType) {
        case SourceType::SOURCE_TYPE_MIC:
            return GetMicInstance();
        case SourceType::SOURCE_TYPE_WAKEUP:
            if (!strcmp(sourceName, "Built_in_wakeup_mirror")) {
                return GetWakeupInstance(true);
            } else {
                return GetWakeupInstance(false);
            }
        default:
            AUDIO_ERR_LOG("sourceType error %{public}d", sourceType);
            return GetMicInstance();
    }
}

static enum AudioInputType ConvertToHDIAudioInputType(const int32_t currSourceType)
{
    enum AudioInputType hdiAudioInputType;
    switch (currSourceType) {
        case SOURCE_TYPE_INVALID:
            hdiAudioInputType = AUDIO_INPUT_DEFAULT_TYPE;
            break;
        case SOURCE_TYPE_MIC:
        case SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SOURCE_TYPE_ULTRASONIC:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
        case SOURCE_TYPE_WAKEUP:
            hdiAudioInputType = AUDIO_INPUT_SPEECH_WAKEUP_TYPE;
            break;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_COMMUNICATION_TYPE;
            break;
        case SOURCE_TYPE_VOICE_RECOGNITION:
            hdiAudioInputType = AUDIO_INPUT_VOICE_RECOGNITION_TYPE;
            break;
        default:
            hdiAudioInputType = AUDIO_INPUT_MIC_TYPE;
            break;
    }
    return hdiAudioInputType;
}

AudioCapturerSource *AudioCapturerSource::GetMicInstance()
{
    static AudioCapturerSourceInner audioCapturer;
    return &audioCapturer;
}

AudioCapturerSource *AudioCapturerSource::GetWakeupInstance(bool isMirror)
{
    if (isMirror) {
        static AudioCapturerSourceWakeup audioCapturerMirror;
        return &audioCapturerMirror;
    }
    static AudioCapturerSourceWakeup audioCapturer;
    return &audioCapturer;
}

bool AudioCapturerSourceInner::IsInited(void)
{
    return capturerInited_;
}

void AudioCapturerSourceInner::DeInit()
{
    started_ = false;
    capturerInited_ = false;

    if (audioAdapter_ != nullptr) {
        audioAdapter_->DestroyCapture(audioAdapter_, captureId_);
    }

    IAudioSourceCallback* callback = nullptr;
    {
        std::lock_guard<std::mutex> lck(wakeupClosecallbackMutex_);
        callback = wakeupCloseCallback_;
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
    if (dumpFile_) {
        fclose(dumpFile_);
        dumpFile_ = nullptr;
    }
#ifdef CAPTURE_DUMP
    if (pfd_) {
        fclose(pfd_);
        pfd_ = nullptr;
    }
#endif // CAPTURE_DUMP
}

void AudioCapturerSourceInner::InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = true;
    attrs.streamId = PRIMARY_INPUT_STREAM_ID;
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

int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size, const std::string &adapterNameCase,
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

int32_t AudioCapturerSourceInner::InitAudioManager()
{
    AUDIO_INFO_LOG("Initialize audio proxy manager");

    audioManager_ = IAudioManagerGet(false);
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

int32_t AudioCapturerSourceInner::CreateCapture(struct AudioPort &capturePort)
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
    param.sourceType = static_cast<int32_t>(ConvertToHDIAudioInputType(attr_.sourceType));

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

int32_t AudioCapturerSourceInner::Init(IAudioSourceAttr &attr)
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
            AUDIO_ERR_LOG("update route FAILED: %{public}d", ret);
        }
    }
    capturerInited_ = true;

    OpenDumpFile();
    return SUCCESS;
}

void AudioCapturerSourceInner::OpenDumpFile()
{
#ifdef CAPTURE_DUMP
    pfd_ = fopen(g_audioOutTestFilePath, "wb+");
    if (pfd_ == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
#endif // CAPTURE_DUMP
}

int32_t AudioCapturerSourceInner::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
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

    if (dumpFile_) {
        size_t writeResult = fwrite(frame, 1, replyBytes, dumpFile_);
        if (writeResult != replyBytes) {
            AUDIO_ERR_LOG("Failed to write the file.");
        }
    }
#ifdef CAPTURE_DUMP
    if (pfd_) {
        size_t writeResult = fwrite(frame, 1, replyBytes, pfd_);
        if (writeResult != replyBytes) {
            AUDIO_ERR_LOG("Failed to write the file.");
        }
    }
#endif // CAPTURE_DUMP

    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_DEBUG_LOG("RenderFrame len[%{public}" PRIu64 "] cost[%{public}" PRId64 "]ms", requestBytes, stamp);
    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Start(void)
{
    AUDIO_INFO_LOG("Start.");
    if (keepRunningLock_ == nullptr) {
        switch (attr_.sourceType) {
            case SOURCE_TYPE_WAKEUP:
                keepRunningLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioWakeupCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
                break;
            case SOURCE_TYPE_MIC:
            default:
                keepRunningLock_ = PowerMgr::PowerMgrClient::GetInstance().CreateRunningLock("AudioPrimaryCapturer",
                    PowerMgr::RunningLockType::RUNNINGLOCK_BACKGROUND_AUDIO);
        }
    }
    if (keepRunningLock_ != nullptr) {
        AUDIO_INFO_LOG("AudioCapturerSourceInner call KeepRunningLock lock");
        keepRunningLock_->Lock(RUNNINGLOCK_LOCK_TIMEOUTMS_LASTING); // -1 for lasting.
    } else {
        AUDIO_ERR_LOG("keepRunningLock_ is null, start can not work well!");
    }

    SetAudioDumpBySysParam();
    if (enableDump_ && dumpFile_ == nullptr) {
        std::string dumpFilePath = "/data/data/.pulse_dir/dump_audiocapture_" +
        std::to_string(dumpCount) + ".pcm";
        dumpFile_ = fopen(dumpFilePath.c_str(), "wb+");
        if (dumpFile_ == nullptr) {
            AUDIO_ERR_LOG("Error opening dump file!");
        } else {
            dumpCount++;
        }
    }

    int32_t ret;
    if (!started_) {
        IAudioSourceCallback* callback = nullptr;
        {
            std::lock_guard<std::mutex> lck(audioCapturerSourceCallbackMutex_);
            callback = audioCapturerSourceCallback_;
        }
        if (callback != nullptr) {
            callback->OnCapturerState(true);
        }

        ret = audioCapture_->Start(audioCapture_);
        if (ret < 0) {
            return ERR_NOT_STARTED;
        }
        started_ = true;
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetVolume(float left, float right)
{
    float volume;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed audioCapture_ null");
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

int32_t AudioCapturerSourceInner::GetVolume(float &left, float &right)
{
    float val = 0.0;
    audioCapture_->GetVolume(audioCapture_, &val);
    left = val;
    right = val;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetMute(bool isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("SetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    ret = audioCapture_->SetMute(audioCapture_, isMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("SetMute failed from hdi");
    }

    if (audioAdapter_ != nullptr) {
        ret = audioAdapter_->SetMicMute(audioAdapter_, isMute);
        if (ret != 0) {
            AUDIO_ERR_LOG("SetMicMute failed from hdi");
        }
    }

    AudioCapturerSource::micMuteState_ = isMute;

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::GetMute(bool &isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("GetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    bool isHdiMute = false;
    ret = audioCapture_->GetMute(audioCapture_, &isHdiMute);
    if (ret != 0) {
        AUDIO_ERR_LOG("GetMute failed from hdi");
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
    AUDIO_DEBUG_LOG("Audio category returned is: %{public}d", audioCategory);

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

int32_t AudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice)
{
    AudioPortPin inputPortPin = PIN_IN_MIC;
    return SetInputRoute(inputDevice, inputPortPin);
}

int32_t AudioCapturerSourceInner::SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin)
{
    AudioRouteNode source = {};
    AudioRouteNode sink = {};

    int32_t ret = SetInputPortPin(inputDevice, source);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetOutputRoute FAILED: %{public}d", ret);
        return ret;
    }

    inputPortPin = source.ext.device.type;
    AUDIO_INFO_LOG("Input PIN is: 0x%{public}X", inputPortPin);
    source.portId = static_cast<int32_t>(audioPort.portId);
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;
    source.ext.device.desc = (char *)"";

    sink.portId = 0;
    sink.role = AUDIO_PORT_SINK_ROLE;
    sink.type = AUDIO_PORT_MIX_TYPE;
    sink.ext.mix.moduleId = 0;
    sink.ext.mix.streamId = PRIMARY_INPUT_STREAM_ID;
    sink.ext.device.desc = (char *)"";

    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = &sink,
        .sinksLen = 1,
    };

    if (audioAdapter_ == nullptr) {
        AUDIO_ERR_LOG("AudioAdapter object is null.");
        return ERR_OPERATION_FAILED;
    }

    ret = audioAdapter_->UpdateAudioRoute(audioAdapter_, &route, &routeHandle_);
    if (ret != 0) {
        AUDIO_ERR_LOG("UpdateAudioRoute failed");
        return ERR_OPERATION_FAILED;
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    AUDIO_INFO_LOG("SetAudioScene scene: %{public}d, device: %{public}d",
        audioScene, activeDevice);
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene <= AUDIO_SCENE_PHONE_CHAT,
        ERR_INVALID_PARAM, "invalid audioScene");
    if (audioCapture_ == nullptr) {
        AUDIO_ERR_LOG("SetAudioScene failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }
    if (openMic_) {
        AudioPortPin audioSceneInPort = PIN_IN_MIC;
        int32_t ret = SetInputRoute(activeDevice, audioSceneInPort);
        if (ret < 0) {
            AUDIO_ERR_LOG("Update route FAILED: %{public}d", ret);
        }
        struct AudioSceneDescriptor scene;
        scene.scene.id = GetAudioCategory(audioScene);
        scene.desc.pins = audioSceneInPort;
        scene.desc.desc = (char *)"";

        ret = audioCapture_->SelectScene(audioCapture_, &scene);
        if (ret < 0) {
            AUDIO_ERR_LOG("Select scene FAILED: %{public}d", ret);
            return ERR_OPERATION_FAILED;
        }
    }
    AUDIO_DEBUG_LOG("Select audio scene SUCCESS: %{public}d", audioScene);
    return SUCCESS;
}

uint64_t AudioCapturerSourceInner::GetTransactionId()
{
    AUDIO_INFO_LOG("GetTransactionId in");
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t AudioCapturerSourceInner::Stop(void)
{
    AUDIO_INFO_LOG("Stop.");

    if (keepRunningLock_ != nullptr) {
        AUDIO_INFO_LOG("AudioCapturerSourceInner call KeepRunningLock UnLock");
        keepRunningLock_->UnLock();
    } else {
        AUDIO_ERR_LOG("keepRunningLock_ is null, stop can not work well!");
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

    IAudioSourceCallback* callback = nullptr;
    {
        std::lock_guard<std::mutex> lck(audioCapturerSourceCallbackMutex_);
        callback = audioCapturerSourceCallback_;
    }
    if (callback != nullptr) {
        callback->OnCapturerState(false);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Pause(void)
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

int32_t AudioCapturerSourceInner::Resume(void)
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

int32_t AudioCapturerSourceInner::Reset(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

int32_t AudioCapturerSourceInner::Flush(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->Flush(audioCapture_);
    }

    return SUCCESS;
}

void AudioCapturerSourceInner::RegisterWakeupCloseCallback(IAudioSourceCallback* callback)
{
    AUDIO_INFO_LOG("Register WakeupClose Callback");
    std::lock_guard<std::mutex> lck(wakeupClosecallbackMutex_);
    wakeupCloseCallback_ = callback;
}

void AudioCapturerSourceInner::RegisterAudioCapturerSourceCallback(IAudioSourceCallback* callback)
{
    AUDIO_INFO_LOG("Register AudioCapturerSource Callback");
    std::lock_guard<std::mutex> lck(audioCapturerSourceCallbackMutex_);
    audioCapturerSourceCallback_ = callback;
}

int32_t AudioCapturerSourceWakeup::Init(IAudioSourceAttr &attr)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isInited) {
        return res;
    }
    noStart_ = 0;
    if (initCount == 0) {
        if (wakeupBuffer_ == nullptr) {
            wakeupBuffer_ = std::make_unique<WakeupBuffer>();
        }
        res = audioCapturerSource_.Init(attr);
    }
    if (res == SUCCESS) {
        isInited = true;
        initCount++;
    }
    return res;
}

bool AudioCapturerSourceWakeup::IsInited(void)
{
    return isInited;
}

void AudioCapturerSourceWakeup::DeInit(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (!isInited) {
        return;
    }
    isInited = false;
    initCount--;
    if (initCount == 0) {
        wakeupBuffer_.reset();
        audioCapturerSource_.DeInit();
    }
}

int32_t AudioCapturerSourceWakeup::Start(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isStarted) {
        return res;
    }
    if (startCount == 0) {
        res = audioCapturerSource_.Start();
    }
    if (res == SUCCESS) {
        isStarted = true;
        startCount++;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Stop(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (!isStarted) {
        return res;
    }
    if (startCount == 1) {
        res = audioCapturerSource_.Stop();
    }
    if (res == SUCCESS) {
        isStarted = false;
        startCount--;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Flush(void)
{
    return audioCapturerSource_.Flush();
}

int32_t AudioCapturerSourceWakeup::Reset(void)
{
    return audioCapturerSource_.Reset();
}

int32_t AudioCapturerSourceWakeup::Pause(void)
{
    return audioCapturerSource_.Pause();
}

int32_t AudioCapturerSourceWakeup::Resume(void)
{
    return audioCapturerSource_.Resume();
}

int32_t AudioCapturerSourceWakeup::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t res = wakeupBuffer_->Poll(frame, requestBytes, replyBytes, noStart_);
    noStart_ += replyBytes;
    return res;
}

int32_t AudioCapturerSourceWakeup::SetVolume(float left, float right)
{
    return audioCapturerSource_.SetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::GetVolume(float &left, float &right)
{
    return audioCapturerSource_.GetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::SetMute(bool isMute)
{
    return audioCapturerSource_.SetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::GetMute(bool &isMute)
{
    return audioCapturerSource_.GetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    return audioCapturerSource_.SetAudioScene(audioScene, activeDevice);
}

int32_t AudioCapturerSourceWakeup::SetInputRoute(DeviceType inputDevice)
{
    return audioCapturerSource_.SetInputRoute(inputDevice);
}

uint64_t AudioCapturerSourceWakeup::GetTransactionId()
{
    return audioCapturerSource_.GetTransactionId();
}

void AudioCapturerSourceWakeup::RegisterWakeupCloseCallback(IAudioSourceCallback* callback)
{
    audioCapturerSource_.RegisterWakeupCloseCallback(callback);
}

void AudioCapturerSourceWakeup::RegisterAudioCapturerSourceCallback(IAudioSourceCallback* callback)
{
    audioCapturerSource_.RegisterAudioCapturerSourceCallback(callback);
}

void AudioCapturerSourceInner::SetAudioDumpBySysParam(void)
{
    std::string dumpEnable;
    enableDump_ = false;
    bool res = GetSysPara("sys.media.dump.audioframe.write.enable", dumpEnable);
    if (!res || dumpEnable.empty()) {
        AUDIO_INFO_LOG("sys.media.dump.audioframe.write.enable is not set, dump audio is not required");
        if (dumpFile_) {
            fclose(dumpFile_);
            dumpFile_ = nullptr;
        }
        return;
    }
    AUDIO_INFO_LOG("sys.media.dump.audioframe.write.enable=%s", dumpEnable.c_str());
    if (dumpEnable == "true") {
        enableDump_ = true;
        return;
    }
    if (dumpFile_) {
        fclose(dumpFile_);
        dumpFile_ = nullptr;
    }
}
} // namespace AudioStandard
} // namesapce OHOS
