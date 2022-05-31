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

#include "audio_errors.h"
#include "audio_policy_server.h"
#include "audio_policy_types.h"
#include "audio_log.h"
#include "audio_policy_manager_stub.h"

namespace OHOS {
namespace AudioStandard {
void AudioPolicyManagerStub::ReadAudioInterruptParams(MessageParcel &data, AudioInterrupt &audioInterrupt)
{
    audioInterrupt.streamUsage = static_cast<StreamUsage>(data.ReadInt32());
    audioInterrupt.contentType = static_cast<ContentType>(data.ReadInt32());
    audioInterrupt.streamType = static_cast<AudioStreamType>(data.ReadInt32());
    audioInterrupt.sessionID = data.ReadUint32();
}

void AudioPolicyManagerStub::ReadAudioManagerInterruptParams(MessageParcel &data, AudioInterrupt &audioInterrupt)
{
    audioInterrupt.streamUsage = static_cast<StreamUsage>(data.ReadInt32());
    audioInterrupt.contentType = static_cast<ContentType>(data.ReadInt32());
    audioInterrupt.streamType = static_cast<AudioStreamType>(data.ReadInt32());
    audioInterrupt.pauseWhenDucked = data.ReadBool();
}

void AudioPolicyManagerStub::WriteAudioInteruptParams(MessageParcel &reply, const AudioInterrupt &audioInterrupt)
{
    reply.WriteInt32(static_cast<int32_t>(audioInterrupt.streamUsage));
    reply.WriteInt32(static_cast<int32_t>(audioInterrupt.contentType));
    reply.WriteInt32(static_cast<int32_t>(audioInterrupt.streamType));
    reply.WriteUint32(audioInterrupt.sessionID);
}

void AudioPolicyManagerStub::ReadStreamChangeInfo(MessageParcel &data, const AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_PLAYBACK)
    {
        streamChangeInfo.audioRendererChangeInfo.sessionId = data.ReadInt32();        
        streamChangeInfo.audioRendererChangeInfo.rendererState = static_cast<RendererState>(data.ReadInt32());
        streamChangeInfo.audioRendererChangeInfo.clientUID = data.ReadInt32();
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.contentType = static_cast<ContentType>(data.ReadInt32());
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage = static_cast<StreamUsage>(data.ReadInt32());
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.rendererFlags = data.ReadInt32();
        return;
    }
    // mode == AUDIO_MODE_RECORDING
    streamChangeInfo.audioCapturerChangeInfo.sessionId = data.ReadInt32();
    streamChangeInfo.audioCapturerChangeInfo.capturerState = static_cast<CapturerState>(data.ReadInt32());
    streamChangeInfo.audioCapturerChangeInfo.clientUID = data.ReadInt32();
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = static_cast<SourceType>(data.ReadInt32());
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.capturerFlags = data.ReadInt32();
}

void AudioPolicyManagerStub::SetStreamVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    float volume = data.ReadFloat();
    int result = SetStreamVolume(streamType, volume);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::SetRingerModeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode rMode = static_cast<AudioRingerMode>(data.ReadInt32());
    int32_t result = SetRingerMode(rMode);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetRingerModeInternal(MessageParcel &reply)
{
    AudioRingerMode rMode = GetRingerMode();
    reply.WriteInt32(static_cast<int>(rMode));
}

void AudioPolicyManagerStub::SetAudioSceneInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioScene audioScene = static_cast<AudioScene>(data.ReadInt32());
    int32_t result = SetAudioScene(audioScene);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAudioSceneInternal(MessageParcel &reply)
{
    AudioScene audioScene = GetAudioScene();
    reply.WriteInt32(static_cast<int>(audioScene));
}

void AudioPolicyManagerStub::GetStreamVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    float volume = GetStreamVolume(streamType);
    reply.WriteFloat(volume);
}

void AudioPolicyManagerStub::SetStreamMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    bool mute = data.ReadBool();
    int result = SetStreamMute(streamType, mute);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::GetStreamMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    bool mute = GetStreamMute(streamType);
    reply.WriteBool(mute);
}

void AudioPolicyManagerStub::IsStreamActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    bool isActive = IsStreamActive(streamType);
    reply.WriteBool(isActive);
}

