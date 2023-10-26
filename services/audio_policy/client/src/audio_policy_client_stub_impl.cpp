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
    }
}
} // namespace AudioStandard
} // namespace OHOS