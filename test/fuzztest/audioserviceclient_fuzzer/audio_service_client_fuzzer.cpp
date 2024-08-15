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
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
bool g_hasClientInit = false;

static AudioProcessConfig getAudioProcessConfig(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE
        || size <= (sizeof(AudioSamplingRate) + sizeof(sizeof(AudioChannel)+ sizeof(AudioSampleFormat) +
            sizeof(AudioEncodingType) + sizeof(ContentType) + sizeof(int32_t)*3 + sizeof(bool)*2))) { 
        return {};
    }
    int32_t callerUid = INVALID_UID;
    AppInfo appInfo = {};
    AudioStreamInfo streamInfo = {};
    streamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    rawData += sizeof(AudioSamplingRate);
    streamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
    rawData += sizeof(AudioChannel);
    streamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    rawData += sizeof(AudioSampleFormat);
    streamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
    rawData += sizeof(AudioEncodingType);
    AudioMode audioMode = AUDIO_MODE_PLAYBACK;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    rawData += sizeof(int32_t);
    AudioCapturerInfo capturerInfo = {sourceType, capturerFlags};
    AudioStreamType streamType = STREAM_DEFAULT;
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    bool isInnerCapturer = false;
    bool isWakeupCapturer = false;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    InnerCapMode innerCapMode {InnerCapMode::INVALID_CAP_MODE};

    ContentType contentType = *reinterpret_cast<const ContentType *>(rawData);
    StreamUsage streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    int32_t rendererFlags = *reinterpret_cast<const int32_t *>(rawData);
    rawData += sizeof(int32_t);
    bool spatializationEnabled = *reinterpret_cast<const bool *>(rawData);
    rawData += sizeof(bool);
    bool headTrackingEnabled = *reinterpret_cast<const bool *>(rawData);
    rawData += sizeof(bool);
    int32_t originalFlag = *reinterpret_cast<const int32_t *>(rawData);
    rawData += sizeof(int32_t);
    std::string sceneType(reinterpret_cast<const char*>(rawData), size - (sizeof(AudioSamplingRate) + sizeof(sizeof(AudioChannel)+ sizeof(AudioSampleFormat) +
            sizeof(AudioEncodingType) + sizeof(ContentType) + sizeof(int32_t)*3 + sizeof(bool)*2)));

    AudioRendererInfo rendererInfo = {contentType, streamUsage, rendererFlags, sceneType,
        spatializationEnabled, headTrackingEnabled,originalFlag};

    AudioProcessConfig audioProcessConfig = {callerUid, appInfo, streamInfo, audioMode, rendererInfo,
        capturerInfo, streamType, deviceType, isInnerCapturer, isWakeupCapturer, privacyType, innerCapMode};
    return audioProcessConfig;
}

std::shared_ptr<AudioProcessInClient> GetAudioProcessInClient(const uint8_t *rawData, size_t size)
{
    static std::shared_ptr<AudioProcessInClient> audioProcessInClient = nullptr;
    if (!g_hasClientInit) {
        AudioProcessConfig audioProcessConfig = getAudioProcessConfig(rawData, size);
        AudioProcessInClient::CheckIfSupport(audioProcessConfig);
        audioProcessInClient = AudioProcessInClient::Create(audioProcessConfig);
        audioProcessInClient->Start();
        g_hasClientInit = true;
    }
    return audioProcessInClient;
}

float Convert2Float(const uint8_t *ptr)
{
    float floatValue = static_cast<float>(*ptr);
    return floatValue / 128.0f - 1.0f;
}

void AudioServerLoadConfigurationTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys = {};
    std::unordered_map<std::string, std::set<std::string>> audioParameterKey = {};
    std::set<std::string> audioParameterValue = {};
    // 这里的string都使用一个即可
    std::string audioParameterKeyStr_1(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyStr_2(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyValueStr_1(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyValueStr_2(reinterpret_cast<const char*>(rawData), size - 1);
    audioParameterValue.insert(audioParameterKeyValueStr_1);
    audioParameterValue.insert(audioParameterKeyValueStr_2);
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr_1, audioParameterValue));
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr_2, audioParameterValue));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr_1, audioParameterKey));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr_2, audioParameterKey));

    std::shared_ptr<AudioParamParser> audioParamParser = std::make_shared<AudioParamParser>();
    audioParamParser->LoadConfiguration(audioParameterKeys);
}

void AudioClientSetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t vol = *reinterpret_cast<const int32_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->SetVolume(vol);
}

void AudioClientGetSessionIDTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->GetSessionID(sessionID);
}

void AudioClientGetAudioTimeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE ||
        size < (sizeof(uint32_t) + sizeof(int64_t)*2)) {
        return;
    }
    uint32_t framePos = *reinterpret_cast<const uint32_t*>(rawData);
    rawData += sizeof(uint32_t);
    int64_t sec = *reinterpret_cast<const int64_t*>(rawData);
    rawData += sizeof(int64_t);
    int64_t nanoSec = *reinterpret_cast<const int64_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->GetAudioTime(framePos, sec, nanoSec);
}

void AudioClientGetBufferSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    size_t bufferSize = *reinterpret_cast<const size_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->GetBufferSize(bufferSize);
}

void AudioClientGetFrameCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t frameCount = *reinterpret_cast<const uint32_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->GetFrameCount(frameCount);
}

void AudioClientGetLatencyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    uint64_t latency = *reinterpret_cast<const uint64_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->GetLatency(latency);
}

void AudioClientSetVolumeFloatTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    GetAudioProcessInClient(rawData, size)->SetVolume(volume);
}

void AudioClientGetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->GetVolume();
}

void AudioClientSetDuckVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    GetAudioProcessInClient(rawData, size)->SetDuckVolume(volume);
}

void AudioClientGetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->GetUnderflowCount();
}

void AudioClientGetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->GetOverflowCount();
}

void AudioClientSetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t underflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->SetUnderflowCount(underflowCount);
}

void AudioClientSetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t overflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    GetAudioProcessInClient(rawData, size)->SetOverflowCount(overflowCount);
}

void AudioClientGetFramesWrittenTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->GetFramesWritten();
}

void AudioClientGetFramesReadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->GetFramesRead();
}

void AudioClientSetApplicationCachePathTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string cachePath(reinterpret_cast<const char*>(rawData), size - 1);
    GetAudioProcessInClient(rawData, size)->SetApplicationCachePath(cachePath);
}

void AudioClientSetPreferredFrameSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int32_t)) {
        return;
    }
    int32_t frameSize = *(reinterpret_cast<const int32_t*>(rawData));
    GetAudioProcessInClient(rawData, size)->SetPreferredFrameSize(frameSize);
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
    GetAudioProcessInClient(rawData, size)->UpdateLatencyTimestamp(timestamp, isRenderer);
}

void AudioClientPauseTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->Pause();
}

void AudioClientResumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->Resume();
}

void AudioClientStopTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    GetAudioProcessInClient(rawData, size)->Stop();
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
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
