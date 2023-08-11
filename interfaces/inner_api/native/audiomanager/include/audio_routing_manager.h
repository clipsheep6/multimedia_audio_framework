/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ROUTING_MANAGER_H
#define ST_AUDIO_ROUTING_MANAGER_H

#include <iostream>

#include "audio_system_manager.h"
#include "audio_info.h"
#include "audio_group_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceDescriptor;
class AudioRendererFilter;
class AudioPreferOutputDeviceChangeCallback {
public:
    virtual ~AudioPreferOutputDeviceChangeCallback() = default;
    /**
     * Called when the prefer output device changes
     *
     * @param vector<sptr<AudioDeviceDescriptor>> deviceDescriptor.
     */
    virtual void OnPreferOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
};

class AudioRoutingManager {
public:
    AudioRoutingManager() = default;
    virtual ~AudioRoutingManager() = default;

    static AudioRoutingManager *GetInstance();
    int32_t SetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);
    int32_t GetPreferOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &desc);
    int32_t GetPreferInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &desc);
    int32_t GetPreferOutputDeviceByFilter(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<DeviceType> &deviceTypes);
    int32_t SetPreferOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
        const std::shared_ptr<AudioPreferOutputDeviceChangeCallback>& callback);
    int32_t UnsetPreferOutputDeviceChangeCallback();
private:
    uint32_t GetCallingPid();
};

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ROUTING_MANAGER_H

