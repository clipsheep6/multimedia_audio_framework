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
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_manager_listener_stub.h"
#include "message_parcel.h"
#include "audio_policy_client.h"
#include "audio_socket_thread.h"
#include "audio_pnp_server.h"
#include "audio_input_thread.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasServerInit = false;
bool g_hasPnpServerInit = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;

static AudioRendererInfo getAudioRenderInfo(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return {};
    }
    ContentType contentType = *reinterpret_cast<const ContentType *>(rawData);
    StreamUsage streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    int32_t rendererFlags = *reinterpret_cast<const int32_t *>(rawData);
    std::string sceneType(reinterpret_cast<const char*>(rawData), size - 1);
    bool spatializationEnabled = *reinterpret_cast<const bool *>(rawData);
    bool headTrackingEnabled = *reinterpret_cast<const bool *>(rawData);
    int32_t originalFlag = *reinterpret_cast<const int32_t *>(rawData);
    AudioRendererInfo rendererInfo = {
        contentType,
        streamUsage,
        rendererFlags,
        sceneType,
        spatializationEnabled,
        headTrackingEnabled,
        originalFlag
    };
    return rendererInfo;
}

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        g_hasServerInit = true;
    }
    return &server;
}

AudioPnpServer* GetPnpServerPtr()
{
    static AudioPnpServer pnpServer;
    if (!g_hasPnpServerInit) {
        pnpServer.init();
        g_hasPnpServerInit = true;
    }
    return &pnpServer;
}

void AudioVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioStreamType streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    VolumeAdjustType adjustType = *reinterpret_cast<const VolumeAdjustType *>(rawData);
    int32_t volume = *reinterpret_cast<const int32_t *>(rawData);
    int32_t streamId = *reinterpret_cast<const int32_t *>(rawData);
    DeviceType deviceType = *reinterpret_cast<const DeviceType *>(rawData);
    int32_t uid = *reinterpret_cast<const int32_t *>(rawData);
    int32_t pid = *reinterpret_cast<const int32_t *>(rawData);

    bool mute = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetSystemVolumeLevel(streamType, volume);
    GetServerPtr()->GetSystemVolumeLevel(streamType);
    GetServerPtr()->SetLowPowerVolume(streamId, volume);
    GetServerPtr()->GetLowPowerVolume(streamId);
    GetServerPtr()->GetSingleStreamVolume(streamId);
    GetServerPtr()->SetStreamMute(streamType, mute);
    GetServerPtr()->GetStreamMute(streamType);
    GetServerPtr()->IsStreamActive(streamType);
    GetServerPtr()->GetMaxVolumeLevel(streamType);
    GetServerPtr()->GetMinVolumeLevel(streamType);
    GetServerPtr()->SetSystemVolumeLevelLegacy(streamType, volume);
    GetServerPtr()->IsVolumeUnadjustable();
    GetServerPtr()->AdjustVolumeByStep(adjustType);
    GetServerPtr()->AdjustSystemVolumeByStep(streamType, adjustType);
    GetServerPtr()->GetSystemVolumeInDb(streamType, volume, deviceType);
    GetServerPtr()->GetSelectedDeviceInfo(uid, pid, streamType);

    AudioRendererInfo rendererInfo = getAudioRenderInfo(rawData, size);
    GetServerPtr()->GetPreferredOutputStreamType(rendererInfo);

    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    AudioCapturerInfo capturerInfo = {
        sourceType,
        capturerFlags
    };
    GetServerPtr()->GetPreferredInputStreamType(capturerInfo);
}

void AudioDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    DeviceFlag flag = *reinterpret_cast<const DeviceFlag *>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    SourceType sourceType = *reinterpret_cast<const SourceType *>(rawData);
    int32_t capturerFlags = *reinterpret_cast<const int32_t *>(rawData);
    AudioCapturerInfo capturerInfo = {
        sourceType,
        capturerFlags
    };
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
    audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);

    InternalDeviceType deviceType = *reinterpret_cast<const InternalDeviceType *>(rawData);

    uint32_t sessionId = *reinterpret_cast<const uint32_t *>(rawData);
    bool active = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetDeviceActive(deviceType, active);
    GetServerPtr()->IsDeviceActive(deviceType);
    GetServerPtr()->NotifyCapturerAdded(capturerInfo, audioStreamInfo, sessionId);
    GetServerPtr()->GetDevices(flag);
    GetServerPtr()->GetDevicesInner(flag);
    AudioRingerMode ringMode = *reinterpret_cast<const AudioRingerMode *>(rawData);
    GetServerPtr()->SetRingerMode(ringMode);

#ifdef FEATURE_DTMF_TONE
    int32_t ltonetype = *reinterpret_cast<const int32_t *>(rawData);
    GetServerPtr()->GetToneConfig(ltonetype);
#endif

    AudioScene audioScene = *reinterpret_cast<const AudioScene *>(rawData);
    GetServerPtr()->SetAudioScene(audioScene);

    bool mute = *reinterpret_cast<const bool *>(rawData);
    bool legacy = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetMicrophoneMute(mute);
    GetServerPtr()->SetMicrophoneMuteCommon(mute, legacy);
    GetServerPtr()->SetMicrophoneMuteAudioConfig(mute);
    GetServerPtr()->SetMicrophoneMutePersistent(mute, PolicyType::PRIVACY_POLCIY_TYPE);
    GetServerPtr()->GetPersistentMicMuteState();
    GetServerPtr()->IsMicrophoneMuteLegacy();
    GetServerPtr()->GetAudioScene();

    const sptr<AudioStandard::AudioDeviceDescriptor> deviceDescriptor = new AudioStandard::AudioDeviceDescriptor();
    CastType type = *reinterpret_cast<const CastType *>(rawData);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->ConfigDistributedRoutingRole(deviceDescriptor, type);
    GetServerPtr()->SetDistributedRoutingRoleCallback(object);
    GetServerPtr()->UnsetDistributedRoutingRoleCallback();
    GetServerPtr()->SetAudioDeviceRefinerCallback(object);
    GetServerPtr()->UnsetAudioDeviceRefinerCallback();
    GetServerPtr()->TriggerFetchDevice();
}

void AudioInterruptFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
    GetServerPtr()->SetAudioInterruptCallback(sessionID, object);
    GetServerPtr()->UnsetAudioInterruptCallback(sessionID);

    int32_t clientId = *reinterpret_cast<const uint32_t *>(rawData);
    GetServerPtr()->SetAudioManagerInterruptCallback(clientId, object);
    GetServerPtr()->UnsetAudioManagerInterruptCallback(clientId);

    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = *reinterpret_cast<const ContentType *>(rawData);
    audioInterrupt.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    audioInterrupt.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    GetServerPtr()->RequestAudioFocus(clientId, audioInterrupt);
    GetServerPtr()->AbandonAudioFocus(clientId, audioInterrupt);
    GetServerPtr()->ActivateAudioInterrupt(audioInterrupt);
    GetServerPtr()->DeactivateAudioInterrupt(audioInterrupt);
    GetServerPtr()->GetStreamInFocus();
    GetServerPtr()->GetSessionInfoInFocus(audioInterrupt);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    focusInfo.first.streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    focusInfo.first.contentType = *reinterpret_cast<const ContentType *>(rawData);
    focusInfo.first.audioFocusType.streamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    focusInfo.first.audioFocusType.sourceType = *reinterpret_cast<const SourceType *>(rawData);
    focusInfo.first.audioFocusType.isPlay = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.sessionId = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.pauseWhenDucked = *reinterpret_cast<const bool *>(rawData);
    focusInfo.first.pid = *reinterpret_cast<const int32_t *>(rawData);
    focusInfo.first.mode = *reinterpret_cast<const InterruptMode *>(rawData);
    focusInfo.second = *reinterpret_cast<const AudioFocuState *>(rawData);
    focusInfoList.push_back(focusInfo);
    GetServerPtr()->GetAudioFocusInfoList(focusInfoList);
}

void AudioPolicyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->RegisterPolicyCallbackClient(object);

    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
    SessionEvent sessionEvent = *reinterpret_cast<const SessionEvent *>(rawData);
    sessionEvent.type = *reinterpret_cast<const SessionEvent::Type *>(rawData);
    sessionEvent.sessionID = sessionID;
    sessionEvent.sessionInfo_ = *reinterpret_cast<const SessionInfo *>(rawData);

    GetServerPtr()->OnAudioStreamRemoved(sessionID);
    GetServerPtr()->ProcessSessionRemoved(sessionID);
    GetServerPtr()->ProcessSessionAdded(sessionEvent);
    GetServerPtr()->ProcessorCloseWakeupSource(sessionID);
    GetServerPtr()->ProcessorCloseWakeupSource(sessionID);
    GetServerPtr()->UnregisterSpatializationStateEventListener(sessionID);

    AudioPolicyServer::DeathRecipientId id =
        *reinterpret_cast<const AudioPolicyServer::DeathRecipientId *>(rawData);
    GetServerPtr()->RegisterClientDeathRecipient(object, id);
}

void AudioPolicyOtherFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int pid = *reinterpret_cast<const int *>(rawData);
    GetServerPtr()->RegisteredTrackerClientDied(pid);

    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);
    audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
    GetServerPtr()->IsAudioRendererLowLatencySupported(audioStreamInfo);

    int32_t clientUid = *reinterpret_cast<const int32_t *>(rawData);
    StreamSetState streamSetState = *reinterpret_cast<const StreamSetState *>(rawData);
    StreamUsage streamUsage = *reinterpret_cast<const StreamUsage *>(rawData);
    GetServerPtr()->UpdateStreamState(clientUid, streamSetState, streamUsage);
    
    int32_t sessionId = *reinterpret_cast<const int32_t *>(rawData);
    GetServerPtr()->GetAudioCapturerMicrophoneDescriptors(sessionId);

    AudioPipeType type = *reinterpret_cast<const AudioPipeType *>(rawData);
    GetServerPtr()->MoveToNewPipe(sessionId, type);

    sptr<AudioStandard::AudioDeviceDescriptor> deviceDescriptor = new AudioStandard::AudioDeviceDescriptor();
    deviceDescriptor->deviceType_ = *reinterpret_cast<const DeviceType *>(rawData);
    deviceDescriptor->deviceRole_ = *reinterpret_cast<const DeviceRole *>(rawData);
    GetServerPtr()->GetHardwareOutputSamplingRate(deviceDescriptor);

    GetServerPtr()->GetAvailableMicrophones();

    std::string macAddress(reinterpret_cast<const char*>(rawData), size - 1);
    bool support = *reinterpret_cast<const bool *>(rawData);
    int32_t volume = *reinterpret_cast<const int32_t *>(rawData);
    bool updateUi = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetDeviceAbsVolumeSupported(macAddress, support);
    GetServerPtr()->SetA2dpDeviceVolume(macAddress, volume, updateUi);

    bool state = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetCaptureSilentState(state);

    GetServerPtr()->IsHighResolutionExist();
    bool highResExist = *reinterpret_cast<const bool *>(rawData);
    GetServerPtr()->SetHighResolutionExist(highResExist);

    std::string networkId(reinterpret_cast<const char*>(rawData), size - 1);
    InterruptEvent event = {};
    event.eventType = *reinterpret_cast<const InterruptType *>(rawData);
    event.forceType = *reinterpret_cast<const InterruptForceType *>(rawData);
    event.hintType = *reinterpret_cast<const InterruptHint *>(rawData);
    GetServerPtr()->InjectInterruption(networkId, event);
}

void AudioConcurrencyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
    GetServerPtr()->SetAudioConcurrencyCallback(sessionID, object);
    GetServerPtr()->UnsetAudioConcurrencyCallback(sessionID);
    AudioPipeType pipeType = *reinterpret_cast<const AudioPipeType *>(rawData);
    GetServerPtr()->ActivateAudioConcurrency(pipeType);
    GetServerPtr()->ResetRingerModeMute();
}

void AudioVolumeKeyCallbackStub(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    sptr<AudioPolicyClientStub> listener =
        static_cast<sptr<AudioPolicyClientStub>>(new(std::nothrow) AudioPolicyClientStubImpl());
    VolumeEvent volumeEvent = {};
    volumeEvent.volumeType =  *reinterpret_cast<const AudioStreamType *>(rawData);
    volumeEvent.volume = *reinterpret_cast<const int32_t *>(rawData);
    volumeEvent.updateUi = *reinterpret_cast<const bool *>(rawData);
    volumeEvent.volumeGroupId = *reinterpret_cast<const int32_t *>(rawData);
    std::string id(reinterpret_cast<const char*>(rawData), size - 1);
    volumeEvent.networkId = id;

    MessageParcel data;
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT));
    data.WriteInt32(static_cast<int32_t>(volumeEvent.volumeType));
    data.WriteInt32(volumeEvent.volume);
    data.WriteBool(volumeEvent.updateUi);
    data.WriteInt32(volumeEvent.volumeGroupId);
    data.WriteString(volumeEvent.networkId);
    MessageParcel reply;
    MessageOption option;
    listener->OnRemoteRequest(static_cast<uint32_t>(UPDATE_CALLBACK_CLIENT), data, reply, option);
}

void AudioPolicyManagerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioEvent audioEvent;
    uint32_t eventType = *reinterpret_cast<const uint32_t *>(rawData);
    uint32_t deviceType = *reinterpret_cast<const uint32_t *>(rawData);
    std::string name = std::string(reinterpret_cast<const char*>(rawData), size - 1);
    std::string address = std::string(reinterpret_cast<const char*>(rawData), size - 1);
    audioEvent.eventType = eventType;
    audioEvent.deviceType = deviceType;
    audioEvent.name = name;
    audioEvent.address = address;
    int fd = reinterpret_cast<int>(rawData);
    int sockfd = reinterpret_cast<int>(rawData);
    size_t length = reinterpret_cast<size_t>(rawData);
    const ssize_t strLength = reinterpret_cast<ssize_t>(rawData);
    const char *msg = reinterpret_cast<const char *>(rawData);
    char* buffer = const_cast<char*>(reinterpret_cast<const char*>(rawData));
    AudioSocketThread::IsUpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::UpdatePnpDeviceState(&audioEvent);
    AudioSocketThread::AudioPnpUeventOpen(&fd);
    AudioSocketThread::UpdateDeviceState(audioEvent);
    AudioSocketThread::DetectUsbHeadsetState(&audioEvent);
    AudioSocketThread::DetectAnalogHeadsetState(&audioEvent);
    AudioSocketThread::AudioPnpUeventParse(msg, strLength);
    AudioSocketThread::AudioPnpReadUeventMsg(sockfd, buffer, length);
    AudioInputThread::AudioPnpInputOpen();
    AudioInputThread::AudioPnpInputPollAndRead();

    std::string info = std::string(reinterpret_cast<const char*>(rawData), size - 1);
    GetPnpServerPtr()->GetAudioPnpServer();
    GetPnpServerPtr()->UnRegisterPnpStatusListener();
    GetPnpServerPtr()->OnPnpDeviceStatusChanged(info);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::GetServerPtr();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioVolumeFuzzTest(data, size);
    OHOS::AudioStandard::AudioDeviceFuzzTest(data, size);
    OHOS::AudioStandard::AudioInterruptFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyOtherFuzzTest(data, size);
    OHOS::AudioStandard::AudioVolumeKeyCallbackStub(data, size);
    OHOS::AudioStandard::AudioConcurrencyFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyManagerFuzzTest(data, size);
    OHOS::AudioStandard::AudioPolicyFuzzTest(data, size);
    return 0;
}

