/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <mutex>
#include <condition_variable>
#include "timestamp.h"
#include "event_handler.h"
#include "event_runner.h"
#include "audio_info.h"
#include "audio_service_client.h"
#include "audio_stream_tracker.h"
#define DUMP_AUDIOSTREAM_PCM
namespace OHOS {
namespace AudioStandard {
static constexpr int32_t MAX_WRITECB_NUM_BUFFERS = 1;
static constexpr int32_t MAX_READCB_NUM_BUFFERS = 3;
static constexpr int32_t TWO_BYTE_PER_SAMPLE = 2;
static constexpr int32_t THREE_BYTE_PER_SAMPLE = 3;
static constexpr int32_t FOUR_BYTE_PER_SAMPLE = 4;
static constexpr int32_t CHANNEL_ONE = 0;
static constexpr int32_t CHANNEL_TWO = 1;
static constexpr int32_t CHANNEL_THREE = 2;
static constexpr int32_t CHANNEL_FOUR = 3;
static constexpr int32_t CHANNEL_FIVE = 4;
static constexpr int32_t CHANNEL_SIX = 5;
static constexpr int32_t CHANNEL_SEVEN = 6;
static constexpr int32_t CHANNEL_EIGHT = 7;
static constexpr int32_t NUMBER_TWO = 2;
typedef struct {
    int8_t value[3];
} __attribute__((__packed__)) int24_t;

class AudioStream : public AudioServiceClient {
public:
    AudioStream(AudioStreamType eStreamType, AudioMode eMode, int32_t appUid);
    virtual ~AudioStream();

    void SetRendererInfo(const AudioRendererInfo &rendererInfo) override;
    void SetCapturerInfo(const AudioCapturerInfo &capturerInfo) override;
    int32_t SetAudioStreamInfo(const AudioStreamParams info,
        const std::shared_ptr<AudioClientTracker> &proxyObj) override;
    int32_t GetAudioStreamInfo(AudioStreamParams &info) override;
    int32_t GetAudioSessionID(uint32_t &sessionID) override;
    State GetState() override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    int32_t GetBufferSize(size_t &bufferSize) override;
    int32_t GetFrameCount(uint32_t &frameCount) override;
    int32_t GetLatency(uint64_t &latency) override;
    int32_t SetAudioStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) override;
    float GetVolume() override;
    int32_t SetRenderRate(AudioRendererRate renderRate) override;
    AudioRendererRate GetRenderRate() override;
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback) override;

    // callback mode api
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override;
    AudioCaptureMode GetCaptureMode() override;
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t GetBufQueueState(BufferQueueState &bufState) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear() override;
    void SubmitAllFreeBuffers();

    void SetInnerCapturerState(bool isInnerCapturer) override;
    void SetPrivacyType(AudioPrivacyType privacyType) override;

    int32_t SetLowPowerVolume(float volume) override;
    float GetLowPowerVolume() override;
    float GetSingleStreamVolume() override;
    AudioEffectMode GetAudioEffectMode() override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) override;
    int64_t GetFramesWritten() override;
    int64_t GetFramesRead() override;

    std::vector<AudioSampleFormat> GetSupportedFormats() const;
    std::vector<AudioEncodingType> GetSupportedEncodingTypes() const;
    std::vector<AudioSamplingRate> GetSupportedSamplingRates() const;

    // Common APIs
    bool StartAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool PauseAudioStream(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool StopAudioStream() override;
    bool ReleaseAudioStream(bool releaseRunner = true) override;
    bool FlushAudioStream() override;

    // Playback related APIs
    bool DrainAudioStream() override;
    size_t Write(uint8_t *buffer, size_t buffer_size) override;

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override;

    void SetChannelBlendMode(ChannelBlendMode blendMode) override;
    void ProcessPcmDataByBlendMode(uint8_t *buffer, size_t buffer_size);
    void ProcessDataByBlendLR(uint8_t *buffer, size_t buffer_size);
    void ProcessDataByAllLeftMode(uint8_t *buffer, size_t buffer_size);
    void ProcessDataByAllRightMode(uint8_t *buffer, size_t buffer_size);
private:
    AudioStreamType eStreamType_;
    AudioMode eMode_;
    State state_;
    bool resetTime_;
    uint64_t resetTimestamp_;
    struct timespec baseTimestamp_ = {0};
    AudioRenderMode renderMode_;
    AudioCaptureMode captureMode_;
    std::queue<BufferDesc> freeBufferQ_;
    std::queue<BufferDesc> filledBufferQ_;
    std::array<std::unique_ptr<uint8_t[]>, MAX_WRITECB_NUM_BUFFERS> writeBufferPool_ = {};
    std::array<std::unique_ptr<uint8_t[]>, MAX_READCB_NUM_BUFFERS> readBufferPool_ = {};
    std::unique_ptr<std::thread> writeThread_ = nullptr;
    std::unique_ptr<std::thread> readThread_ = nullptr;
    bool isReadyToWrite_;
    bool isReadyToRead_;
    void WriteCbTheadLoop();
    void ReadCbThreadLoop();
    std::unique_ptr<AudioStreamTracker> audioStreamTracker_;
    AudioRendererInfo rendererInfo_;
    AudioCapturerInfo capturerInfo_;
    uint32_t sessionId_;

    bool isFirstRead_;
    bool isFirstWrite_;
    std::mutex bufferQueueLock_;
    std::condition_variable bufferQueueCV_;
    AudioStreamParams streamParams_;
    ChannelBlendMode blendMode_;
#ifdef DUMP_AUDIOSTREAM_PCM
    FILE *pfd_;
#endif
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_H
