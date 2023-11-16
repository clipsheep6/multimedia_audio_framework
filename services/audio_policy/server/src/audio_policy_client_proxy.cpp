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
#include "audio_enhancement_monitoring_callback_listerner.h"
#include "audio_group_handle.h"

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
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_OUTPUT_DEVICE_UPDATED):
            preferredOutputDeviceCbList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_ACTIVE_INPUT_DEVICE_UPDATED):
            preferredInputDeviceCbList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE):
            rendererStateChangeCbList_.clear();
            break;
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE):
            capturerStateChangeCbList_.clear();
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
        AUDIO_ERR_LOG("Error while sending volume key event %{public}d", error);
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
        AUDIO_ERR_LOG("Error while sending focus change info: %{public}d", error);
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
        AUDIO_ERR_LOG("Error while sending device change info: %{public}d", error);
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
        AUDIO_ERR_LOG("Error while sending interrupt info: %{public}d", error);
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
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
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
        AUDIO_ERR_LOG("Error while sending ringer mode updated info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetPreferredOutputDeviceChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetPreferredOutputDeviceChangeCallback listener obj cast failed");
    std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetPreferredOutputDeviceChangeCallback failed to create cb obj");
    preferredOutputDeviceCbList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnPreferredOutputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    if (preferredOutputDeviceCbList_.empty()) {
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

int32_t AudioPolicyClientProxy::SetPreferredInputDeviceChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetPreferredInputDeviceChangeCallback listener obj cast failed");
    std::shared_ptr<AudioPreferredInputDeviceChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetPreferredInputDeviceChangeCallback failed to create cb obj");
    preferredInputDeviceCbList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::OnPreferredInputDeviceUpdated(const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    if (preferredInputDeviceCbList_.empty()) {
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

int32_t AudioPolicyClientProxy::SetRendererStateChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetRendererStateChangeCallback listener obj cast failed");
    std::shared_ptr<AudioRendererStateChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetRendererStateChangeCallback failed to create cb obj");
    rendererStateChangeCbList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::UpdateRendererDeviceInfo(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos)
{
    if (!hasBTPermission_) {
        size_t rendererChangeInfoLength = rendererChangeInfos.size();
        for (size_t i = 0; i < rendererChangeInfoLength; i++) {
            if ((rendererChangeInfos[i]->outputDeviceInfo.deviceType == DEVICE_TYPE_BLUETOOTH_A2DP)
                || (rendererChangeInfos[i]->outputDeviceInfo.deviceType == DEVICE_TYPE_BLUETOOTH_SCO)) {
                rendererChangeInfos[i]->outputDeviceInfo.deviceName = "";
                rendererChangeInfos[i]->outputDeviceInfo.macAddress = "";
            }
        }
    }

    if (!hasSystemPermission_) {
        size_t rendererChangeInfoLength = rendererChangeInfos.size();
        for (size_t i = 0; i < rendererChangeInfoLength; i++) {
            rendererChangeInfos[i]->clientUID = 0;
            rendererChangeInfos[i]->rendererState = RENDERER_INVALID;
            rendererChangeInfos[i]->outputDeviceInfo.networkId = "";
            rendererChangeInfos[i]->outputDeviceInfo.interruptGroupId = GROUP_ID_NONE;
            rendererChangeInfos[i]->outputDeviceInfo.volumeGroupId = GROUP_ID_NONE;
        }
    }
}

void AudioPolicyClientProxy::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    if (rendererStateChangeCbList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnRendererStateChange: WriteInterfaceToken failed");
        return;
    }

    UpdateRendererDeviceInfo(audioRendererChangeInfos);
    size_t size = audioRendererChangeInfos.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE));
    data.WriteInt32(size);
    for (const std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo: audioRendererChangeInfos) {
        if (!rendererChangeInfo) {
            AUDIO_ERR_LOG("Renderer change info null, something wrong!!");
            continue;
        }
        rendererChangeInfo->Marshalling(data);
    }
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending renderer state change info: %{public}d", error);
    }
    reply.ReadInt32();
}

int32_t AudioPolicyClientProxy::SetCapturerStateChangeCallback(const sptr<IRemoteObject> &object)
{
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetCapturerStateChangeCallback listener obj cast failed");
    std::shared_ptr<AudioCapturerStateChangeCallback> callback =
        std::make_shared<AudioEnhancementMonitoringCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetCapturerStateChangeCallback failed to create cb obj");
    capturerStateChangeCbList_.push_back(callback);

    return SUCCESS;
}

void AudioPolicyClientProxy::UpdateCapturerDeviceInfo(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos)
{
    if (!hasBTPermission_) {
        size_t capturerChangeInfoLength = capturerChangeInfos.size();
        for (size_t i = 0; i < capturerChangeInfoLength; i++) {
            if ((capturerChangeInfos[i]->inputDeviceInfo.deviceType == DEVICE_TYPE_BLUETOOTH_A2DP)
                || (capturerChangeInfos[i]->inputDeviceInfo.deviceType == DEVICE_TYPE_BLUETOOTH_SCO)) {
                capturerChangeInfos[i]->inputDeviceInfo.deviceName = "";
                capturerChangeInfos[i]->inputDeviceInfo.macAddress = "";
            }
        }
    }

    if (!hasSystemPermission_) {
        size_t capturerChangeInfoLength = capturerChangeInfos.size();
        for (size_t i = 0; i < capturerChangeInfoLength; i++) {
            capturerChangeInfos[i]->clientUID = 0;
            capturerChangeInfos[i]->capturerState = CAPTURER_INVALID;
            capturerChangeInfos[i]->inputDeviceInfo.networkId = "";
            capturerChangeInfos[i]->inputDeviceInfo.interruptGroupId = GROUP_ID_NONE;
            capturerChangeInfos[i]->inputDeviceInfo.volumeGroupId = GROUP_ID_NONE;
        }
    }
}

void AudioPolicyClientProxy::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    if (capturerStateChangeCbList_.empty()) {
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy::OnCapturerStateChange: WriteInterfaceToken failed");
        return;
    }

    UpdateCapturerDeviceInfo(audioCapturerChangeInfos);
    size_t size = audioCapturerChangeInfos.size();
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE));
    data.WriteInt32(size);
    for (const std::unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo: audioCapturerChangeInfos) {
        if (!capturerChangeInfo) {
            AUDIO_ERR_LOG("Capturer change info null, something wrong!!");
            continue;
        }
        capturerChangeInfo->Marshalling(data);
    }

    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_ERR_LOG("Error while sending capturer state change info: %{public}d", error);
    }
    reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
