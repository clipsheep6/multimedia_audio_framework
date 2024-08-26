﻿/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SAFE_VOLUME_NOTIFICATION_H
#define AUDIO_SAFE_VOLUME_NOTIFICATION_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AudioStandard {
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

const int32_t NOTIFICATION_BANNER_FLAG = 1 << 9;

constexpr const char* SAFE_VOLUME_ICON_ID = "ohos_ic_public_device_speaker_filled";
constexpr const char* SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID = "ohos_id_notification_restore_volume_tittle";
constexpr const char* SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID = "ohos_id_notification_increase_volume_tittle";
constexpr const char* SAFE_VOLUME_MUSIC_TIMER_TEXT_ID = "ohos_id_notification_restore_volume_context";
constexpr const char* SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID = "ohos_id_notification_increase_volume_context";
constexpr const char* SAFE_VOLUME_RESTORE_VOL_BUTTON_ID = "ohos_id_notification_restore_volume_button";
constexpr const char* SAFE_VOLUME_INCREASE_VOL_BUTTON_ID = "ohos_id_notification_increase_volume_button";

class AudioSafeVolumeNotification {
public:
    AudioSafeVolumeNotification() = default;
    virtual ~AudioSafeVolumeNotification() = default;

    virtual void PublishSafeVolumeNotification(int32_t notificationId) = 0;
    virtual void CancelSafeVolumeNotification(int32_t notificationId) = 0;
};

typedef AudioSafeVolumeNotification* CreateSafeVolumeNotification();
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SAFE_VOLUME_NOTIFICATION_H
