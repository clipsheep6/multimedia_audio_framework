/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef AUDIO_BLUETOOTH_MANAGERI_H
#define AUDIO_BLUETOOTH_MANAGERI_H

#include "bluetooth_a2dp_codec.h"
#include "bluetooth_a2dp_a2dpCodecInfo.h"
#include "bluetooth_a2dp_a2dpCodecStatus.h"
#include "idevice_status_observer.h"
#include "raw_address.h"

namespace OHOS {
namespace Bluetooth {
using namespace bluetooth;

typedef struct BtA2dpAudioCallback {
    void (*OnPlayingStatusChanged)(const RawAddress &device, int playingState, int error);
    void (*OnConfigurationChanged)(const RawAddress &device, const BluetoothA2dpCodecInfo &info, int error);
    void (*OnConnectionStateChanged)(const RawAddress &device, int state);
} BtA2dpAudioCallback;

int GetPlayingState();
RawAddress& GetDevice();
void GetProxy();
void RegisterObserver(AudioStandard::IDeviceStatusObserver &observer);
}
}
#endif  // AUDIO_BLUETOOTH_MANAGERI_H
