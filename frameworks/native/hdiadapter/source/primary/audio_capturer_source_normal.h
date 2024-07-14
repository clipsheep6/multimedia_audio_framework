/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {

class AudioCapturerSourceInner : public AudioCapturerSource {
public:
    int32_t Init(const IAudioSourceAttr &attr) override;
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
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;

    int32_t Preload(const std::string &usbInfoStr) override;
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    explicit AudioCapturerSourceInner(const std::string &halName = "primary");
    ~AudioCapturerSourceInner();

private:
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    static constexpr uint32_t PRIMARY_INPUT_STREAM_ID = 14; // 14 + 0 * 8
    static constexpr uint32_t USB_DEFAULT_BUFFERSIZE = 3840;
    static constexpr uint32_t STEREO_CHANNEL_COUNT = 2;

    int32_t CreateCapture(struct AudioPort &capturePort);
    void InitAttrsCapture(struct AudioSampleAttributes &attrs);
    int32_t InitAudioManager();
    int32_t InitManagerAndAdapter();
    int32_t InitAdapterAndCapture();
    int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, uint32_t size,
        const std::string &adapterNameCase, enum AudioPortDirection portFlag, struct AudioPort &capturePort)

    int32_t UpdateUsbAttrs(const std::string &usbInfoStr);

    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckLatencySignal(uint8_t *frame, size_t replyBytes);

    void CheckUpdateState(char *frame, uint64_t replyBytes);

    IAudioSourceAttr attr_ = {};
    bool sourceInited_ = false;
    bool captureInited_ = false;
    bool started_ = false;
    bool paused_ = false;
    float leftVolume_ = 0.0f;
    float rightVolume_ = 0.0f;

    int32_t routeHandle_ = -1;
    int32_t logMode_ = 0;
    uint32_t openMic_ = 0;
    uint32_t captureId_ = 0;
    std::string adapterNameCase_ = "";

    // for get amplitude
    float maxAmplitude_ = 0;
    int64_t lastGetMaxAmplitudeTime_ = 0;
    int64_t last10FrameStartTime_ = 0;
    bool startUpdate_ = false;
    int capFrameNum_ = 0;

    struct IAudioManager *audioManager_ = nullptr;
    std::atomic<bool> adapterLoaded_ = false;
    struct IAudioAdapter *audioAdapter_ = nullptr;
    struct IAudioCapture *audioCapture_ = nullptr;
    const std::string halName_ = "";
    struct AudioAdapterDescriptor adapterDesc_ = {};
    struct AudioPort audioPort_ = {};
#ifdef FEATURE_POWER_MANAGER
    std::shared_ptr<AudioRunningLockManager<PowerMgr::RunningLock>> runningLockManager_;
#endif
    IAudioSourceCallback* wakeupCloseCallback_ = nullptr;
    std::mutex wakeupClosecallbackMutex_;

    std::unique_ptr<ICapturerStateCallback> audioCapturerSourceCallback_ = nullptr;
    FILE *dumpFile_ = nullptr;
    bool muteState_ = false;
    DeviceType currentActiveDevice_ = DEVICE_TYPE_INVALID;
    AudioScene currentAudioScene_ = AUDIO_SCENE_INVALID;
    bool latencyMeasEnabled_ = false;
    bool signalDetected_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
    std::mutex managerAndAdapterMutex_;
};

} // namespace AudioStandard
} // namesapce OHOS
