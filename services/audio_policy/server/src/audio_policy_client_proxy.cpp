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

#include "audio_policy_client_proxy.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyClientProxy::AudioPolicyClientProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAudioPolicyClient>(impl)
{}

AudioPolicyClientProxy::~AudioPolicyClientProxy()
{}

AudioEnhancementMonitoringCallback::AudioEnhancementMonitoringCallback(const sptr<IAudioPolicyClient> &listener)
: listener_(listener) {}

AudioEnhancementMonitoringCallback::~AudioEnhancementMonitoringCallback() {}

int32_t AudioPolicyClientProxy::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const uint32_t code)
{
    if (code > static_cast<uint32_t>(AudioPolicyClientCode::AUDIO_POLICY_CLIENT_CODE_MAX)) {
        return -1;
    }
    return (this->*handlers[code])(object);
}

void AudioPolicyClientProxy::UnregisterPolicyCallbackClient(const uint32_t code)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT):
            volumeKeyEventCallbackList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED):
            focusInfoChangeCallbackList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE):
            deviceChangeCallbackList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_INTERRUPT):
            audioInterruptCallbackList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE):
            audioRingerModeCallbackList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED):
            amMicStateChangeCallbackList_.clear();
            break;
        default:
            break;
    }
}

int32_t AudioPolicyClientProxy::SetVolumeKeyEventCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetVolumeKeyEventCallback listener obj cast failed");
    std::shared_ptr<VolumeKeyEventCallback> callback = std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetVolumeKeyEventCallback failed to create cb obj");
    volumeKeyEventCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    if (volumeKeyEventCallbackList_.empty()) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnVolumeKeyEvent: WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT));
    data.WriteInt32(static_cast<int32_t>(volumeEvent.volumeType));
    data.WriteInt32(volumeEvent.volume);
    data.WriteBool(volumeEvent.updateUi);
    data.WriteInt32(volumeEvent.volumeGroupId);
    data.WriteString(volumeEvent.networkId);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending volume key event %{public}d", error);
    }
    reply.ReadInt32();
}


int32_t AudioPolicyClientProxy::SetFocusInfoChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetFocusInfoChangeCallback listener obj cast failed");
    std::shared_ptr<AudioFocusInfoChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetFocusInfoChangeCallback failed to create cb obj");
    focusInfoChangeCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    if (focusInfoChangeCallbackList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnAudioFocusInfoChange: WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED));
    size_t size = focusInfoList.size();
    data.WriteInt32(static_cast<int32_t>(size));
    for (auto iter = focusInfoList.begin(); iter != focusInfoList.end(); ++iter) {
        iter->first.Marshalling(data);
        data.WriteInt32(iter->second);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending focus change info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetDeviceChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetFocusInfoChangeCallback listener obj cast failed");
    std::shared_ptr<AudioManagerDeviceChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetFocusInfoChangeCallback failed to create cb obj");
    deviceChangeCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    if (deviceChangeCallbackList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnDeviceChange: WriteInterfaceToken failed");
        return;
    }

    auto devices = deviceChangeAction.deviceDescriptors;
    size_t size = deviceChangeAction.deviceDescriptors.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE));
    data.WriteInt32(deviceChangeAction.type);
    data.WriteInt32(deviceChangeAction.flag);
    data.WriteInt32(static_cast<int32_t>(size));

    for (size_t i = 0; i < size; i++) {
        devices[i]->Marshalling(data);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending device change info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetAudioInterruptCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetAudioInterruptCallback listener obj cast failed");
    std::shared_ptr<AudioInterruptCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetFocusInfoChangeCallback failed to create cb obj");
    audioInterruptCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    if (audioInterruptCallbackList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnInterrupt: WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_INTERRUPT));
    data.WriteInt32(static_cast<int32_t>(interruptEvent.eventType));
    data.WriteInt32(static_cast<int32_t>(interruptEvent.forceType));
    data.WriteInt32(static_cast<int32_t>(interruptEvent.hintType));
    data.WriteFloat(interruptEvent.duckVolume);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending interrupt info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetRingerModeUpdatedCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetRingerModeUpdatedCallback listener obj cast failed");
    std::shared_ptr<AudioRingerModeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetRingerModeUpdatedCallback failed to create cb obj");
    audioRingerModeCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    if (audioRingerModeCallbackList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnRingerModeUpdated: WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE));
    data.WriteInt32(static_cast<int32_t>(ringerMode));
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetMicStateChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetMicStateChangeCallback listener obj cast failed");
    std::shared_ptr<AudioManagerMicStateChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetMicStateChangeCallback failed to create cb obj");
    amMicStateChangeCallbackList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    if (amMicStateChangeCallbackList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnRingerModeUpdated: WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED));
    data.WriteBool(micStateChangeEvent.mute);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
