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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_bluetooth_manager.h"
#include "audio_hdiadapter_info.h"
#include "bluetooth_renderer_sink.h"
#include "audio_device_info.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;

void GetCurNanoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    ClockTime::GetCurNano()
}
void AbsoluteSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int64_t)) {
        return;
    }

    int64_t nanoTime = *(reinterpret_cast<const int64_t*>(rawData));
    ClockTime::AbsoluteSleep(nanoTime)
}

void RelativeSleepFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(int64_t)) {
        return;
    }

    int64_t nanoTime = *(reinterpret_cast<const int64_t*>(rawData));
    ClockTime::RelativeSleep(nanoTime)
}

void CountFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <= sizeof(int64_t)) {
        return;
    }

    int64_t count = *(reinterpret_cast<const int64_t*>(rawData));
    rawData += sizeof(count);
    size -= sizeof(count);
    const std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    Trace::Count(value, count);
}

void CountVolumeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size <= sizeof(uint8_t)) {
        return;
    }

    uint8_t data = *(reinterpret_cast<const uint8_t*>(rawData));
    rawData += sizeof(data);
    size -= sizeof(count);
    const std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    Trace::CountVolume(value, data);
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::GetCurNanoFuzzTest(data, size);
    OHOS::AudioStandard::AbsoluteSleepFuzzTest(data, size);
    OHOS::AudioStandard::RelativeSleepFuzzTest(data, size);

    OHOS::AudioStandard::CountFuzzTest(data, size);
    OHOS::AudioStandard::CountVolumeFuzzTest(data, size);
    return 0;
}
