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

#ifndef AUDIO_SAFE_VOLUME_NOTIFICATION_IMPL_H
#define AUDIO_SAFE_VOLUME_NOTIFICATION_IMPL_H

#include "audio_safe_volume_notification.h"
#include <mutex>
#include <string>
#include "image_source.h"
#include "pixel_map.h"
#include "resource_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioSafeVolumeNotificationImpl : public AudioSafeVolumeNotification {
public:
    AudioSafeVolumeNotificationImpl() = default;
    virtual ~AudioSafeVolumeNotificationImpl() = default;

    void PublishSafeVolumeNotification(int32_t notificationId) override;
    void CancelSafeVolumeNotification(int32_t notificationId) override;
    bool GetPixelMap(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);
private:
    std::mutex mutex_ {};
    std::shared_ptr<Media::PixelMap> iconPixelMap_ {};
};

extern "C" AudioSafeVolumeNotification* CreateSafeVolumeNotificationImpl() {
    return new AudioSafeVolumeNotificationImpl;
}
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_SAFE_VOLUME_NOTIFICATION_IMPL_H
