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

#ifndef ST_AUDIO_SPATIALIZATION_MANAGER_H
#define ST_AUDIO_SPATIALIZATION_MANAGER_H

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"
// #include "audio_interrupt_callback.h"
// #include "audio_group_manager.h"
// #include "audio_routing_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationEnabledChangeCallback {
public:
    virtual ~AudioSpatializationEnabledChangeCallback() = default;
    /**
     * @brief AudioSpatializationEnabledChangeCallback will be executed when spatialization enabled state changes
     *
     * @param enabled the spatialization enabled state.
     * @since 11
     */
    virtual void OnSpatializationEnabledChange(const bool &enabled) = 0;
};

class AudioHeadTrackingEnabledChangeCallback {
public:
    virtual ~AudioHeadTrackingEnabledChangeCallback() = default;
    /**
     * @brief AudioHeadTrackingEnabledChangeCallback will be executed when head tracking enabled state changes
     *
     * @param enabled the head tracking enabled state.
     * @since 11
     */
    virtual void OnHeadTrackingEnabledChange(const bool &enabled) = 0;
};

/**
 * @brief The AudioSpatializationManager class is an abstract definition of audio spatialization manager.
 *        Provides a series of client/interfaces for audio spatialization management
 */

class AudioSpatializationManager {
public:
    // AudioSpatializationManager() = default;
    // virtual ~AudioSpatializationManager() = default;

    static AudioSpatializationManager *GetInstance();

    /**
     * @brief Check whether the spatialization is enabled
     *
     * @return Returns <b>true</b> if the spatialization is successfully enabled; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsSpatializationEnabled();

    /**
     * @brief Set the spatialization enabled or disabled
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetSpatializationEnabled(const bool enable);

    /**
     * @brief Check whether the head tracking is enabled
     *
     * @return Returns <b>true</b> if the head tracking is successfully enabled; returns <b>false</b> otherwise.
     * @since 11
     */
    bool IsHeadTrackingEnabled();

    /**
     * @brief Set the head tracking enabled or disabled
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetHeadTrackingEnabled(const bool enable);
private:
    AudioSpatializationManager();
    virtual ~AudioSpatializationManager();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SPATIALIZATION_MANAGER_H
