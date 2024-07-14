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

#include <mutex>
#include <string>
#include <cinttypes>

#include "audio_capturer_source.h"
#include "audio_capturer_source_normal.h"

class AudioCapturerSourceWakeup : public AudioCapturerSource {
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

    int32_t SetInputRoute(DeviceType inputDevice) override;
    uint64_t GetTransactionId() override;
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override;
    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback *callback) override;
    void RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback) override;
    void RegisterParameterCallback(IAudioSourceCallback *callback) override;
    float GetMaxAmplitude() override;

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
        const size_t size) final;
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) final;

    AudioCapturerSourceWakeup() = default;
    ~AudioCapturerSourceWakeup() = default;

private:
    class WakeupBuffer {
    public:
        explicit WakeupBuffer(size_t sizeMax = BUFFER_SIZE_MAX);

        ~WakeupBuffer() = default;

        int32_t Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &noStart);
    private:
        static constexpr size_t BUFFER_SIZE_MAX = 32000; // 2 seconds

        const size_t sizeMax_;
        size_t size_ = 0;

        std::unique_ptr<char[]> buffer_;
        std::mutex mutex_;

        uint64_t head_ = 0;

        uint64_t headNum_ = 0;

        void Offer(const char *frame, const uint64_t bufferBytes);
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