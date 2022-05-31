/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_policy_manager.h"
#include "audio_log.h"
#include "audio_policy_proxy.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyProxy::AudioPolicyProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAudioPolicy>(impl)
{
}

void AudioPolicyProxy::WriteAudioInteruptParams(MessageParcel &data, const AudioInterrupt &audioInterrupt)
{
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.streamUsage));
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.contentType));
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.streamType));
    data.WriteUint32(audioInterrupt.sessionID);
}

void AudioPolicyProxy::WriteAudioManagerInteruptParams(MessageParcel &data, const AudioInterrupt &audioInterrupt)
{
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.streamUsage));
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.contentType));
    data.WriteInt32(static_cast<int32_t>(audioInterrupt.streamType));
    data.WriteBool(audioInterrupt.pauseWhenDucked);
}

void AudioPolicyProxy::ReadAudioInterruptParams(MessageParcel &reply, AudioInterrupt &audioInterrupt)
{
    audioInterrupt.streamUsage = static_cast<StreamUsage>(reply.ReadInt32());
    audioInterrupt.contentType = static_cast<ContentType>(reply.ReadInt32());
    audioInterrupt.streamType = static_cast<AudioStreamType>(reply.ReadInt32());
    audioInterrupt.sessionID = reply.ReadUint32();
}

void AudioPolicyProxy::WriteStreamChangeInfo(MessageParcel &data, const AudioMode &mode, const AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_PLAYBACK)
    {
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.sessionId);
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.rendererState);
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.clientUID);
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.rendererInfo.contentType);
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage);
        data.WriteInt32(streamChangeInfo.audioRendererChangeInfo.rendererInfo.rendererFlags);
        return;
    }
    // mode == AUDIO_MODE_RECORDING
    data.WriteInt32(streamChangeInfo.audioCapturerChangeInfo.sessionId);
    data.WriteInt32(streamChangeInfo.audioCapturerChangeInfo.capturerState);
    data.WriteInt32(streamChangeInfo.audioCapturerChangeInfo.clientUID);
    data.WriteInt32(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType);
    data.WriteInt32(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.capturerFlags);
}

int32_t AudioPolicyProxy::SetStreamVolume(AudioStreamType streamType, float volume)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(static_cast<int32_t>(streamType));
    data.WriteFloat(volume);
    int32_t error = Remote()->SendRequest(SET_STREAM_VOLUME, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("set volume failed, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetRingerMode(AudioRingerMode ringMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(static_cast<int>(ringMode));
    int32_t error = Remote()->SendRequest(SET_RINGER_MODE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("set ringermode failed, error: %d", error);
        return error;
    }

    return reply.ReadInt32();
}

AudioRingerMode AudioPolicyProxy::GetRingerMode()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return RINGER_MODE_NORMAL;
    }
    int32_t error = Remote()->SendRequest(GET_RINGER_MODE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get ringermode failed, error: %d", error);
    }
    return static_cast<AudioRingerMode>(reply.ReadInt32());
}

int32_t AudioPolicyProxy::SetAudioScene(AudioScene scene)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(static_cast<int>(scene));
    int32_t error = Remote()->SendRequest(SET_AUDIO_SCENE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("set audio scene failed, error: %d", error);
        return error;
    }

    return reply.ReadInt32();
}

AudioScene AudioPolicyProxy::GetAudioScene()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return AUDIO_SCENE_DEFAULT;
    }
    int32_t error = Remote()->SendRequest(GET_AUDIO_SCENE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get audio scene failed, error: %d", error);
    }
    return static_cast<AudioScene>(reply.ReadInt32());
}

float AudioPolicyProxy::GetStreamVolume(AudioStreamType streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(static_cast<int32_t>(streamType));
    int32_t error = Remote()->SendRequest(GET_STREAM_VOLUME, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get volume failed, error: %d", error);
        return error;
    }
    return reply.ReadFloat();
}

int32_t AudioPolicyProxy::SetStreamMute(AudioStreamType streamType, bool mute)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(static_cast<int32_t>(streamType));
    data.WriteBool(mute);
    int32_t error = Remote()->SendRequest(SET_STREAM_MUTE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("set mute failed, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

bool AudioPolicyProxy::GetStreamMute(AudioStreamType streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return false;
    }
    data.WriteInt32(static_cast<int32_t>(streamType));
    int32_t error = Remote()->SendRequest(GET_STREAM_MUTE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get mute failed, error: %d", error);
        return false;
    }
    return reply.ReadBool();
}

bool AudioPolicyProxy::IsStreamActive(AudioStreamType streamType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return false;
    }
    data.WriteInt32(static_cast<int32_t>(streamType));
    int32_t error = Remote()->SendRequest(IS_STREAM_ACTIVE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("isStreamActive failed, error: %d", error);
        return false;
    }
    return reply.ReadBool();
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyProxy::GetDevices(DeviceFlag deviceFlag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<sptr<AudioDeviceDescriptor>> deviceInfo;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return deviceInfo;
    }
    data.WriteInt32(static_cast<int32_t>(deviceFlag));
    int32_t error = Remote()->SendRequest(GET_DEVICES, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("Get devices failed, error: %d", error);
        return deviceInfo;
    }

    int32_t size = reply.ReadInt32();
    for (int32_t i = 0; i < size; i++) {
        deviceInfo.push_back(AudioDeviceDescriptor::Unmarshalling(reply));
    }

    return deviceInfo;
}

