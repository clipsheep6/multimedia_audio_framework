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
#undef private
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_interrupt_service.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";

void InitFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioPolicyServer> server = nullptr;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->Init(server);
}

void AddDumpInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    PolicyData policyData;
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->AddDumpInfo(policyData);
}

void SetCallbackHandlerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    std::shared_ptr<AudioPolicyServerHandler> handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->SetCallbackHandler(handler);
}

void SetAudioManagerInterruptCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();

    interruptService->SetAudioManagerInterruptCallback(object);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::InitFuzzTest(data, size);
    OHOS::AudioStandard::AddDumpInfoFuzzTest(data, size);
    OHOS::AudioStandard::SetCallbackHandlerFuzzTest(data, size);
    OHOS::AudioStandard::SetAudioManagerInterruptCallbackFuzzTest(data, size);
    return 0;
}