/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <iostream>
#include "audio_policy_server.h"
#include <cstddef>
#include <cstdint>
#include "message_parcel.h"
#include "audio_info.h"
#include "audio_stream_collector.h"
#include "audio_client_tracker_callback_proxy.h"
#include "audio_policy_proxy.h"
using namespace std;

namespace OHOS {
    constexpr int32_t OFFSET = 4;
    const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
    const int32_t SYSTEM_ABILITY_ID = 3009;
    const bool RUN_ON_CREATE = false;
    namespace AudioStandard {
        uint32_t Convert2Uint32(const uint8_t *ptr)
        {
            if (ptr == nullptr)
            {
                return 0;
            }
            // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
            return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
        }
        void AudioPolicyFuzzTestOne(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            uint32_t code = Convert2Uint32(rawData);
            rawData = rawData + OFFSET;
            size = size - OFFSET;

            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            MessageParcel reply;
            MessageOption option;
            std::shared_ptr<AudioPolicyServer> AudioPolicyServerPtr =
                std::make_shared<AudioPolicyServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
            AudioPolicyServerPtr->OnRemoteRequest(code, data, reply, option);
        }
        void AudioPolicyFuzzTestTwo(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            std::shared_ptr<AudioPolicyServer> AudioPolicyServerPtr =
                std::make_shared<AudioPolicyServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

            AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            float volume = *reinterpret_cast<const float *>(rawData);
            int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
            bool mute = *reinterpret_cast<const bool *>(rawData);
            AudioPolicyServerPtr->SetStreamVolume(streamType, volume);
            AudioPolicyServerPtr->GetStreamVolume(streamType);
            AudioPolicyServerPtr->SetLowPowerVolume(streamId, volume);
            AudioPolicyServerPtr->GetLowPowerVolume(streamId);
            AudioPolicyServerPtr->GetSingleStreamVolume(streamId);
            AudioPolicyServerPtr->SetStreamMute(streamType, mute);
            AudioPolicyServerPtr->GetStreamMute(streamType);
            AudioPolicyServerPtr->IsStreamActive(streamType);

            InternalDeviceType deviceType = *reinterpret_cast<const InternalDeviceType *>(rawData);
            bool active = *reinterpret_cast<const bool *>(rawData);
            AudioPolicyServerPtr->SetDeviceActive(deviceType, active);
            AudioPolicyServerPtr->IsDeviceActive(deviceType);

            AudioRingerMode ringMode = *reinterpret_cast<const AudioRingerMode *>(rawData);
            AudioPolicyServerPtr->SetRingerMode(ringMode);

            int32_t ltonetype = *reinterpret_cast<const int32_t *>(rawData);
            AudioPolicyServerPtr->GetToneConfig(ltonetype);

            AudioScene audioScene = *reinterpret_cast<const AudioScene *>(rawData);
            AudioPolicyServerPtr->SetAudioScene(audioScene);
            AudioPolicyServerPtr->SetMicrophoneMute(mute);

            int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            AudioPolicyServerPtr->SetRingerModeCallback(clientId, object);
            AudioPolicyServerPtr->UnsetRingerModeCallback(clientId);

            DeviceFlag flag = *reinterpret_cast<const DeviceFlag *>(rawData);
            AudioPolicyServerPtr->SetDeviceChangeCallback(clientId, flag, object);
            AudioPolicyServerPtr->UnsetDeviceChangeCallback(clientId);

            uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
            AudioPolicyServerPtr->SetAudioInterruptCallback(sessionID, object);
            AudioPolicyServerPtr->UnsetAudioInterruptCallback(sessionID);
            uint32_t clientID = *reinterpret_cast<const uint32_t *>(rawData);
            AudioPolicyServerPtr->SetAudioManagerInterruptCallback(clientID, object);

            AudioInterrupt audioInterrupt;
            audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
            audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
            audioInterrupt.streamType = STREAM_ACCESSIBILITY;
            AudioPolicyServerPtr->RequestAudioFocus(clientID, audioInterrupt);
            AudioPolicyServerPtr->AbandonAudioFocus(clientID, audioInterrupt);

            int32_t clientPid = *reinterpret_cast<const int32_t *>(rawData);
            AudioPolicyServerPtr->SetVolumeKeyEventCallback(clientPid, object);
            AudioPolicyServerPtr->UnsetVolumeKeyEventCallback(clientPid);
            AudioPolicyServerPtr->OnSessionRemoved(sessionID);

            int32_t clientUID = *reinterpret_cast<const int32_t *>(rawData);
            AudioPolicyServerPtr->RegisterAudioRendererEventListener(clientUID, object);
            AudioPolicyServerPtr->UnregisterAudioRendererEventListener(clientUID);
            AudioPolicyServerPtr->RegisterAudioCapturerEventListener(clientUID, object);
            AudioPolicyServerPtr->UnregisterAudioCapturerEventListener(clientUID);

            AudioPolicyServer::DeathRecipientId id = *reinterpret_cast<const AudioPolicyServer::DeathRecipientId *>(rawData);
            AudioPolicyServerPtr->RegisterClientDeathRecipient(object, id);

            int pid = *reinterpret_cast<const int *>(rawData);
            AudioPolicyServerPtr->RegisteredTrackerClientDied(pid);
            AudioStreamInfo audioStreamInfo = {};
            audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
            audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
            audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
            audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
            AudioPolicyServerPtr->IsAudioRendererLowLatencySupported(audioStreamInfo);

            int32_t clientUid = *reinterpret_cast<const int32_t *>(rawData);
            StreamSetState streamSetState = *reinterpret_cast<const StreamSetState *>(rawData);
            AudioStreamType audioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            AudioPolicyServerPtr->UpdateStreamState(clientUid, streamSetState, audioStreamType);
        }
        void AudioPolicyFuzzTestThree(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            int32_t clientUID = *reinterpret_cast<const int32_t *>(rawData);
            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            bool hasBTPermission = *reinterpret_cast<const bool *>(rawData);
            AudioStreamCollector::GetAudioStreamCollector().RegisterAudioRendererEventListener(clientUID, object, hasBTPermission);
            AudioStreamCollector::GetAudioStreamCollector().UnregisterAudioRendererEventListener(clientUID);
            AudioStreamCollector::GetAudioStreamCollector().RegisterAudioCapturerEventListener(clientUID, object, hasBTPermission);
            AudioStreamCollector::GetAudioStreamCollector().UnregisterAudioCapturerEventListener(clientUID);

            int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
            AudioStreamCollector::GetAudioStreamCollector().RegisteredTrackerClientDied(uid);
            AudioStreamCollector::GetAudioStreamCollector().RegisteredStreamListenerClientDied(uid);

            int32_t clientUid = *reinterpret_cast<const int32_t *>(rawData);
            StreamSetStateEventInternal streamSetStateEventInternal = {};
            streamSetStateEventInternal.streamSetState = *reinterpret_cast<const StreamSetState *>(rawData);
            streamSetStateEventInternal.audioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            AudioStreamCollector::GetAudioStreamCollector().UpdateStreamState(clientUid, streamSetStateEventInternal);

            int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
            float volume = *reinterpret_cast<const float *>(rawData);
            AudioStreamCollector::GetAudioStreamCollector().SetLowPowerVolume(streamId, volume);
            AudioStreamCollector::GetAudioStreamCollector().GetLowPowerVolume(streamId);
            AudioStreamCollector::GetAudioStreamCollector().GetSingleStreamVolume(streamId);
        }

