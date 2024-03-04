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

#include "audio_spatialization_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_manager_proxy.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_manager.h"
#include "audio_utils.h"
#undef LOG_TAG
#define LOG_TAG "AudioSpatializationManager"
namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioSpatializationManager::AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager start");
}

AudioSpatializationManager::~AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager::~AudioSpatializationManager");
}

AudioSpatializationManager *AudioSpatializationManager::GetInstance()
{
    static AudioSpatializationManager audioSpatializationManager;
    return &audioSpatializationManager;
}

bool AudioSpatializationManager::IsSpatializationEnabled()
{
    return AudioPolicyManager::GetInstance().IsSpatializationEnabled();
}

int32_t AudioSpatializationManager::SetSpatializationEnabled(const bool enable)
{
    return AudioPolicyManager::GetInstance().SetSpatializationEnabled(enable);
}

bool AudioSpatializationManager::IsHeadTrackingEnabled()
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
}

int32_t AudioSpatializationManager::SetHeadTrackingEnabled(const bool enable)
{
    return AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(enable);
}

int32_t AudioSpatializationManager::RegisterSpatializationEnabledEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
}

int32_t AudioSpatializationManager::RegisterHeadTrackingEnabledEventListener(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    return AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
}

int32_t AudioSpatializationManager::UnregisterSpatializationEnabledEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
}

int32_t AudioSpatializationManager::UnregisterHeadTrackingEnabledEventListener()
{
    return AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
}

bool AudioSpatializationManager::IsSpatializationSupported()
{
    return AudioPolicyManager::GetInstance().IsSpatializationSupported();
}

bool AudioSpatializationManager::IsSpatializationSupportedForDevice(const sptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    return AudioPolicyManager::GetInstance().IsSpatializationSupportedForDevice(selectedAudioDevice->macAddress_);
}

bool AudioSpatializationManager::IsHeadTrackingSupported()
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingSupported();
}

bool AudioSpatializationManager::IsHeadTrackingSupportedForDevice(const sptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    return AudioPolicyManager::GetInstance().IsHeadTrackingSupportedForDevice(selectedAudioDevice->macAddress_);
}

int32_t AudioSpatializationManager::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    return AudioPolicyManager::GetInstance().UpdateSpatialDeviceState(audioSpatialDeviceState);
}
} // namespace AudioStandard
} // namespace OHOS