int32_t AudioPolicyProxy::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(static_cast<int32_t>(deviceType));
    data.WriteBool(active);
    int32_t error = Remote()->SendRequest(SET_DEVICE_ACTIVE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("set device active failed, error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

bool AudioPolicyProxy::IsDeviceActive(InternalDeviceType deviceType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return false;
    }
    data.WriteInt32(static_cast<int32_t>(deviceType));
    int32_t error = Remote()->SendRequest(IS_DEVICE_ACTIVE, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("is device active failed, error: %d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t AudioPolicyProxy::SetRingerModeCallback(const int32_t clientId, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyProxy: SetRingerModeCallback object is null");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(clientId);
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(SET_RINGERMODE_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: set ringermode callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetRingerModeCallback(const int32_t clientId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(clientId);
    int error = Remote()->SendRequest(UNSET_RINGERMODE_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: unset ringermode callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetDeviceChangeCallback(const int32_t clientId, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyProxy: SetDeviceChangeCallback object is null");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(clientId);
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(SET_DEVICE_CHANGE_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: SetDeviceChangeCallback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetDeviceChangeCallback(const int32_t clientId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteInt32(clientId);
    int error = Remote()->SendRequest(UNSET_DEVICE_CHANGE_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: unset device change callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyProxy: SetAudioInterruptCallback object is null");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteUint32(sessionID);
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(SET_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: set callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioInterruptCallback(const uint32_t sessionID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteUint32(sessionID);
    int error = Remote()->SendRequest(UNSET_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: unset callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    WriteAudioInteruptParams(data, audioInterrupt);
    int error = Remote()->SendRequest(ACTIVATE_INTERRUPT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: activate interrupt failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    WriteAudioInteruptParams(data, audioInterrupt);
    int error = Remote()->SendRequest(DEACTIVATE_INTERRUPT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: deactivate interrupt failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetAudioManagerInterruptCallback(const uint32_t clientID, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyProxy: SetAudioManagerInterruptCallback object is null");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteUint32(clientID);
    (void)data.WriteRemoteObject(object);
    int error = Remote()->SendRequest(SET_INTERRUPT_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: set callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetAudioManagerInterruptCallback(const uint32_t clientID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteUint32(clientID);

    int error = Remote()->SendRequest(UNSET_INTERRUPT_CALLBACK, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: unset callback failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::RequestAudioFocus(const uint32_t clientID, const AudioInterrupt &audioInterrupt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteUint32(clientID);
    WriteAudioManagerInteruptParams(data, audioInterrupt);

    int error = Remote()->SendRequest(REQUEST_AUDIO_FOCUS, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: activate interrupt failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::AbandonAudioFocus(const uint32_t clientID, const AudioInterrupt &audioInterrupt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    data.WriteUint32(clientID);
    WriteAudioManagerInteruptParams(data, audioInterrupt);

    int error = Remote()->SendRequest(ABANDON_AUDIO_FOCUS, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy: deactivate interrupt failed, error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

AudioStreamType AudioPolicyProxy::GetStreamInFocus()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return STREAM_DEFAULT;
    }
    int32_t error = Remote()->SendRequest(GET_STREAM_IN_FOCUS, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("get stream in focus failed, error: %d", error);
    }
    return static_cast<AudioStreamType>(reply.ReadInt32());
}

int32_t AudioPolicyProxy::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    int32_t error = Remote()->SendRequest(GET_SESSION_INFO_IN_FOCUS, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("AudioPolicyProxy::GetSessionInfoInFocus failed, error: %d", error);
    }
    ReadAudioInterruptParams(reply, audioInterrupt);

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::SetVolumeKeyEventCallback(const int32_t clientPid, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }
    if (object == nullptr) {
        AUDIO_ERR_LOG("VolumeKeyEventCallback object is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteInt32(clientPid);
    data.WriteRemoteObject(object);
    int result = Remote()->SendRequest(SET_VOLUME_KEY_EVENT_CALLBACK, data, reply, option);
    if (result != ERR_NONE) {
        AUDIO_ERR_LOG("SetAudioVolumeKeyEventCallback failed, result: %{public}d", result);
        return result;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnsetVolumeKeyEventCallback(const int32_t clientPid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(clientPid);
    int result = Remote()->SendRequest(UNSET_VOLUME_KEY_EVENT_CALLBACK, data, reply, option);
    if (result != ERR_NONE) {
        AUDIO_ERR_LOG("UnsetVolumeKeyEventCallback failed, result: %{public}d", result);
        return result;
    }

    return reply.ReadInt32();
}

bool AudioPolicyProxy::VerifyClientPermission(const std::string &permissionName, uint32_t appTokenId)
{
    AUDIO_ERR_LOG("VerifyClientPermission [permission : %{public}s] | [tid : %{public}d]",
        permissionName.c_str(), appTokenId);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("VerifyClientPermission: WriteInterfaceToken failed");
        return false;
    }

    data.WriteString(permissionName);
    data.WriteUint32(appTokenId);

    int result = Remote()->SendRequest(QUERY_PERMISSION, data, reply, option);
    if (result != ERR_NONE) {
        AUDIO_ERR_LOG("VerifyClientPermission failed, result: %{public}d", result);
        return false;
    }

    return reply.ReadBool();
}

<<<<<<< HEAD
int32_t AudioPolicyProxy::RegisterAudioRendererEventListener(const int32_t clientUID, const sptr<IRemoteObject> &object)
{
=======
int32_t AudioPolicyProxy::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
    AUDIO_ERR_LOG("ReconfigureAudioChannel proxy %{public}d, %{public}d", count, deviceType);
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

<<<<<<< HEAD
    AUDIO_INFO_LOG("AudioPolicyProxy::RegisterAudioRendererEventListener");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("RegisterAudioRendererEventListener: WriteInterfaceToken failed");
        return -1;
    }
    if (object == nullptr) {
        AUDIO_ERR_LOG("RegisterAudioRendererEventListener Event object is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteInt32(clientUID);
    data.WriteRemoteObject(object);
    int32_t error = Remote() ->SendRequest(REGISTER_PLAYBACK_EVENT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("RegisterAudioRendererEventListener register playback event faild , error: %d", error);
        return -1;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnregisterAudioRendererEventListener(const int32_t clientUID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    AUDIO_INFO_LOG("AudioPolicyProxy::UnregisterAudioRendererEventListener");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("UnregisterAudioRendererEventListener WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(clientUID);
    int32_t error = Remote() ->SendRequest(UNREGISTER_PLAYBACK_EVENT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("UnregisterAudioRendererEventListener unregister playback event failed , error: %d", error);
        return -1;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::RegisterAudioCapturerEventListener(const int32_t clientUID, const sptr<IRemoteObject> &object)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    AUDIO_INFO_LOG("AudioPolicyProxy::RegisterAudioCapturerEventListener");

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("RegisterAudioCapturerEventListener:: WriteInterfaceToken failed");
        return -1;
    }
    if (object == nullptr) {
        AUDIO_ERR_LOG("RegisterAudioCapturerEventListener Event object is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteInt32(clientUID);
    data.WriteRemoteObject(object);
    int32_t error = Remote() ->SendRequest(REGISTER_RECORDING_EVENT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("RegisterAudioCapturerEventListener recording event faild , error: %d", error);
        return -1;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::UnregisterAudioCapturerEventListener(const int32_t clientUID)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    AUDIO_INFO_LOG("AudioPolicyProxy::UnregisterAudioCapturerEventListener");

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy:: WriteInterfaceToken failed");
        return -1;
    }

    data.WriteInt32(clientUID);
    int32_t error = Remote() ->SendRequest(UNREGISTER_RECORDING_EVENT, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("UnregisterAudioCapturerEventListener recording event faild , error: %d", error);
        return -1;
    }

    return reply.ReadInt32();
}

int32_t AudioPolicyProxy::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    AUDIO_INFO_LOG("AudioPolicyProxy::RegisterTracker");

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("RegisterTracker WriteInterfaceToken failed");
        return -1;
    }

    //streamChangeInfo.Serialize(data, mode);
    data.WriteUint32(mode);
    WriteStreamChangeInfo(data, mode, streamChangeInfo);

    int32_t error = Remote()->SendRequest(REGISTER_TRACKER, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("RegisterTracker event failed , error: %d", error);
        return -1;
=======
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("ReconfigureAudioChannel: WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    data.WriteUint32(count);
    data.WriteInt32(deviceType);

    int result = Remote()->SendRequest(RECONFIGURE_CHANNEL, data, reply, option);
    if (result != ERR_NONE) {
        AUDIO_ERR_LOG("ReconfigureAudioChannel failed, result: %{public}d", result);
        return ERR_TRANSACTION_FAILED;
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
    }

    return reply.ReadInt32();
}

<<<<<<< HEAD
int32_t AudioPolicyProxy::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
=======
int32_t AudioPolicyProxy::GetAudioLatencyFromXml()
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

<<<<<<< HEAD
    AUDIO_INFO_LOG("AudioPolicyProxy::UpdateTracker");

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("UpdateTracker: WriteInterfaceToken failed");
        return -1;
    }

    //streamChangeInfo.Serialize(data, mode);
    data.WriteUint32(mode);
    WriteStreamChangeInfo(data, mode, streamChangeInfo);

    int32_t error = Remote()->SendRequest(UPDATE_TRACKER, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("UpdateTracker event failed , error: %d", error);
        return -1;
=======
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyProxy: GetAudioLatencyFromXml WriteInterfaceToken failed");
        return IPC_PROXY_ERR;
    }

    int32_t error = Remote()->SendRequest(GET_AUDIO_LATENCY, data, reply, option);
    if (error != ERR_NONE) {
        AUDIO_ERR_LOG("GetAudioLatencyFromXml, error: %d", error);
        return ERR_TRANSACTION_FAILED;
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
    }

    return reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
