/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef BLUETOOTH_DEVICE_UTILS_H
#define BLUETOOTH_DEVICE_UTILS_H

#include "bluetooth_device.h"

namespace OHOS {
namespace Bluetooth {
enum BluetoothDeviceAction {
    WEAR = 0,
    UNWEAR = 1,
    ENABLEFROMREMOTE = 2,
    DISABLEFROMREMOTE = 3,
    CONNECT = 4,
    DISCONNECT = 5,
};

enum DeviceStatus {
    ADD = 0,
    REMOVE = 1,
};
} // namespace Bluetooth
} // namespace OHOS

#endif // BLUETOOTH_DEVICE_UTILS_H