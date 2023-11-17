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

#include "audio_policy_manager_listener_proxy.h"
#include "audio_system_manager.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyManagerListenerProxy::AudioPolicyManagerListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioPolicyManagerListener>(impl)
{
    AUDIO_DEBUG_LOG("Instances create");
}

AudioPolicyManagerListenerProxy::~AudioPolicyManagerListenerProxy()
{
    AUDIO_DEBUG_LOG("~AudioPolicyManagerListenerProxy: Instance destroy");
}

void AudioPolicyManagerListenerProxy::OnAvailableDeviceChange(const AudioDeviceUsage usage,
    const DeviceChangeAction &deviceChangeAction)
{
    AUDIO_DEBUG_LOG("AudioPolicyManagerListenerProxy: OnAvailableDeviceChange at listener proxy");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    CHECK_AND_RETURN_LOG(data.WriteInterfaceToken(GetDescriptor()),
        "AudioPolicyManagerListenerProxy: WriteInterfaceToken failed");

    auto devices = deviceChangeAction.deviceDescriptors;
    size_t size = deviceChangeAction.deviceDescriptors.size();

    bool token = data.WriteInt32(usage) && data.WriteInt32(deviceChangeAction.type) &&
        data.WriteInt32(deviceChangeAction.flag) && data.WriteInt32(static_cast<int32_t>(size));
    CHECK_AND_RETURN_LOG(token, "Write data failed");

    for (size_t i = 0; i < size; i++) {
        devices[i]->Marshalling(data);
    }

    int error = Remote()->SendRequest(ON_AVAILABLE_DEVICE_CAHNGE, data, reply, option);
    CHECK_AND_RETURN_LOG(error == ERR_NONE, "OnDeviceChange failed, error: %{public}d", error);
}
} // namespace AudioStandard
} // namespace OHOS
