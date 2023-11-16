/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "audio_policy_client_stub_impl.h"
#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioPolicyClientStubImpl::SetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &cb)
{
    volumeKeyEventCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetVolumeKeyEventCallback()
{
    volumeKeyEventCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    for (auto it = volumeKeyEventCallbackList_.begin(); it != volumeKeyEventCallbackList_.end(); ++it) {
        (*it)->OnVolumeKeyEvent(volumeEvent);
    }
}

int32_t AudioPolicyClientStubImpl::SetFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &cb)
{
    focusInfoChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetFocusInfoChangeCallback()
{
    focusInfoChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusInfoChange(focusInfoList);
    }
}


int32_t AudioPolicyClientStubImpl::SetDeviceChangeCallback(const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    deviceChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnSetDeviceChangeCallback()
{
    deviceChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    for (auto it = deviceChangeCallbackList_.begin(); it != deviceChangeCallbackList_.end(); ++it) {
        (*it)->OnDeviceChange(deviceChangeAction);
    }
}

int32_t AudioPolicyClientStubImpl::SetAudioInterruptCallback(const std::shared_ptr<AudioInterruptCallback> &cb)
{
    audioInterruptCallbackList_.push_back(cb);
    return SUCCESS;
}
int32_t AudioPolicyClientStubImpl::UnSetAudioInterruptCallback()
{
    audioInterruptCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    for (auto it = audioInterruptCallbackList_.begin(); it != audioInterruptCallbackList_.end(); ++it) {
        (*it)->OnInterrupt(interruptEvent);
    }
}

int32_t AudioPolicyClientStubImpl::SetRingerModeCallback(const std::shared_ptr<AudioRingerModeCallback> &cb)
{
    ringerModeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetRingerModeCallback()
{
    ringerModeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    for (auto it = ringerModeCallbackList_.begin(); it != ringerModeCallbackList_.end(); ++it) {
        (*it)->OnRingerModeUpdated(ringerMode);
    }
}

int32_t AudioPolicyClientStubImpl::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &cb)
{
    micStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}
int32_t AudioPolicyClientStubImpl::UnsetMicStateChangeCallback()
{
    micStateChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    for (auto it = micStateChangeCallbackList_.begin(); it != micStateChangeCallbackList_.end(); ++it) {
        (*it)->OnMicStateUpdated(micStateChangeEvent);
    }
}

int32_t AudioPolicyClientStubImpl::SetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &cb)
{
    preferredOutputDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetPreferredOutputDeviceChangeCallback()
{
    preferredOutputDeviceCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    for (auto it = preferredOutputDeviceCallbackList_.begin(); it != preferredOutputDeviceCallbackList_.end(); ++it) {
        (*it)->OnPreferredOutputDeviceUpdated(desc);
    }
}

int32_t AudioPolicyClientStubImpl::SetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &cb)
{
    preferredInputDeviceCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetPreferredInputDeviceChangeCallback()
{
    preferredInputDeviceCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    for (auto it = preferredInputDeviceCallbackList_.begin(); it != preferredInputDeviceCallbackList_.end(); ++it) {
        (*it)->OnPreferredInputDeviceUpdated(desc);
    }
}

int32_t AudioPolicyClientStubImpl::SetRendererStateChangeCallback(
    const std::shared_ptr<AudioRendererStateChangeCallback> &cb)
{
    rendererStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetRendererStateChangeCallback()
{
    rendererStateChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    for (auto it = rendererStateChangeCallbackList_.begin(); it != rendererStateChangeCallbackList_.end(); ++it) {
        (*it)->OnRendererStateChange(audioRendererChangeInfos);
    }
}

int32_t AudioPolicyClientStubImpl::SetCapturerStateChangeCallback(
    const std::shared_ptr<AudioCapturerStateChangeCallback> &cb)
{
    capturerStateChangeCallbackList_.push_back(cb);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnsetCapturerStateChangeCallback()
{
    capturerStateChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    for (auto it = capturerStateChangeCallbackList_.begin(); it != capturerStateChangeCallbackList_.end(); ++it) {
        (*it)->OnCapturerStateChange(audioCapturerChangeInfos);
    }
}

void AudioPolicyClientStubImpl::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT)) {
        VolumeEvent volumeEvent = *(event->GetUniqueObject<VolumeEvent>());
        OnVolumeKeyEvent(volumeEvent);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED)) {
        std::list<std::pair<AudioInterrupt, AudioFocuState>> foucusInfolist =
            *(event->GetUniqueObject<std::list<std::pair<AudioInterrupt, AudioFocuState>>>());
        OnAudioFocusInfoChange(foucusInfolist);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE)) {
        DeviceChangeAction deviceChangeAction = *(event->GetUniqueObject<DeviceChangeAction>());
        OnDeviceChange(deviceChangeAction);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_INTERRUPT)) {
        InterruptEventInternal interruptEvent = *(event->GetUniqueObject<InterruptEventInternal>());
        OnInterrupt(interruptEvent);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE)) {
        AudioRingerMode ringerMode = *(event->GetUniqueObject<AudioRingerMode>());
        OnRingerModeUpdated(ringerMode);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED)) {
        MicStateChangeEvent micStateChangeEvent = *(event->GetUniqueObject<MicStateChangeEvent>());
        OnMicStateUpdated(micStateChangeEvent);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_OUTPUT_DEVICE_UPDATED)) {
        std::vector<sptr<AudioDeviceDescriptor>> desc =
            *(event->GetUniqueObject<std::vector<sptr<AudioDeviceDescriptor>>>());
        OnPreferredOutputDeviceUpdated(desc);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_INPUT_DEVICE_UPDATED)) {
        std::vector<sptr<AudioDeviceDescriptor>> desc =
            *(event->GetUniqueObject<std::vector<sptr<AudioDeviceDescriptor>>>());
        OnPreferredInputDeviceUpdated(desc);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE)) {
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos =
            std::move(*(event->GetUniqueObject<std::vector<std::unique_ptr<AudioRendererChangeInfo>>>()));
        OnRendererStateChange(audioRendererChangeInfos);
    } else if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE)) {
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos =
            std::move(*(event->GetUniqueObject<std::vector<std::unique_ptr<AudioCapturerChangeInfo>>>()));
        OnCapturerStateChange(audioCapturerChangeInfos);
    }
}
} // namespace AudioStandard
} // namespace OHOS