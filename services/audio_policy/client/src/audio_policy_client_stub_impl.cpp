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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnVolumeKeyEvent ----------");
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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnAudioFocusInfoChange ----------");
    for (auto it = focusInfoChangeCallbackList_.begin(); it != focusInfoChangeCallbackList_.end(); ++it) {
        (*it)->OnAudioFocusInfoChange(focusInfoList);
    }
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyClientStubImpl::DeviceFilterByFlag(DeviceFlag flag,
    const std::vector<sptr<AudioDeviceDescriptor>>& desc)
{
    std::vector<sptr<AudioDeviceDescriptor>> descRet;
    DeviceRole role = DEVICE_ROLE_NONE;
    switch (flag) {
        case DeviceFlag::ALL_DEVICES_FLAG:
            for (sptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ == LOCAL_NETWORK_ID) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG:
            for (sptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ != LOCAL_NETWORK_ID) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::ALL_L_D_DEVICES_FLAG:
            descRet = desc;
            break;
        case DeviceFlag::OUTPUT_DEVICES_FLAG:
        case DeviceFlag::INPUT_DEVICES_FLAG:
            role = flag == INPUT_DEVICES_FLAG ? INPUT_DEVICE : OUTPUT_DEVICE;
            for (sptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ == LOCAL_NETWORK_ID && var->deviceRole_ == role) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        case DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG:
            role = flag == DISTRIBUTED_INPUT_DEVICES_FLAG ? INPUT_DEVICE : OUTPUT_DEVICE;
            for (sptr<AudioDeviceDescriptor> var : desc) {
                if (var->networkId_ != LOCAL_NETWORK_ID && var->deviceRole_ == role) {
                    descRet.insert(descRet.end(), var);
                }
            }
            break;
        default:
            AUDIO_INFO_LOG("AudioPolicyService::%{public}s:deviceFlag type are not supported", __func__);
            break;
    }
    return descRet;
}

void AudioPolicyClientStubImpl::UpdateDescWhenNoBTPermission(std::vector<sptr<AudioDeviceDescriptor>> &deviceDescs)
{
    AUDIO_WARNING_LOG("UpdateDescWhenNoBTPermission: No bt permission");

    for (sptr<AudioDeviceDescriptor> &desc : deviceDescs) {
        if ((desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) || (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO)) {
            sptr<AudioDeviceDescriptor> copyDesc = new AudioDeviceDescriptor(desc);
            copyDesc->deviceName_ = "";
            copyDesc->macAddress_ = "";
            desc = copyDesc;
        }
    }
}

int32_t AudioPolicyClientStubImpl::SetDeviceChangeCallback(const DeviceFlag &flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback> &cb)
{
    deviceChangeCallbackList_[flag] = cb;
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnSetDeviceChangeCallback()
{
    deviceChangeCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnDeviceChange(const DeviceChangeAction &dca, bool hasBTPermission)
{
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnDeviceChange ----------");
    DeviceChangeAction deviceChangeAction;
    for (auto it = deviceChangeCallbackList_.begin(); it != deviceChangeCallbackList_.end(); ++it) {
        deviceChangeAction.flag = it->first;
        deviceChangeAction.deviceDescriptors = DeviceFilterByFlag(it->first, dca.deviceDescriptors);
        if (it->second && deviceChangeAction.deviceDescriptors.size() > 0) {
            if (!(hasBTPermission)) {
                UpdateDescWhenNoBTPermission(deviceChangeAction.deviceDescriptors);
            }
            it->second->OnDeviceChange(deviceChangeAction);
        }
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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnRingerModeUpdated ----------");
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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnMicStateUpdated ----------");
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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnPreferredOutputDeviceUpdated ----------");
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
    AUDIO_INFO_LOG("AudioPolicyClientStubImpl::OnPreferredInputDeviceUpdated ----------");
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
} // namespace AudioStandard
} // namespace OHOS