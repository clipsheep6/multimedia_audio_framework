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


#include "device_init_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

DeviceStatusCallbackImpl::DeviceStatusCallback()
    : audioPolicyService_(AudioPolicyService::GetAudioPolicyService())
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
}

void DeviceStatusCallbackImpl::OnDeviceChanged(DmDeviceBasicInfo &dmDeviceBasicInfo)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
	std::string strDeviceName(dmDeviceBasicInfo.deviceName);
    AUDIO_INFO_LOG("UpdateDisplayName local name [%{public}s]", strDeviceName);
	std::string strNetworkId(dmDeviceBasicInfo.networkId);
     AUDIO_INFO_LOG("UpdateStrNetworkId local name [%{public}s]", strNetworkId);

    audioPolicyService_.SetDisplayName(strDeviceName, strNetworkId);
}
} // namespace AudioStandard
} // namespace OHOS