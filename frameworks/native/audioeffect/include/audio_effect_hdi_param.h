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

#ifndef AUDIO_EFFECT_HDI_PARAM_H
#define AUDIO_EFFECT_HDI_PARAM_H

#include <mutex>
#include "dm_common.h"

namespace OHOS {
namespace AudioStandard {
class AudioEffectRotation {
public:
    AudioEffectRotation();
    ~AudioEffectRotation();
    int32_t Init();
    int32_t Enable();
    int32_t Disable();
private:
    class AudioDispalyListener : public OHOS::Rosen::DisplayManager::IDisplayListener {
    public:
        AudioDispalyListener(const AudioEffectRotation& audioEffectRotation) 
        {
            audioEffectRotation_ = audioEffectRotation;
        }
        void OnCreate(Rosen::DisplayId displayId) override
        {
            if (audioEffectRotation_ != nullptr) {
                audioEffectRotation_->OnCreate(displayId);
            }
        }
        void OnDestroy(Rosen::DisplayId displayId) override
        {
            if (audioEffectRotation_ != nullptr) {
                audioEffectRotation_->OnDestroy(displayId);
            }
        }
        void OnChange(Rosen::DisplayId displayId) override
        {
            if (audioEffectRotation_ != nullptr) {
                audioEffectRotation_->OnChange(displayId);
            }
        }
    }

    uint32_t rotationState_;
    sptr<AudioDispalyListener> audioDisplayListener_ = nullptr;
    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);
}

class AudioEffectVolume {
public:
    AudioEffectVolume();
    ~AudioEffectVolume();
    void SetApVolume(std::string sceneType, uint32_t volume);
    uint32_t GetApVolume(std::string sceneType);
    void SetDspVolume(uint32_t volume);
    uint32_t GetDspVolume();
private:
    uint32_t dspVolume_;
    std::map<std::string, uint32_t>SceneTypeToVolumeMap_;
}

class AudioEffectHdiParam {
public:
    AudioEffectHdiParam();
    ~AudioEffectHdiParam();
    void Init();
    int32_t UpdateHdiState(int8_t *effectHdiInput);
private:
    int8_t input[SEND_HDI_COMMAND_LEN];
    int8_t output[GET_HDI_BUFFER_LEN];
    uint32_t replyLen;
    std::string libName;
    std::string effectId;
    IEffectModel *hdiModel_;
    std::vector<IEffectControl *> libHdiControls_;
    void CreateHdiControl();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_PARAM_MANAGER_H