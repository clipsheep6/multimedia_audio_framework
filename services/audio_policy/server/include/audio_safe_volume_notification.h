/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <mutex>
#include <string>

#include "image_source.h"
#include "pixel_map.h"
#include "resource_manager.h"

namespace OHOS {
namespace AudioStandard {
const int32_t SAFE_VOLUME_NOTIFICATION_ID_1 = 116000;
const int32_t SAFE_VOLUME_NOTIFICATION_ID_2 = 116001;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

const int SLOT_CONTROL_FLAG = 0b101111; // no vibration

const std::string SAFE_VOLUME_MUSIC_TIMER_TITTLE = "长时间保持高音量可能损伤听力";
const std::string SAFE_VOLUME_INCREASE_VOLUME_TITTLE = "提高音量到安全范围以上";
const std::string SAFE_VOLUME_MUSIC_TIMER_TEXT = "为保护听力，音量已降至安全范围。";
const std::string SAFE_VOLUME_INCREASE_VOLUME_TEXT = "长时间保持高音量有可能会损伤听力，确定提高音量？";
const std::string SAFE_VOLUME_RESTORE_VOL_BUTTON = "恢复音量";
const std::string SAFE_VOLUME_INCREASE_VOL_BUTTON = "提高音量";

constexpr const char* SAFE_VOLUME_ICON_ID = "ohos_ic_public_device_speaker_filled";
constexpr const char* SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID = "to do mutilanguage";
constexpr const char* SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID = "to do mutilanguage";
constexpr const char* SAFE_VOLUME_MUSIC_TIMER_TEXT_ID = "to do mutilanguage";
constexpr const char* SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID = "to do mutilanguage";
constexpr const char* SAFE_VOLUME_RESTORE_VOL_BUTTON_ID = "to do mutilanguage";
constexpr const char* SAFE_VOLUME_INCREASE_VOL_BUTTON_ID = "to do mutilanguage";

class AudioSafeVolumeNotification {
public:
    static AudioSafeVolumeNotification& GetInstance(void);

    void PublishSafeVolumeNotification(int32_t notificationId);

private:
    AudioSafeVolumeNotification();
    ~AudioSafeVolumeNotification();
    AudioSafeVolumeNotification(const AudioSafeVolumeNotification&) = delete;
    AudioSafeVolumeNotification& operator=(const AudioSafeVolumeNotification&) = delete;

    bool GetPixelMap(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    std::mutex mutex_ {};
    std::shared_ptr<Media::PixelMap> iconPixelMap_ {};
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SAFE_VOLUME_NOTIFICATION_H
