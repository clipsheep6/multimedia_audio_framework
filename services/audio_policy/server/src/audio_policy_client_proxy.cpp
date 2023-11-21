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
{
    AUDIO_ERR_LOG("AudioPolicyClientProxy::AudioPolicyClientProxy ---------construct---.");
}

AudioPolicyClientProxy::~AudioPolicyClientProxy()
{
    AUDIO_ERR_LOG("AudioPolicyClientProxy::AudioPolicyClientProxy ---------destruct---.");
}

AudioEnhancementMonitoringCallback::AudioEnhancementMonitoringCallback(const sptr<IAudioPolicyClient> &listener)
: listener_(listener) {}

AudioEnhancementMonitoringCallback::~AudioEnhancementMonitoringCallback() {}

int32_t AudioPolicyClientProxy::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("AudioPolicyManager::RegisterPolicyCallbackClientFunc: ------------");
    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::RegisterPolicyCallbackClient ------------.");
        return -1;
    }
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetVolumeKeyEventCallback listener obj cast failed");
    audioCallback_ = std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(audioCallback_ != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient failed to create cb obj");

    return SUCCESS;
}

void AudioPolicyClientProxy::UnregisterPolicyCallbackClient()
{
    if (audioCallback_ != nullptr) {
        audioCallback_.reset();
        audioCallback_ = nullptr;
    }
}

void AudioPolicyClientProxy::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending volume key event %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending focus change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending device change info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending interrupt info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    if (audioCallback_ == nullptr) {
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
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    if (audioCallback_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnPreferredOutputDeviceUpdated: WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_ACTIVE_OUTPUT_DEVICE_UPDATED));
    int32_t size = static_cast<int32_t>(desc.size());
    data.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        desc[i]->Marshalling(data);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending preferred output device updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

void AudioPolicyClientProxy::OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    if (audioCallback_ == nullptr) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnPreferredInputDeviceUpdated: WriteInterfaceToken failed");
        return;
    }

    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_ACTIVE_INPUT_DEVICE_UPDATED));
    int32_t size = static_cast<int32_t>(desc.size());
    data.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        desc[i]->Marshalling(data);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT),
        data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending preferred input device updated info: %{public}d", error);
    }
    reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
