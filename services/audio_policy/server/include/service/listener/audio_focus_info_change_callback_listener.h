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

#ifndef AUDIO_FOCUS_INFO_CHANGE_CALLBACK_LISTENER_H
#define AUDIO_FOCUS_INFO_CHANGE_CALLBACK_LISTENER_H

#include "audio_system_manager.h"
#include "audio_policy_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioFocusInfoChangeCallbackListener : public AudioFocusInfoChangeCallback {
public:
    explicit AudioFocusInfoChangeCallbackListener(const sptr<IAudioPolicyClient> &listener);
    virtual ~AudioFocusInfoChangeCallbackListener();
    DISALLOW_COPY_AND_MOVE(AudioFocusInfoChangeCallbackListener);
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override
    {
        if (listener_ != nullptr) {
            listener_->OnAudioFocusInfoChange(focusInfoList);
        }
    }

private:
    sptr<IAudioPolicyClient> listener_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_FOCUS_INFO_CHANGE_CALLBACK_LISTENER_H
