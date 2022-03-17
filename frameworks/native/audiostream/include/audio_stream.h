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

#include "audio_renderer.h"
#include "audio_session.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t MAX_NUM_BUFFERS = 3;

enum AudioMode {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

class AudioStream : public AudioSession {
public:
    AudioStream(AudioStreamType eStreamType, AudioMode eMode);
    virtual ~AudioStream();

    int32_t SetAudioStreamInfo(const AudioStreamParams info);
    int32_t GetAudioStreamInfo(AudioStreamParams &info);

    int32_t GetAudioSessionID(uint32_t &sessionID);
    State GetState();
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base);
    int32_t GetBufferSize(size_t &bufferSize);
    int32_t GetFrameCount(uint32_t &frameCount);
    int32_t GetLatency(uint64_t &latency);
    static AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
    int32_t SetAudioStreamType(AudioStreamType audioStreamType);
    int32_t SetVolume(float volume);
    float GetVolume();
    int32_t SetRenderRate(AudioRendererRate renderRate);
    AudioRendererRate GetRenderRate();
    int32_t SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback);
    int32_t SetRenderMode(AudioRenderMode renderMode);
    AudioRenderMode GetRenderMode();
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback);
    int32_t GetBufferDesc(BufferDesc &bufDesc);
    int32_t Enqueue(const BufferDesc &bufDesc);
    int32_t Clear();

    std::vector<AudioSampleFormat> GetSupportedFormats();
    std::vector<AudioChannel> GetSupportedChannels();
    std::vector<AudioEncodingType> GetSupportedEncodingTypes();
    std::vector<AudioSamplingRate> GetSupportedSamplingRates();

    // Common APIs
    bool StartAudioStream();
    bool PauseAudioStream();
    bool StopAudioStream();
    bool ReleaseAudioStream();
    bool FlushAudioStream();

    // Playback related APIs
    bool DrainAudioStream();
    size_t Write(uint8_t *buffer, size_t buffer_size);

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead);
private:
    AudioStreamType eStreamType_;
    AudioMode eMode_;
    State state_;
    std::atomic<bool> isReadInProgress_;
    std::atomic<bool> isWriteInProgress_;
    uint64_t resetTimestamp_;
    struct timespec baseTimestamp_;
    AudioRenderMode renderMode_;
    std::queue<BufferDesc> freeBufferQ_;
    std::queue<BufferDesc> filledBufferQ_;
    std::array<std::unique_ptr<uint8_t[]>, MAX_NUM_BUFFERS> bufferPool_ = {};
    std::unique_ptr<std::thread> writeThread_ = nullptr;
    bool isReadyToWrite_;
    void WriteBuffers();

    static constexpr AudioStreamType streamTypeMap_[CONTENT_TYPE_RINGTONE + 1][STREAM_USAGE_VOICE_ASSISTANT + 1] = {
        {STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC},
        {STREAM_MUSIC, STREAM_VOICE_ASSISTANT, STREAM_VOICE_CALL, STREAM_MUSIC, STREAM_VOICE_ASSISTANT},
        {STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC, STREAM_RING, STREAM_VOICE_ASSISTANT},
        {STREAM_MEDIA, STREAM_MEDIA, STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC},
        {STREAM_NOTIFICATION, STREAM_NOTIFICATION, STREAM_MUSIC, STREAM_MUSIC, STREAM_MUSIC},
        {STREAM_RING, STREAM_RING, STREAM_MUSIC, STREAM_RING, STREAM_MUSIC}
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_H
