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
#include "audio_hdiadapter_info.h"
#include "audio_device_info.h"
#include "i_audio_device_manager.h"
#include "audio_device_manager_impl.h"
#include "audio_device_adapter_impl.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const uint32_t ENUMSIZE = 4;
const char *SINK_ADAPTER_NAME = "primary";
const string DEFAULTNAME = "DEFAULT";

void AudioDeviceManagerFactoryFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioDeviceManagerFactory &audioDeviceManagerFactory = AudioDeviceManagerFactory::GetInstance();
    uint32_t audioDeviceManagerType_int = *reinterpret_cast<const uint32_t*>(rawData);
    audioDeviceManagerType_int = audioDeviceManagerType_int%ENUMSIZE;
    AudioDeviceManagerType audioDeviceManagerType = static_cast<AudioDeviceManagerType>(audioDeviceManagerType_int);
    audioDeviceManagerFactory.CreatDeviceManager(audioDeviceManagerType);
    audioDeviceManagerFactory.DestoryDeviceManager(audioDeviceManagerType);
}

void AudioDeviceManagerImlFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    shared_ptr<AudioDeviceManagerImpl> audioDeviceManagerIml = nullptr;
    uint32_t audioDeviceManagerType_int = *reinterpret_cast<const uint32_t*>(rawData);
    audioDeviceManagerType_int = audioDeviceManagerType_int%ENUMSIZE;
    AudioDeviceManagerType audioDeviceManagerType = static_cast<AudioDeviceManagerType>(audioDeviceManagerType_int);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    sptr<IAudioManager> audioMgr = IAudioManager::Get("daudio_primary_service", false);
#else
    sptr<IAudioManager> audioMgr = nullptr;
#endif // FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml = make_shared<AudioDeviceManagerImpl>(audioDeviceManagerType, audioMgr);
    if (audioDeviceManagerIml == nullptr) {
        return;
    }
    audioDeviceManagerIml->GetAllAdapters();
    bool isMmap = *reinterpret_cast<const bool*>(rawData);
    const string adapterName = "";
    audioDeviceManagerIml->LoadAdapters(adapterName, isMmap);
    audioDeviceManagerIml->UnloadAdapter(adapterName);
    audioDeviceManagerIml->LoadAdapters(DEFAULTNAME, isMmap);
    audioDeviceManagerIml->UnloadAdapter(DEFAULTNAME);
    audioDeviceManagerIml->Release();
    struct AudioAdapterDescriptor *desc = audioDeviceManagerIml->GetTargetAdapterDesc(DEFAULTNAME, isMmap);
    if (desc == nullptr || audioMgr == nullptr) {
        return;
    }
    AudioAdapterDescriptor descriptor = {
        .adapterName = desc->adapterName,
    };
    sptr<IAudioAdapter> audioAdapter = nullptr;
    audioMgr->LoadAdapter(descriptor, audioAdapter);

    auto audioDevAdp = std::make_shared<AudioDeviceAdapterImpl>(std::string(desc->adapterName), audioAdapter);
    audioDevAdp->SetParamCallback(audioDevAdp);
    audioDevAdp = AudioDeviceAdapterImpl::GetParamCallback();
    audioDevAdp->Init();
    audioDevAdp->RegExtraParamObserver();
    AudioParamKey audioParamKey = *reinterpret_cast<const AudioParamKey*>(rawData);
    const string condition = "";
    const string value = "";
    audioDevAdp->SetAudioParameter(audioParamKey, condition, value);
    audioDevAdp->GetAudioParameter(audioParamKey, condition);
    audioDevAdp->Release();
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioDeviceManagerFactoryFuzzTest(data, size);
    OHOS::AudioStandard::AudioDeviceManagerImlFuzzTest(data, size);
    return 0;
}
