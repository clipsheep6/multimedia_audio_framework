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

#ifndef ST_DEVICE_INIT_CALLBACK_H
#define ST_DEVICE_INIT_CALLBACK_H

#include "device_manager_callback.h"
#include "device_manager.h"

namespace OHOS {
namespace AudioStandard {
class DeviceInitCallBack : public DistributedHardware::DmInitCallback {
public:
    explicit DeviceInitCallBack() = default;
    ~DeviceInitCallBack() override {};
    void OnRemoteDied() override {};
};

class DeviceStatusCallbackImpl : public DistributedHardware::DeviceStatusCallback {
public:
    explicit DeviceStatusCallbackImpl();
    ~DeviceStatusCallbackImpl() {};
    void OnDeviceChanged(DistributedHardware::DmDeviceBasicInfo &dmDeviceBasicInfo) {};

private:
    AudioPolicyService& audioPolicyService_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DEVICE_INIT_CALLBACK_H
