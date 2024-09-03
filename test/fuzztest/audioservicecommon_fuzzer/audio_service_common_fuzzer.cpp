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

#include "audio_common_converter.h"
#include "message_parcel.h"
#include "audio_info.h"
#include "audio_source_type.h"
#include "audio_ring_cache.h"
#include "audio_thread_task.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const size_t COMMONCOUNT = 1;
const float COMMONVOLUME = 2.0f;
const int32_t ENUMSIZE = 5;
static const size_t MAX_MAX_CACHE_SIZE = 17 * 1024 * 1024; // 16M
static const size_t COMMON_CACHE_SIZE = 1024;
static const std::string THREAD_NAME = "FuzzTestThreadName";
void AudioCommonConvertFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint8_t state_buffer[] = {1, 2, 3, 4, 5};
    uint8_t* buffer = state_buffer;
    int32_t format = *reinterpret_cast<const int32_t*>(rawData);
    format = format%ENUMSIZE;
    int32_t state_dst[] = {1, 2, 3, 4, 5};
    int32_t* dst = state_dst;
    AudioCommonConverter::ConvertBufferTo32Bit(buffer, format, dst, COMMONCOUNT, COMMONVOLUME);
    int16_t state_16dst[] = {1, 2, 3, 4, 5};
    int16_t* dst_16 = state_16dst;
    AudioCommonConverter::ConvertBufferTo16Bit(buffer, format, dst_16, COMMONCOUNT, COMMONVOLUME);
    uint32_t samplePerFrame = *reinterpret_cast<const uint32_t*>(rawData);
    samplePerFrame = samplePerFrame%ENUMSIZE;
    std::vector<float> floatBuffer = {0.0f, 0.0f, 0.0f};
    AudioCommonConverter::ConvertBufferToFloat(buffer, samplePerFrame, floatBuffer, COMMONVOLUME);
    AudioCommonConverter::ConvertFloatToAudioBuffer(floatBuffer, buffer, samplePerFrame);
}

void AudioRingCacheFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unique_ptr<AudioRingCache> audioRingCache = AudioRingCache::Create(MAX_MAX_CACHE_SIZE);
    audioRingCache = AudioRingCache::Create(COMMON_CACHE_SIZE);
    audioRingCache->ReConfig(MAX_MAX_CACHE_SIZE);
    bool copyRemained = *reinterpret_cast<const bool*>(rawData);
    audioRingCache->ReConfig(COMMON_CACHE_SIZE, copyRemained);
    audioRingCache->GetWritableSize();
    audioRingCache->GetReadableSize();
    audioRingCache->GetCahceSize();

    BufferWrap writeBuffer;
    writeBuffer.dataPtr = nullptr;
    writeBuffer.dataSize = 0;
    audioRingCache->Enqueue(writeBuffer);
    audioRingCache->Dequeue(writeBuffer);
    writeBuffer.dataPtr = const_cast<uint8_t*>(rawData);
    writeBuffer.dataSize = size;
    audioRingCache->Enqueue(writeBuffer);
    audioRingCache->Dequeue(writeBuffer);
    audioRingCache->ResetBuffer();
}

void AudioThreadTaskFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unique_ptr<AudioThreadTask> audioThreadTask;
    audioThreadTask = std::make_unique<AudioThreadTask>(THREAD_NAME);
    auto myJob = []() {
        cout<<"Hello Fuzz Test!"<<endl;
    };
    audioThreadTask->RegisterJob(std::move(myJob));
    audioThreadTask->Start();
    audioThreadTask->CheckThreadIsRunning();
    audioThreadTask->Pause();
    audioThreadTask->Start();
    audioThreadTask->PauseAsync();
    audioThreadTask->Start();
    audioThreadTask->StopAsync();
    audioThreadTask->Start();
    audioThreadTask->Stop();
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioCommonConvertFuzzTest(data, size);
    OHOS::AudioStandard::AudioRingCacheFuzzTest(data, size);
    OHOS::AudioStandard::AudioThreadTaskFuzzTest(data, size);
    return 0;
}
