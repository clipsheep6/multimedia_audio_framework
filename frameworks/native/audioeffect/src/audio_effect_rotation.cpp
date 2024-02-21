/*
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

#include "audio_effect_chain_adapter.h"
#include "audio_effect_rotation.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
#ifdef WINDOW_MANAGER_ENABLE
AudioEffectRotation::AudioEffectRotation()
{
    AUDIO_INFO_LOG("AudioEffectRotation created!");
    rotationState_ = 0;
}

AudioEffectRotation::~AudioEffectRotation()
{
    AUDIO_INFO_LOG("AudioEffectRotation destroyed!");
}

AudioEffectRotation *AudioEffectRotation::GetInstance()
{
    static AudioEffectRotation audioEffectRotation;
    return &audioEffectRotation;
}

void AudioEffectRotation::Init()
{
    AUDIO_DEBUG_LOG("Call RegisterDisplayListener.");
    audioRotationListener_ = new AudioRotationListener();
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(audioRotationListener_);
}

void AudioEffectRotation::SetRotation(uint32_t rotationState)
{
    rotationState_ = rotationState;
}

uint32_t AudioEffectRotation::GetRotation()
{
    return rotationState_;
}

void AudioEffectRotation::OnCreate(Rosen::DisplayId displayId)
{
    AUDIO_DEBUG_LOG("Onchange displayId: %{public}llu.", displayId);
}

void AudioEffectRotation::OnDestroy(Rosen::DisplayId displayId)
{
    AUDIO_DEBUG_LOG("OnDestroy displayId: %{public}llu.", displayId);
}

void AudioEffectRotation::OnChange(Rosen::DisplayId displayId)
{
    AUDIO_DEBUG_LOG("Onchange displayId: %{public}llu.", displayId);
    // get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        AUDIO_WARNING_LOG("Get display by displayId: %{public}llu failed.", displayId);
        return;
    }
    // get rotation
    Rosen::Rotation newRotationState = display->GetRotation();
    AUDIO_DEBUG_LOG("Onchange rotationState: %{public}u.", static_cast<uint32_t>(newRotationState));
    EffectChainManagerRotationUpdate(static_cast<uint32_t>(newRotationState));
}
#endif
}  // namespace AudioStandard
}  // namespace OHOS