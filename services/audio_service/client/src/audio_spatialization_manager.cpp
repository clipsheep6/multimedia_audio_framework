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
// #include "audio_volume_key_event_callback_stub.h"
#include "audio_utils.h"
// #include "audio_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

// mutex g_asProxyMutex;
// sptr<IStandardAudioService> g_asProxy = nullptr;

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
} // namespace AudioStandard
} // namespace OHOS
