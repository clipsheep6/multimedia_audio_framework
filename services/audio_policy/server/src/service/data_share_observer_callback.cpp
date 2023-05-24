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


#include "data_share_observer_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const uint32_t DEVICE_NAME_LEN = 128;

DataShareObserverCallBack::DataShareObserverCallBack()
    :mPolicyService(AudioPolicyService::GetAudioPolicyService())
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
}

void DataShareObserverCallBack::OnChange()
{
    char devicesName[DEVICE_NAME_LEN] = {0};
    int32_t ret = mPolicyService.GetDeviceNameFromDataShareHelper(devicesName, DEVICE_NAME_LEN);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Local UpdateDisplayName init device failed");
        return;
    
    std::string strLocalDevicesName(devicesName);
    mPolicyService.SetDisplayName(strLocalDevicesName);
}
    
} // namespace AudioStandard
} // namespace OHOS