void AudioPolicyManagerStub::GetDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AUDIO_DEBUG_LOG("GET_DEVICES AudioManagerStub");
    int deviceFlag = data.ReadInt32();
    DeviceFlag deviceFlagConfig = static_cast<DeviceFlag>(deviceFlag);
    std::vector<sptr<AudioDeviceDescriptor>> devices = GetDevices(deviceFlagConfig);
    int32_t size = static_cast<int32_t>(devices.size());
    AUDIO_DEBUG_LOG("GET_DEVICES size= %{public}d", size);
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool active = data.ReadBool();
    int32_t result = SetDeviceActive(deviceType, active);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::IsDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool result = IsDeviceActive(deviceType);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetRingerModeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: SetRingerModeCallback obj is null");
        return;
    }
    int32_t result = SetRingerModeCallback(clientId, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetRingerModeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    int32_t result = UnsetRingerModeCallback(clientId);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioInterruptCallback obj is null");
        return;
    }
    int32_t result = SetDeviceChangeCallback(clientId, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    int32_t result = UnsetDeviceChangeCallback(clientId);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioInterruptCallback obj is null");
        return;
    }
    int32_t result = SetAudioInterruptCallback(sessionID, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    int32_t result = UnsetAudioInterruptCallback(sessionID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ActivateInterruptInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    ReadAudioInterruptParams(data, audioInterrupt);
    int32_t result = ActivateAudioInterrupt(audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::DeactivateInterruptInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    ReadAudioInterruptParams(data, audioInterrupt);
    int32_t result = DeactivateAudioInterrupt(audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t clientID = data.ReadUint32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioInterruptCallback obj is null");
        return;
    }
    int32_t result = SetAudioManagerInterruptCallback(clientID, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t clientID = data.ReadUint32();
    int32_t result = UnsetAudioManagerInterruptCallback(clientID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::RequestAudioFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    uint32_t clientID = data.ReadUint32();
    ReadAudioManagerInterruptParams(data, audioInterrupt);
    int32_t result = RequestAudioFocus(clientID, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::AbandonAudioFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    uint32_t clientID = data.ReadUint32();
    ReadAudioManagerInterruptParams(data, audioInterrupt);
    int32_t result = AbandonAudioFocus(clientID, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetStreamInFocusInternal(MessageParcel &reply)
{
    AudioStreamType streamInFocus = GetStreamInFocus();
    reply.WriteInt32(static_cast<int32_t>(streamInFocus));
}

void AudioPolicyManagerStub::GetSessionInfoInFocusInternal(MessageParcel &reply)
{
    uint32_t invalidSessionID = static_cast<uint32_t>(-1);
    AudioInterrupt audioInterrupt {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN, STREAM_DEFAULT, invalidSessionID};
    int32_t ret = GetSessionInfoInFocus(audioInterrupt);
    WriteAudioInteruptParams(reply, audioInterrupt);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::SetVolumeKeyEventCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientPid =  data.ReadInt32();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioManagerCallback obj is null");
        return;
    }
    int ret = SetVolumeKeyEventCallback(clientPid, remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnsetVolumeKeyEventCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientPid = data.ReadInt32();
    int ret = UnsetVolumeKeyEventCallback(clientPid);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::VerifyClientPermissionInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string permissionName = data.ReadString();
    uint32_t appTokenId = data.ReadUint32();
    bool ret = VerifyClientPermission(permissionName, appTokenId);
    reply.WriteBool(ret);
}

<<<<<<< HEAD
void AudioPolicyManagerStub::RegisterAudioRendererEventListenerInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUID =  data.ReadInt32();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioRendererStateCallback obj is null");
        return;
    }
    int ret = RegisterAudioRendererEventListener(clientUID, remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnregisterAudioRendererEventListenerInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUID = data.ReadInt32();
    int ret = UnregisterAudioRendererEventListener(clientUID);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::RegisterAudioCapturerEventListenerInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUID =  data.ReadInt32();
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: AudioCapturerStateCallback obj is null");
        return;
    }
    int ret = RegisterAudioCapturerEventListener(clientUID, remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnregisterAudioCapturerEventListenerInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUID = data.ReadInt32();
    int ret = UnregisterAudioCapturerEventListener(clientUID);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::RegisterTrackerInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: RegisterTracker obj is null");
        return;
    }

    AudioStreamChangeInfo streamChangeInfo = {};
    
    //streamChangeInfo.Deserialize(data, mode);
    //RegisterTracker_Internal(mode, streamChangeInfo);
    AudioMode mode = static_cast<AudioMode> (data.ReadInt32());
    ReadStreamChangeInfo(data, mode, streamChangeInfo);
    int ret = RegisterTracker(mode, streamChangeInfo);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UpdateTrackerInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    if (remoteObject == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: UpdateTracker obj is null");
        return;
    }
    //streamChangeInfo.Deserialize(data, mode);
    //UpdateTracker_Internal(mode, streamChangeInfo);
    AudioStreamChangeInfo streamChangeInfo = {};
    AudioMode mode = static_cast<AudioMode> (data.ReadInt32());
    ReadStreamChangeInfo(data, mode, streamChangeInfo);
    int ret = RegisterTracker(mode, streamChangeInfo);
=======
void AudioPolicyManagerStub::GetAudioLatencyFromXmlInternal(MessageParcel &data, MessageParcel &reply)
{
    int ret = GetAudioLatencyFromXml();
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::ReconfigureAudioChannelInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t count = data.ReadUint32();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t ret = ReconfigureAudioChannel(count, deviceType);
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
    reply.WriteInt32(ret);
}

int AudioPolicyManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub: ReadInterfaceToken failed");
        return -1;
    }
    switch (code) {
        case SET_STREAM_VOLUME:
            SetStreamVolumeInternal(data, reply);
            break;

        case SET_RINGER_MODE:
            SetRingerModeInternal(data, reply);
            break;

        case GET_RINGER_MODE:
            GetRingerModeInternal(reply);
            break;

        case SET_AUDIO_SCENE:
            SetAudioSceneInternal(data, reply);
            break;

        case GET_AUDIO_SCENE:
            GetAudioSceneInternal(reply);
            break;

        case GET_STREAM_VOLUME:
            GetStreamVolumeInternal(data, reply);
            break;

        case SET_STREAM_MUTE:
            SetStreamMuteInternal(data, reply);
            break;

        case GET_STREAM_MUTE:
            GetStreamMuteInternal(data, reply);
            break;

        case IS_STREAM_ACTIVE:
            IsStreamActiveInternal(data, reply);
            break;

        case SET_DEVICE_ACTIVE:
            SetDeviceActiveInternal(data, reply);
            break;

        case IS_DEVICE_ACTIVE:
            IsDeviceActiveInternal(data, reply);
            break;

        case SET_RINGERMODE_CALLBACK:
            SetRingerModeCallbackInternal(data, reply);
            break;

        case UNSET_RINGERMODE_CALLBACK:
            UnsetRingerModeCallbackInternal(data, reply);
            break;

        case SET_DEVICE_CHANGE_CALLBACK:
            SetDeviceChangeCallbackInternal(data, reply);
            break;

        case UNSET_DEVICE_CHANGE_CALLBACK:
            UnsetDeviceChangeCallbackInternal(data, reply);
            break;

        case SET_CALLBACK:
            SetInterruptCallbackInternal(data, reply);
            break;

        case UNSET_CALLBACK:
            UnsetInterruptCallbackInternal(data, reply);
            break;

        case ACTIVATE_INTERRUPT:
            ActivateInterruptInternal(data, reply);
            break;

        case DEACTIVATE_INTERRUPT:
            DeactivateInterruptInternal(data, reply);
            break;

        case SET_INTERRUPT_CALLBACK:
            SetAudioManagerInterruptCbInternal(data, reply);
            break;

        case UNSET_INTERRUPT_CALLBACK:
            UnsetAudioManagerInterruptCbInternal(data, reply);
            break;

        case REQUEST_AUDIO_FOCUS:
            RequestAudioFocusInternal(data, reply);
            break;

        case ABANDON_AUDIO_FOCUS:
            AbandonAudioFocusInternal(data, reply);
            break;

        case SET_VOLUME_KEY_EVENT_CALLBACK:
            SetVolumeKeyEventCallbackInternal(data, reply);
            break;

        case UNSET_VOLUME_KEY_EVENT_CALLBACK:
            UnsetVolumeKeyEventCallbackInternal(data, reply);
            break;

        case GET_STREAM_IN_FOCUS:
            GetStreamInFocusInternal(reply);
            break;

        case GET_SESSION_INFO_IN_FOCUS:
            GetSessionInfoInFocusInternal(reply);
            break;

        case GET_DEVICES:
            GetDevicesInternal(data, reply);
            break;

        case QUERY_PERMISSION:
            VerifyClientPermissionInternal(data, reply);
            break;

<<<<<<< HEAD
        case REGISTER_PLAYBACK_EVENT:
            RegisterAudioRendererEventListenerInternal(data, reply);
            break;

        case UNREGISTER_PLAYBACK_EVENT:
            UnregisterAudioRendererEventListenerInternal(data, reply);
            break;
        
        case REGISTER_RECORDING_EVENT:
            RegisterAudioCapturerEventListenerInternal(data, reply);
            break;

        case UNREGISTER_RECORDING_EVENT:
            UnregisterAudioCapturerEventListenerInternal(data, reply);
            break;
        
        case REGISTER_TRACKER:
            RegisterTrackerInternal(data, reply);
            break;

        case UPDATE_TRACKER:
            UpdateTrackerInternal(data, reply);
=======
        case RECONFIGURE_CHANNEL:
            ReconfigureAudioChannelInternal(data, reply);
            break;

        case GET_AUDIO_LATENCY:
            GetAudioLatencyFromXmlInternal(data, reply);
>>>>>>> 129e01d78cf8fcefe99a0e11743f989f485d4eb9
            break;

        default:
            AUDIO_ERR_LOG("default case, need check AudioPolicyManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return AUDIO_OK;
}
} // namespace audio_policy
} // namespace OHOS
