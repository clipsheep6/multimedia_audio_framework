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

#include "i_audio_renderer_sink.h"
#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "message_parcel.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasServerInit = false;
constexpr int32_t OFFSET = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_MIN = 0;
const uint32_t LIMIT_MAX = 30;

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

uint32_t Convert2Uint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    /* Move the 0th digit to the left by 24 bits, the 1st digit to the left by 16 bits,
       the 2nd digit to the left by 8 bits, and the 3rd digit not to the left */
    return (ptr[0] << SHIFT_LEFT_24) | (ptr[1] << SHIFT_LEFT_16) | (ptr[2] << SHIFT_LEFT_8) | (ptr[3]);
}

void AudioServerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code =  Convert2Uint32(rawData) % (LIMIT_MAX - LIMIT_MIN + 1) + LIMIT_MIN;
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK)) {
        sptr<AudioPolicyManagerListenerStub> focusListenerStub = new(std::nothrow) AudioPolicyManagerListenerStub();
        sptr<IRemoteObject> object = focusListenerStub->AsObject();
        GetServerPtr()->SetParameterCallback(object);
        return;
    }
    GetServerPtr()->OnRemoteRequest(code, data, reply, option);

    if (size < LIMITSIZE) {
        return;
    }
    std::string netWorkId(reinterpret_cast<const char*>(rawData), size - 1);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey *>(rawData);
    std::string condition(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    GetServerPtr()->OnAudioSinkParamChange(netWorkId, key, condition, value);
}

void AudioServerCaptureSilentlyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_CAPTURE_SILENT_STATE),
        data, reply, option);
}

float Convert2Float(const uint8_t *ptr)
{
    float floatValue = static_cast<float>(*ptr);
    return floatValue / 128.0f - 1.0f;
}

void AudioServerOffloadSetVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float volume = Convert2Float(rawData);
    data.WriteFloat(volume);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME),
        data, reply, option);
}

void AudioServerOffloadDrainFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_DRAIN),
        data, reply, option);
}

void AudioServerOffloadGetPresentationPositionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_GET_PRESENTATION_POSITION),
        data, reply, option);
}

void AudioServerOffloadSetBufferSizeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_BUFFER_SIZE),
        data, reply, option);
}

void AudioServerNotifyStreamVolumeChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED),
        data, reply, option);
}

void AudioServerGetCapturePresentationPositionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string deviceClass(reinterpret_cast<const char*>(rawData), size);
    data.WriteString(deviceClass);

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_CAPTURE_PRESENTATION_POSITION),
        data, reply, option);
}

void AudioServerGetRenderPresentationPositionFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string deviceClass(reinterpret_cast<const char*>(rawData), size);
    data.WriteString(deviceClass);

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_RENDER_PRESENTATION_POSITION),
        data, reply, option);
}

void AudioServerResetRouteForDisconnectFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteInt32(deviceType);
    MessageParcel reply;
    MessageOption option;

    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT),
        data, reply, option);
}

void AudioServerGetEffectLatencyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string sessionId(reinterpret_cast<const char*>(rawData), size);
    data.WriteString(sessionId);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY),
        data, reply, option);
}

void AudioServerUpdateLatencyTimestampTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string timestamp(reinterpret_cast<const char*>(rawData), size - 1);
    bool isRenderer = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(timestamp);
    data.WriteBool(isRenderer);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP),
        data, reply, option);
}

void AudioServerGetMaxAmplitudeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isOutputDevice = *reinterpret_cast<const bool*>(rawData);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    data.WriteBool(isOutputDevice);
    data.WriteInt32(deviceType);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE),
        data, reply, option);
}

void AudioServerResetAudioEndpointTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_AUDIO_ENDPOINT),
        data, reply, option);
}

void AudioServerCreatePlaybackCapturerManagerTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
   
    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER),
        data, reply, option);
}

void AudioServerSetOutputDeviceSinkTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t deviceType = *reinterpret_cast<const int32_t*>(rawData);
    std::string sinkName(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteInt32(deviceType);
    data.WriteString(sinkName);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK),
        data, reply, option);
}

void AudioServerRequestThreadPriorityTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t tid = *reinterpret_cast<const uint32_t*>(rawData);
    std::string bundleName(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteUint32(tid);
    data.WriteString(bundleName);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::REQUEST_THREAD_PRIORITY),
        data, reply, option);
}

void AudioServerSetAudioMonoStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool audioMono = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(audioMono);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE),
        data, reply, option);
}

void AudioServerSetVoiceVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float volume = *reinterpret_cast<const float*>(rawData);
    data.WriteFloat(volume);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME),
        data, reply, option);
}

void AudioServerCheckRemoteDeviceStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string networkId(reinterpret_cast<const char*>(rawData), size - 1);
    DeviceRole deviceRole = *reinterpret_cast<const DeviceRole*>(rawData);
    bool isStartDevice = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(deviceRole));
    data.WriteBool(isStartDevice);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE),
        data, reply, option);
}

void AudioServerNotifyDeviceInfoTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string networkId(reinterpret_cast<const char*>(rawData), size - 1);
    bool connected = *reinterpret_cast<const bool*>(rawData);
    data.WriteString(networkId);
    data.WriteBool(connected);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO),
        data, reply, option);
}

void AudioServerGetAudioParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string key(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteString(key);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetAudioParameterTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string key(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    data.WriteString(key);
    data.WriteString(value);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetMicrophoneMuteTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isMute = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(isMute);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE),
        data, reply, option);
}

void AudioServerSetAudioBalanceValueTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    float audioBalance = *reinterpret_cast<const float*>(rawData);
    data.WriteFloat(audioBalance);

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE),
        data, reply, option);
}

void AudioServerSetAudioSceneTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    AudioScene audioScene = *reinterpret_cast<const AudioScene*>(rawData);
    DeviceType outputDevice = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceType inputDevice = *reinterpret_cast<const DeviceType*>(rawData);
    data.WriteInt32(static_cast<int32_t>(audioScene));
    data.WriteInt32(static_cast<int32_t>(outputDevice));
    data.WriteInt32(static_cast<int32_t>(inputDevice));

    MessageParcel reply;
    MessageOption option;
    GetServerPtr()->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE),
        data, reply, option);
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
    OHOS::AudioStandard::AudioServerFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerCaptureSilentlyFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerOffloadSetVolumeFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerOffloadDrainFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerOffloadGetPresentationPositionFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerOffloadSetBufferSizeFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerNotifyStreamVolumeChangedFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerGetCapturePresentationPositionFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerGetRenderPresentationPositionFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerResetRouteForDisconnectFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerGetEffectLatencyTest(data, size);
    OHOS::AudioStandard::AudioServerGetMaxAmplitudeTest(data, size);
    OHOS::AudioStandard::AudioServerResetAudioEndpointTest(data, size);
    OHOS::AudioStandard::AudioServerCreatePlaybackCapturerManagerTest(data, size);
    OHOS::AudioStandard::AudioServerSetOutputDeviceSinkTest(data, size);
    OHOS::AudioStandard::AudioServerRequestThreadPriorityTest(data, size);
    OHOS::AudioStandard::AudioServerSetAudioMonoStateTest(data, size);
    OHOS::AudioStandard::AudioServerSetVoiceVolumeTest(data, size);
    OHOS::AudioStandard::AudioServerCheckRemoteDeviceStateTest(data, size);
    OHOS::AudioStandard::AudioServerNotifyDeviceInfoTest(data, size);
    OHOS::AudioStandard::AudioServerGetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioServerSetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioServerSetMicrophoneMuteTest(data, size);
    OHOS::AudioStandard::AudioServerSetAudioBalanceValueTest(data, size);
    OHOS::AudioStandard::AudioServerSetAudioSceneTest(data, size);
    return 0;
}
