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

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_policy_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyManagerListenerStub::AudioPolicyManagerListenerStub()
{
    AUDIO_DEBUG_LOG("AudioPolicyManagerLiternerStub Instance create");
}

AudioPolicyManagerListenerStub::~AudioPolicyManagerListenerStub()
{
    AUDIO_DEBUG_LOG("AudioPolicyManagerListenerStub Instance complete");
}

void AudioPolicyManagerListenerStub::ReadAudioDeviceChangeData(MessageParcel &data, DeviceChangeAction &devChange)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceChangeDesc = {};

    int32_t type = data.ReadInt32();
    int32_t flag = data.ReadInt32();
    int32_t size = data.ReadInt32();

    for (int32_t i = 0; i < size; i++) {
        deviceChangeDesc.push_back(AudioDeviceDescriptor::Unmarshalling(data));
    }

    devChange.type = static_cast<DeviceChangeType>(type);
    devChange.flag = static_cast<DeviceFlag>(flag);
    devChange.deviceDescriptors = deviceChangeDesc;
}

int AudioPolicyManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioPolicyManagerListenerStub: ReadInterfaceToken failed");
        return -1;
    }
    switch (code) {
        case ON_AVAILABLE_DEVICE_CAHNGE: {
            AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
            DeviceChangeAction deviceChangeAction = {};
            ReadAudioDeviceChangeData(data, deviceChangeAction);
            OnAvailableDeviceChange(usage, deviceChangeAction);
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioPolicyManagerListenerStub::OnAvailableDeviceChange(const AudioDeviceUsage usage,
    const DeviceChangeAction &deviceChangeAction)
{
    AUDIO_DEBUG_LOG("AudioPolicyManagerLiternerStub OnAvailableDeviceChange start");
    std::shared_ptr<AudioManagerAvailableDeviceChangeCallback> availabledeviceChangedCallback =
        audioAvailableDeviceChangeCallback_.lock();

    CHECK_AND_RETURN_LOG(availabledeviceChangedCallback != nullptr,
        "OnAvailableDeviceChange: deviceChangeCallback_ or deviceChangeAction is nullptr");

    availabledeviceChangedCallback->OnAvailableDeviceChange(usage, deviceChangeAction);
}

void AudioPolicyManagerListenerStub::SetAvailableDeviceChangeCallback(
    const std::weak_ptr<AudioManagerAvailableDeviceChangeCallback> &cb)
{
    audioAvailableDeviceChangeCallback_ = cb;
}
} // namespace AudioStandard
} // namespace OHOS
