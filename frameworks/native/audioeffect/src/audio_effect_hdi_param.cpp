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

#include "audio_effect_hdi_param.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioRotation::AudioRotation() {
    AUDIO_INFO_LOG("AudioRotation created!");
    rotationState_ = 0;
}

~AudioRotation::AudioRotation() {
    AUDIO_INFO_LOG("AudioRotation destroyed!");
}

 void AudioRotation::Init() {
    AUDIO_DEBUG_LOG("Call RegisterDisplayListener.");
    audioDisplayListener_ = new AudioRotationListener(*this);
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(audioDisplayListener_);
}

void AudioRotation::OnChange(Rosen::DisplayId displayId) {
    AUDIO_DEBUG_LOG("Onchange displayId: %{public}lu .", displayId);
    // get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        AUDIO_WARNING_LOG("Get display by displayId: %{public}lu failed.", displayId);
        return;
    }
    // get rotation
    uint32_t newRotationState= display->GetRotation();
    if (newRotationState != rotationState_) {
        AUDIO_DEBUG_LOG("rotationState change, previous state: %{public}d, new state: %{public}d",
            rotationState_, newRotationState);
        rotationState_ = newRotationState;
        // set param to ap
        // set param to dsp
        effectHdiInput[0] = HDI_ROTATION_MODE;
        effectHdiInput[1] = rotationState_;
        AUDIO_INFO_LOG("set hdi rotation mode: %{public}d", effectHdiInput[1]);
        int32_t ret = audioEffectHdiParam_->UpdateHdiState(effectHdiInput);
        if (ret != 0) {
            AUDIO_WARNING_LOG("set hdi rotation mode failed");
        }
    }
}

AudioEffectVolume::AudioEffectVolume()
{
    SceneTypeToVolumeMap_.clear();
    dspVolume_ = 0;
}

AudioEffectVolume::~AudioEffectVolume()
{
    AUDIO_INFO_LOG("AudioEffectVolume destructor!");
}

void AudioEffectVolume::SetApVolume(std::string sceneType, uint32_t volume)
{
    if (!SceneTypeToVolumeMap_.count(sceneType)) {
        SceneTypeToVolumeMap_.insert(std::make_pair(sceneType, volume));
    } else {
        SceneTypeToVolumeMap_[sceneType] = volume;
    }
}

uint32_t AudioEffectVolume::GetApVolume(std::string sceneType)
{
    if (!SceneTypeToVolumeMap_.count(sceneType)) {
        return 0;
    } else {
        return SceneTypeToVolumeMap_[sceneType];
    }
}

void AudioEffectVolume::SetDspVolume(uint32_t volume)
{
    AUDIO_DEBUG_LOG("setDspVolume: %{public}u", volume);
    dspVolume_ = volume;
}

uint32_t AudioEffectVolume::GetDspVolume()
{
    return dspVolume_;
}

AudioEffectHdiParam::AudioEffectHdiParam()
{
    AUDIO_INFO_LOG("AudioEffectHdiParam constructor.");
    hdiControls_.clear();
    memset_s(static_cast<void *>(input), sizeof(input), 0, sizeof(input));
    memset_s(static_cast<void *>(output), sizeof(output), 0, sizeof(output));
    replyLen = GET_HDI_BUFFER_LEN;
    hdiModel_ = nullptr;
}

AudioEffectHdiParam::~AudioEffectHdiParam()
{
    AUDIO_INFO_LOG("AudioEffectHdiParam destructor!");
}

void AudioEffectHdiParam::CreateHdiControl()
{
    // todo read from vendor/huawei/...
    libName = strdup("libspatialization_processing_dsp");
    effectId = strdup("aaaabbbb-8888-9999-6666-aabbccdd9966gg");
    EffectInfo info = {
        .libName = &libName[0],
        .effectId = &effectId[0],
        .ioDirection = 1,
    };
    ControllerId controllerId;
    IEffectControl *hdiControl = nullptr;
    int32_t ret = hdiModel_->CreateEffectController(hdiModel_, &info, &hdiControl, &controllerId);
    if ((ret != 0) || (hdiControl == nullptr)) {
        AUDIO_WARNING_LOG("hdi init failed");
    } else {
        libHdiControls_.emplace_back(hdiControl);
    }
    
    return;
}

void AudioEffectHdiParam::InitHdi()
{
    hdiModel_ = IEffectModelGet(false);
    if (hdiModel_ == nullptr) {
        AUDIO_WARNING_LOG("IEffectModelGet failed");
        return;
    }

    CreateHdiControl();
}

int32_t AudioEffectHdiParam::UpdateHdiState(int8_t *effectHdiInput)
{
    for (IEffectControl hdiControl : libHdiControls_) {
        if (hdiControl == nullptr) {
            AUDIO_WARNING_LOG("hdiControl is nullptr.");
            return ERROR;
        }
        memcpy_s(static_cast<void *>(input), sizeof(input), static_cast<void *>(effectHdiInput), sizeof(input));
        uint32_t replyLen = GET_HDI_BUFFER_LEN;
        int32_t ret = hdiControl->SendCommand(hdiControl, HDI_SET_PATAM, input, SEND_HDI_COMMAND_LEN, output, &replyLen);
        if (ret != 0) {
            AUDIO_WARNING_LOG("hdi send command failed");
            return ret;
        }
    }
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS