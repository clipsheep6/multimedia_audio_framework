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

#ifndef ST_AUDIO_POLICY_CLIENT_PROXY_H
#define ST_AUDIO_POLICY_CLIENT_PROXY_H

#include "audio_enhancement_monitoring_callback_listerner.h"
//#include "audio_policy_client.h"
//#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_interrupt_callback.h"
//#include "audio_group_manager.h"


namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientProxy : public IRemoteProxy<IAudioPolicyClient> {
public:
    AudioPolicyClientProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioPolicyClientProxy();
    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const uint32_t code);
    void UnregisterPolicyCallbackClient(const uint32_t code);

    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override;
    void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;
    void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) override;

    bool hasBTPermission_ = true;
    bool hasSystemPermission_ = true;
private:
    static inline BrokerDelegator<AudioPolicyClientProxy> delegator_;
    int32_t SetVolumeKeyEventCallback(const sptr<IRemoteObject> &object);
    int32_t SetFocusInfoChangeCallback(const sptr<IRemoteObject> &object);
    int32_t SetDeviceChangeCallback(const sptr<IRemoteObject> &object);
    int32_t SetAudioInterruptCallback(const sptr<IRemoteObject> &object);
    int32_t SetRingerModeUpdatedCallback(const sptr<IRemoteObject> &object);
    int32_t SetMicStateChangeCallback(const sptr<IRemoteObject> &object);
    int32_t SetPreferredOutputDeviceChangeCallback(const sptr<IRemoteObject> &object);
    int32_t SetPreferredInputDeviceChangeCallback(const sptr<IRemoteObject> &object);

    using HandlerFunc = int32_t(AudioPolicyClientProxy::*)(const sptr<IRemoteObject> &object);
    static inline HandlerFunc handlers[] = {
        &AudioPolicyClientProxy::SetVolumeKeyEventCallback,
        &AudioPolicyClientProxy::SetFocusInfoChangeCallback,
        &AudioPolicyClientProxy::SetDeviceChangeCallback,
        &AudioPolicyClientProxy::SetAudioInterruptCallback,
        &AudioPolicyClientProxy::SetRingerModeUpdatedCallback,
        &AudioPolicyClientProxy::SetMicStateChangeCallback,
        &AudioPolicyClientProxy::SetPreferredOutputDeviceChangeCallback,
        &AudioPolicyClientProxy::SetPreferredInputDeviceChangeCallback,
    };

    std::vector<std::shared_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> focusInfoChangeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerDeviceChangeCallback>> deviceChangeCallbackList_;
    std::vector<std::shared_ptr<AudioInterruptCallback>> audioInterruptCallbackList_;
    std::vector<std::shared_ptr<AudioRingerModeCallback>> audioRingerModeCallbackList_;
    std::vector<std::shared_ptr<AudioManagerMicStateChangeCallback>> amMicStateChangeCallbackList_;
    std::vector<std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>> preferredOutputDeviceCbList_;
    std::vector<std::shared_ptr<AudioPreferredInputDeviceChangeCallback>> preferredInputDeviceCbList_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_PROXY_H
