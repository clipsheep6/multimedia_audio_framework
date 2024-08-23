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

#include <iostream>
#include <cstddef>
#include <cstdint>

#include "i_audio_renderer_sink.h"
#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "message_parcel.h"
#include "audio_process_in_client.h"
#include "audio_param_parser.h"
#include "audio_info.h"
#include "audio_source_type.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
bool g_hasClientInit = false;
const size_t AudioClientGetAudioInt64Count = 2; //函数AudioClientGetAudioTimeTest从rawData中转换几个int64_t数据
shared_ptr<AudioProcessInClient> g_AudioProcessInClient = nullptr;

AudioProcessConfig GetAudioProcessConfig(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return {};
    }
    int32_t callerUid = INTELL_VOICE_SERVICR_UID;
    AppInfo appInfo = {};
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    audioStreamInfo.channels = MONO;
    audioStreamInfo.format = SAMPLE_S16LE;
    audioStreamInfo.encoding = ENCODING_PCM;
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    ContentType contentType = CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    int32_t rendererFlags = *reinterpret_cast<const int32_t *>(rawData);
    std::string sceneType(reinterpret_cast<const char*>(rawData), size - 1);
    bool spatializationEnabled = *reinterpret_cast<const bool *>(rawData);
    bool headTrackingEnabled = *reinterpret_cast<const bool *>(rawData);
    int32_t originalFlag = *reinterpret_cast<const int32_t *>(rawData);
    AudioRendererInfo rendererInfo = {contentType, streamUsage, rendererFlags, sceneType, spatializationEnabled,
        headTrackingEnabled, originalFlag
    };
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    AudioCapturerInfo capturerInfo = {sourceType, capturerFlags};
    AudioStreamType streamType = STREAM_MUSIC;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    bool isInnerCapturer = true;
    bool isWakeupCapturer = true;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    InnerCapMode innerCapMode = LEGACY_INNER_CAP;
    AudioProcessConfig audioProcessConfig = {};
    audioProcessConfig.appInfo = appInfo;
    audioProcessConfig.callerUid = callerUid;
    audioProcessConfig.streamInfo = audioStreamInfo;
    audioProcessConfig.audioMode = audioMode;
    audioProcessConfig.rendererInfo = rendererInfo;
    audioProcessConfig.capturerInfo = capturerInfo;
    audioProcessConfig.streamType = streamType;
    audioProcessConfig.deviceType = deviceType;
    audioProcessConfig.isInnerCapturer = isInnerCapturer;
    audioProcessConfig.isWakeupCapturer = isWakeupCapturer;
    audioProcessConfig.privacyType = privacyType;
    audioProcessConfig.innerCapMode = innerCapMode;
    return audioProcessConfig;
}

void GetAudioProcessInClient(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<AudioProcessInClient> audioProcessInClient = nullptr;
    if (!g_hasClientInit) {
        AudioProcessConfig audioProcessConfig = GetAudioProcessConfig(rawData, size);
        bool ret = AudioProcessInClient::CheckIfSupport(audioProcessConfig);
        if (!ret) {
            return;
        }
        g_AudioProcessInClient = AudioProcessInClient::Create(audioProcessConfig);
        if (g_AudioProcessInClient) {
            g_AudioProcessInClient->Start();
            g_hasClientInit = true;
        } else {
            return;
        }
    }
}

void AudioClientSetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t vol = *reinterpret_cast<const int32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetVolume(vol);
    }
}

void AudioClientGetSessionIDTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetSessionID(sessionID);
    }
}

void AudioClientGetAudioTimeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE ||
        size < (sizeof(uint32_t) + sizeof(int64_t)*AudioClientGetAudioInt64Count)) {
        return;
    }
    uint32_t framePos = *reinterpret_cast<const uint32_t*>(rawData);
    rawData += sizeof(uint32_t);
    int64_t sec = *reinterpret_cast<const int64_t*>(rawData);
    rawData += sizeof(int64_t);
    int64_t nanoSec = *reinterpret_cast<const int64_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetAudioTime(framePos, sec, nanoSec);
    }
}

void AudioClientGetBufferSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    size_t bufferSize = *reinterpret_cast<const size_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetBufferSize(bufferSize);
    }
}

void AudioClientGetFrameCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t frameCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFrameCount(frameCount);
    }
}

void AudioClientGetLatencyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    uint64_t latency = *reinterpret_cast<const uint64_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetLatency(latency);
    }
}

void AudioClientSetVolumeFloatTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetVolume(volume);
    }
}

void AudioClientGetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetVolume();
    }
}

void AudioClientSetDuckVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetDuckVolume(volume);
    }
}

void AudioClientGetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetUnderflowCount();
    }
}

void AudioClientGetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetOverflowCount();
    }
}

void AudioClientSetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t underflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetUnderflowCount(underflowCount);
    }
}

void AudioClientSetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t overflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetOverflowCount(overflowCount);
    }
}

void AudioClientGetFramesWrittenTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFramesWritten();
    }
}

void AudioClientGetFramesReadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFramesRead();
    }
}

void AudioClientSetApplicationCachePathTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string cachePath(reinterpret_cast<const char*>(rawData), size - 1);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetApplicationCachePath(cachePath);
    }
}

void AudioClientSetPreferredFrameSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int32_t)) {
        return;
    }
    int32_t frameSize = *(reinterpret_cast<const int32_t*>(rawData));
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetPreferredFrameSize(frameSize);
    }
}

void AudioClientUpdateLatencyTimestampTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE ||
        size < (sizeof(bool))) {
        return;
    }
    bool isRenderer = *(reinterpret_cast<const bool*>(rawData));
    rawData += sizeof(bool);
    size -= sizeof(bool);
    std::string timestamp(reinterpret_cast<const char*>(rawData), size - 1);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->UpdateLatencyTimestamp(timestamp, isRenderer);
    }
}

void AudioClientPauseTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Pause();
    }
}

void AudioClientResumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Resume();
    }
}

void AudioClientStopTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Stop();
    }
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::GetAudioProcessInClient(data, size);
    OHOS::AudioStandard::AudioClientSetVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientGetSessionIDTest(data, size);
    OHOS::AudioStandard::AudioClientGetAudioTimeTest(data, size);
    OHOS::AudioStandard::AudioClientGetBufferSizeTest(data, size);
    OHOS::AudioStandard::AudioClientGetFrameCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetLatencyTest(data, size);
    OHOS::AudioStandard::AudioClientSetVolumeFloatTest(data, size);
    OHOS::AudioStandard::AudioClientGetVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientSetDuckVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientGetUnderflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetOverflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientSetUnderflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientSetOverflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetFramesWrittenTest(data, size);
    OHOS::AudioStandard::AudioClientGetFramesReadTest(data, size);
    OHOS::AudioStandard::AudioClientSetApplicationCachePathTest(data, size);
    OHOS::AudioStandard::AudioClientSetPreferredFrameSizeTest(data, size);
    OHOS::AudioStandard::AudioClientUpdateLatencyTimestampTest(data, size);

    OHOS::AudioStandard::AudioClientPauseTest(data, size);
    OHOS::AudioStandard::AudioClientResumeTest(data, size);
    OHOS::AudioStandard::AudioClientStopTest(data, size);
    return 0;
}
