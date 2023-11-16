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
#include "audio_group_manager.h"
#include "audio_routing_manager.h"

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
    int32_t SetRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb);
    int32_t UnsetRingerModeCallback();
    int32_t SetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &cb);
    int32_t UnsetMicStateChangeCallback();
    int32_t SetPreferredOutputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb);
    int32_t UnsetPreferredOutputDeviceChangeCallback();
    int32_t SetPreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb);
    int32_t UnsetPreferredInputDeviceChangeCallback();
    int32_t SetRendererStateChangeCallback(const std::shared_ptr<AudioRendererStateChangeCallback> &cb);
    int32_t UnsetRendererStateChangeCallback();
    int32_t SetCapturerStateChangeCallback(const std::shared_ptr<AudioCapturerStateChangeCallback> &cb);
    int32_t UnsetCapturerStateChangeCallback();

    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override;
    void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;
    void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;
    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
    void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;
protected:
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
private:
    std::vector<std::shared_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> focusInfoChangeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerDeviceChangeCallback>> deviceChangeCallbackList_;
    std::vector<std::shared_ptr<AudioInterruptCallback>> audioInterruptCallbackList_;
    std::vector<std::shared_ptr<AudioRingerModeCallback>> ringerModeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerMicStateChangeCallback>> micStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>> preferredOutputDeviceCallbackList_;
    std::vector<std::shared_ptr<AudioPreferredInputDeviceChangeCallback>> preferredInputDeviceCallbackList_;
    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> rendererStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioCapturerStateChangeCallback>> capturerStateChangeCallbackList_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_IMPL_H