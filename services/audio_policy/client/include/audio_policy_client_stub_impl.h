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

#ifndef AUDIO_POLICY_CLIENT_STUB_IMPL_H
#define AUDIO_POLICY_CLIENT_STUB_IMPL_H

#include "audio_policy_client_stub.h"
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientStubImpl : public AudioPolicyClientStub {
public:
    int32_t SetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb);
    int32_t UnsetVolumeKeyEventCallback();
    int32_t SetFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &cb);
    int32_t UnsetFocusInfoChangeCallback();
    int32_t SetDeviceChangeCallback(const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);
    int32_t UnSetDeviceChangeCallback();
    int32_t SetAudioInterruptCallback(const std::shared_ptr<AudioInterruptCallback> &cb);
    int32_t UnSetAudioInterruptCallback();
protected:
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
private:
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;

    std::vector<std::shared_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> focusInfoChangeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerDeviceChangeCallback>> deviceChangeCallbackList_;
    std::vector<std::shared_ptr<AudioInterruptCallback>> audioInterruptCallbackList_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_IMPL_H