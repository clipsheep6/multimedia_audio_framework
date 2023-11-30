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

#ifndef ST_AUDIO_POLICY_CLIENT_H
#define ST_AUDIO_POLICY_CLIENT_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_interrupt_info.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t UPDATE_CALLBACK_CLIENT = 0;

enum class AudioPolicyClientCode {
    ON_VOLUME_KEY_EVENT = 0,
    ON_FOCUS_INFO_CHANGED,
    ON_FOCUS_REQUEST_CHANGED,
    ON_FOCUS_ABANDON_CHANGED,
    ON_DEVICE_CHANGE,
    ON_RINGERMODE_UPDATE,
    ON_MIC_STATE_UPDATED,
    ON_ACTIVE_OUTPUT_DEVICE_UPDATED,
    ON_ACTIVE_INPUT_DEVICE_UPDATED,
    ON_RENDERERSTATE_CHANGE,
    ON_CAPTURERSTATE_CHANGE,
    AUDIO_POLICY_CLIENT_CODE_MAX = ON_CAPTURERSTATE_CHANGE,
};
class IAudioPolicyClient : public IRemoteBroker {
public:
    virtual void OnVolumeKeyEvent(VolumeEvent volumeEvent) = 0;
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) = 0;
    virtual void OnAudioFocusRequested(const AudioInterrupt &requestFocus) = 0;
    virtual void OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus) = 0;
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) =0;
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) = 0;
    virtual void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) = 0;
    virtual void OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
    virtual void OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc) = 0;
    virtual void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) = 0;
    virtual void OnCapturerStateChange(
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) = 0;

    bool hasBTPermission_ = true;
    bool hasSystemPermission_ = true;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioPolicyClient");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_H