        void AudioPolicyFuzzTestFour(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
            std::shared_ptr<ClientTrackerCallbackListener> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
            StreamSetStateEventInternal streamSetStateEventInternal = {};
            streamSetStateEventInternal.streamSetState = *reinterpret_cast<const StreamSetState *>(rawData);
            streamSetStateEventInternal.audioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            callback->PausedStreamImpl(streamSetStateEventInternal);
            callback->ResumeStreamImpl(streamSetStateEventInternal);
            float volume = *reinterpret_cast<const float *>(rawData);
            callback->SetLowPowerVolumeImpl(volume);
            callback->GetLowPowerVolumeImpl(volume);
            callback->GetSingleStreamVolumeImpl(volume);

            std::shared_ptr<AudioClientTrackerCallbackProxy> audioClientTrackerCallbackProxy = std::make_shared<AudioClientTrackerCallbackProxy>(object);
            audioClientTrackerCallbackProxy->PausedStreamImpl(streamSetStateEventInternal);
            audioClientTrackerCallbackProxy->ResumeStreamImpl(streamSetStateEventInternal);

            audioClientTrackerCallbackProxy->SetLowPowerVolumeImpl(volume);
            audioClientTrackerCallbackProxy->GetLowPowerVolumeImpl(volume);
            audioClientTrackerCallbackProxy->GetSingleStreamVolumeImpl(volume);
        }

