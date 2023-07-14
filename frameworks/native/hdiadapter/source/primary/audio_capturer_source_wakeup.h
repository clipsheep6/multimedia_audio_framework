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

#ifndef AUDIO_CAPTURER_SOURCE_WAKEUP_H
#define AUDIO_CAPTURER_SOURCE_WAKEUP_H

#include "audio_capturer_source.h"

namespace OHOS {
namespace AudioStandard {
class WakeupBuffer {
public:
//构造复制赋值
    poll()
    {

    }
    offer()
    {
        
    }
private:
    static constexpr size_t BUFFER_SIZE = 32000; //2 seconds

    size_t size_ = BUFFER_SIZE;
    std::unique_ptr<uint8_t[]> buffer_ = std::make_unique<uint8_t[]>(BUFFER_SIZE);
    // uint8_t buffer_[BUFFER_SIZE];
}
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

    int32_t SetInputRoute(DeviceType inputDevice, AudioPortPin &inputPortPin);

    int32_t SetInputRoute(DeviceType inputDevice) override;
    uint64_t GetTransactionId() override;

    void RegisterWakeupCloseCallback(IAudioSourceCallback* callback) override;
    AudioCapturerSourceWakeup();
    ~AudioCapturerSourceWakeup();
private:
    static constexpr int32_t HALF_FACTOR = 2;
    static constexpr uint32_t MAX_AUDIO_ADAPTER_NUM = 5;
    static constexpr float MAX_VOLUME_LEVEL = 15.0f;
    
    static inline std::unique_ptr<uint8_t[]> wakeupBuffer;
    static lastWriteToBufferTime

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

    std::shared_ptr<PowerMgr::RunningLock> mKeepRunningLock;

    IAudioSourceCallback* callback_ = nullptr;
    std::mutex callbackMutex_;

    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t InitAudioManager();
    void OpenDumpFile();

#ifdef CAPTURE_DUMP
    FILE *pfd;
#endif
};

#ifdef CAPTURE_DUMP
const char *g_audioOutTestFilePath = "/data/data/.pulse_dir/dump_audiosource.pcm";
#endif // CAPTURE_DUMP
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_SOURCE_WAKEUP_H