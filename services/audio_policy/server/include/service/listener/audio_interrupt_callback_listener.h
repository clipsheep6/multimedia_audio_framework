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

#ifndef AUDIO_INTERRUPT_CALLBACK_LISTENER_H
#define AUDIO_INTERRUPT_CALLBACK_LISTENER_H

//#include "audio_system_manager.h"
#include "audio_policy_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioInterruptCallbackListener : public AudioInterruptCallback {
public:
    explicit AudioInterruptCallbackListener(const sptr<IAudioPolicyClient> &listener);
    virtual ~AudioInterruptCallbackListener();
    DISALLOW_COPY_AND_MOVE(AudioInterruptCallbackListener);
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override
    {
        if (listener_ != nullptr) {
            listener_->OnInterrupt(interruptEvent);
        }
    }

private:
    sptr<IAudioPolicyClient> listener_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INTERRUPT_CALLBACK_LISTENER_H