        // clent-audio_policy_proxy.h
        void AudioPolicyFuzzTestFive(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);

            AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            float volume = *reinterpret_cast<const float *>(rawData);
            audioPolicyProxy->SetStreamVolume(streamType, volume);
            audioPolicyProxy->GetStreamVolume(streamType);

            int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
            audioPolicyProxy->SetLowPowerVolume(streamId, volume);
            audioPolicyProxy->GetLowPowerVolume(streamId);
            audioPolicyProxy->GetSingleStreamVolume(streamId);

            bool mute = *reinterpret_cast<const bool *>(rawData);
            audioPolicyProxy->SetStreamMute(streamType, mute);
            audioPolicyProxy->GetStreamMute(streamType);
            audioPolicyProxy->IsStreamActive(streamType);

            DeviceFlag deviceFlag = *reinterpret_cast<const DeviceFlag *>(rawData);
            audioPolicyProxy->GetDevices(deviceFlag);

            InternalDeviceType deviceType = *reinterpret_cast<const InternalDeviceType *>(rawData);
            bool active = *reinterpret_cast<const bool *>(rawData);
            audioPolicyProxy->SetDeviceActive(deviceType, active);
            audioPolicyProxy->IsDeviceActive(deviceType);

            int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
            int32_t pid = *reinterpret_cast<const int32_t *>(rawData);
            audioPolicyProxy->GetSelectedDeviceInfo(uid, pid, streamType);

            AudioRingerMode ringMode = *reinterpret_cast<const AudioRingerMode *>(rawData);
            audioPolicyProxy->SetRingerMode(ringMode);

            int32_t ltonetype = *reinterpret_cast<const int32_t *>(rawData);
            audioPolicyProxy->GetToneConfig(ltonetype);

            AudioScene scene = *reinterpret_cast<const AudioScene *>(rawData);
            audioPolicyProxy->SetAudioScene(scene);

            bool isMute = *reinterpret_cast<const bool *>(rawData);
            audioPolicyProxy->SetMicrophoneMute(isMute);

            int32_t clientId = *reinterpret_cast<const int32_t *>(rawData);
            audioPolicyProxy->SetRingerModeCallback(clientId, object);
            audioPolicyProxy->UnsetRingerModeCallback(clientId);
            audioPolicyProxy->SetMicStateChangeCallback(clientId, object);
            audioPolicyProxy->SetDeviceChangeCallback(clientId, deviceFlag, object);
            audioPolicyProxy->UnsetDeviceChangeCallback(clientId);

            uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
            audioPolicyProxy->SetAudioInterruptCallback(sessionID, object);
            audioPolicyProxy->UnsetAudioInterruptCallback(sessionID);

            AudioInterrupt audioInterrupt;
            audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
            audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
            audioInterrupt.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            audioPolicyProxy->ActivateAudioInterrupt(audioInterrupt);
            audioPolicyProxy->DeactivateAudioInterrupt(audioInterrupt);

            uint32_t clientID = *reinterpret_cast<const uint32_t *>(rawData);
            audioPolicyProxy->SetAudioManagerInterruptCallback(clientID, object);
            audioPolicyProxy->UnsetAudioManagerInterruptCallback(clientID);
            audioPolicyProxy->RequestAudioFocus(clientID, audioInterrupt);
            audioPolicyProxy->AbandonAudioFocus(clientID, audioInterrupt);

            uint32_t clientPid = *reinterpret_cast<const uint32_t *>(rawData);
            audioPolicyProxy->SetVolumeKeyEventCallback(clientPid, object);
            audioPolicyProxy->UnsetVolumeKeyEventCallback(clientPid);
            audioPolicyProxy->GetSessionInfoInFocus(audioInterrupt);

            std::string permissionName = *reinterpret_cast<const std::string *>(rawData);
            uint32_t appTokenId = *reinterpret_cast<const uint32_t *>(rawData);
            int32_t appUid = *reinterpret_cast<const int32_t *>(rawData);
            bool privacyFlag = *reinterpret_cast<const bool *>(rawData);
            AudioPermissionState state = *reinterpret_cast<const AudioPermissionState *>(rawData);
            audioPolicyProxy->VerifyClientPermission(permissionName, appTokenId, appUid, privacyFlag, state);
            audioPolicyProxy->getUsingPemissionFromPrivacy(permissionName, appTokenId, state);

            uint32_t count = *reinterpret_cast<const uint32_t *>(rawData);
            DeviceType deviceTypeOne = *reinterpret_cast<const DeviceType *>(rawData);
            audioPolicyProxy->ReconfigureAudioChannel(count, deviceTypeOne);

            int32_t clientUID = *reinterpret_cast<const int32_t *>(rawData);
            audioPolicyProxy->RegisterAudioRendererEventListener(clientUID, object);
            audioPolicyProxy->UnregisterAudioRendererEventListener(clientUID);
            audioPolicyProxy->RegisterAudioCapturerEventListener(clientUID, object);
            audioPolicyProxy->UnregisterAudioCapturerEventListener(clientUID);

            AudioStreamChangeInfo streamChangeInfo = {};
            streamChangeInfo.audioRendererChangeInfo.sessionId = *reinterpret_cast<const int32_t *>(rawData);
            streamChangeInfo.audioRendererChangeInfo.rendererState = *reinterpret_cast<const RendererState *>(rawData);
            streamChangeInfo.audioRendererChangeInfo.clientUID = *reinterpret_cast<const int32_t *>(rawData);
            streamChangeInfo.audioRendererChangeInfo.rendererInfo.contentType = *reinterpret_cast<const ContentType *>(rawData);
            streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
            streamChangeInfo.audioRendererChangeInfo.rendererInfo.rendererFlags = *reinterpret_cast<const int32_t *>(rawData);
            AudioMode mode = *reinterpret_cast<const AudioMode *>(rawData);
            audioPolicyProxy->RegisterTracker(mode, streamChangeInfo, object);
            audioPolicyProxy->UpdateTracker(mode, streamChangeInfo);

            StreamSetState streamSetState = *reinterpret_cast<const StreamSetState *>(rawData);
            AudioStreamType audioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
            audioPolicyProxy->UpdateStreamState(clientUID, streamSetState, audioStreamType);

            AudioStreamInfo audioStreamInfo = {};
            audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
            audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
            audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
            audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
            audioPolicyProxy->IsAudioRendererLowLatencySupported(audioStreamInfo);
        }
    } // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyFuzzTestOne(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestTwoInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyFuzzTestTwo(data, size);
    return 0;
}

extern "C" int LLVMFuzzerTestThreeInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyFuzzTestThree(data, size);
    return 0;
}
extern "C" int LLVMFuzzerTestFourInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyFuzzTestFour(data, size);
    return 0;
}
extern "C" int LLVMFuzzerTestFiveInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyFuzzTestFive(data, size);
    return 0;
}
