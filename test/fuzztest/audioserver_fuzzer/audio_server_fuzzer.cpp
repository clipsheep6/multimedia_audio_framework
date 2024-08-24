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
#include "audio_process_in_client.h"
#include "audio_param_parser.h"
#include "audio_process_config.h"
#include "ipc_stream_in_server.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t OFFSET = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_MIN = 0;
const uint32_t LIMIT_MAX = static_cast<uint32_t>(AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX);
const uint32_t COMMON_SIZE = 2;
const uint32_t IOPERTAION_LENGTH = 13;
const uint32_t APPID_LENGTH = 10;

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

    std::shared_ptr<AudioServer> AudioServerPtr =
        std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK)) {
        sptr<AudioPolicyManagerListenerStub> focusListenerStub = new(std::nothrow) AudioPolicyManagerListenerStub();
        sptr<IRemoteObject> object = focusListenerStub->AsObject();
        AudioServerPtr->SetParameterCallback(object);
        return;
    }
    AudioServerPtr->OnRemoteRequest(code, data, reply, option);

    if (size < LIMITSIZE) {
        return;
    }
    std::string netWorkId(reinterpret_cast<const char*>(rawData), size - 1);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey *>(rawData);
    std::string condition(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value(reinterpret_cast<const char*>(rawData), size - 1);
    AudioServerPtr->OnAudioSinkParamChange(netWorkId, key, condition, value);
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_CAPTURE_SILENT_STATE),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE),
        data, reply, option);
}

void AudioServerResetAudioEndpointTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_AUDIO_ENDPOINT),
        data, reply, option);
}

void AudioServerCreatePlaybackCapturerManagerTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
   
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::REQUEST_THREAD_PRIORITY),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE),
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

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE),
        data, reply, option);
}

void AudioServerSetSpatializationSceneTypeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE),
        data, reply, option);
}

void AudioServerUpdateSpatialDeviceTypeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE),
        data, reply, option);
}

void AudioServerSetCaptureSilentStateTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isSilent = *reinterpret_cast<const bool*>(rawData);
    data.WriteBool(isSilent);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_CAPTURE_SILENT_STATE),
        data, reply, option);
}

void AudioServerLoadConfigurationTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys = {};
    std::unordered_map<std::string, std::set<std::string>> audioParameterKey = {};
    std::set<std::string> audioParameterValue = {};
    std::string audioParameterKeyStr_1(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyStr_2(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyValueStr_1(reinterpret_cast<const char*>(rawData), size - 1);
    std::string audioParameterKeyValueStr_2(reinterpret_cast<const char*>(rawData), size - 1);
    audioParameterValue.insert(audioParameterKeyValueStr_1);
    audioParameterValue.insert(audioParameterKeyValueStr_2);
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr_1, audioParameterValue));
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr_2, audioParameterValue));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr_1, audioParameterKey));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr_2, audioParameterKey));

    std::shared_ptr<AudioParamParser> audioParamParser = std::make_shared<AudioParamParser>();
    audioParamParser->LoadConfiguration(audioParameterKeys);
}

void AudioServerGetExtarAudioParametersTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string mainKey(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value_1(reinterpret_cast<const char*>(rawData), size - 1);
    std::string value_2(reinterpret_cast<const char*>(rawData), size - 1);
    std::vector<std::string> subkeys = {};
    std::vector<std::pair<std::string, std::string>> result;
    subkeys.push_back(value_1);
    subkeys.push_back(value_2);

    data.WriteString(static_cast<std::string>(mainKey));
    data.WriteInt32(subkeys.size());
    for (std::string subKey : subkeys) {
        data.WriteString(static_cast<std::string>(subKey));
    }

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS),
        data, reply, option);
}

void AudioServerSetExtraAudioParametersTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string mainKey(reinterpret_cast<const char*>(rawData), size - 1);
    std::vector<std::pair<std::string, std::string>> kvpairs;
    for (uint32_t i = 0; i < COMMON_SIZE; i++) {
        std::string subKey(reinterpret_cast<const char*>(rawData), size - 1);
        std::string subValue(reinterpret_cast<const char*>(rawData), size - 1);
        kvpairs.push_back(std::make_pair(subKey, subValue));
    }

    data.WriteString(mainKey);
    data.WriteInt32(static_cast<int32_t>(kvpairs.size()));
    for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
        data.WriteString(static_cast<std::string>(it->first));
        data.WriteString(static_cast<std::string>(it->second));
    }

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS),
        data, reply, option);
}

void AudioServerUpdateRouteReqTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    DeviceType type = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceFlag flag = *reinterpret_cast<const DeviceFlag*>(rawData);
    BluetoothOffloadState a2dpOffloadFlag = *reinterpret_cast<const BluetoothOffloadState*>(rawData);
    data.WriteInt32(type);
    data.WriteInt32(flag);
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTE_REQ),
        data, reply, option);
}

void AudioServerUpdateActiveDevicesRouteTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    for (int i = 0; i < COMMON_SIZE; i++) {
        DeviceType deviceType = *reinterpret_cast<const DeviceType*>(rawData);
        DeviceFlag deviceFlag = *reinterpret_cast<const DeviceFlag*>(rawData);
        activeDevices.push_back(std::make_pair(deviceType, deviceFlag));
    }
    data.WriteInt32(static_cast<int32_t>(activeDevices.size()));
    for (auto it = activeDevices.begin(); it != activeDevices.end(); it++) {
        data.WriteInt32(static_cast<int32_t>(it->first));
        data.WriteInt32(static_cast<int32_t>(it->second));
    }

    BluetoothOffloadState a2dpOffloadFlag = *reinterpret_cast<const BluetoothOffloadState*>(rawData);
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ),
        data, reply, option);
}

void AudioServerUpdateDualToneStateTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool enable = *reinterpret_cast<const bool*>(rawData);
    int32_t sessionId = *reinterpret_cast<const int32_t*>(rawData);

    data.WriteBool(enable);
    data.WriteInt32(sessionId);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ),
        data, reply, option);
}

void AudioServerGetTransactionIdTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    DeviceType deviceType = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceRole deviceRole = *reinterpret_cast<const DeviceRole*>(rawData);
    data.WriteInt32(deviceType);
    data.WriteInt32(deviceRole);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_TRANSACTION_ID),
        data, reply, option);
}

void AudioGetAudioParameterTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioParamKey)) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    std::string networkId(reinterpret_cast<const char*>(rawData), size);
    std::string condition(reinterpret_cast<const char*>(rawData), size);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(key));
    data.WriteString(condition);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioSetAudioParameterTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioParamKey)) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    std::string networkId(reinterpret_cast<const char*>(rawData), size);
    std::string condition(reinterpret_cast<const char*>(rawData), size);
    std::string value(reinterpret_cast<const char*>(rawData), size);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<uint32_t>(key));
    data.WriteString(condition);
    data.WriteString(value);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioCreateAudioProcessTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;

    ProcessConfig::WriteConfigToParcel(config, data);

    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    audioServer->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOPROCESS),
        data, reply, option);
}

void AudioLoadAudioEffectLibrariesTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    int32_t countLib = 2;
    int32_t countEff = 2;
    data.WriteInt32(countLib);
    data.WriteInt32(countEff);
    for (int32_t i = 0; i < countLib; i++) {
        std::string libName(reinterpret_cast<const char*>(rawData), size - 1);
        std::string libPath(reinterpret_cast<const char*>(rawData), size - 1);
        data.WriteString(libName);
        data.WriteString(libPath);
    }

    for (int32_t i = 0; i < countEff; i++) {
        std::string effectName(reinterpret_cast<const char*>(rawData), size - 1);
        std::string libName(reinterpret_cast<const char*>(rawData), size - 1);
        data.WriteString(effectName);
        data.WriteString(libName);
    }

    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    audioServer->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES),
        data, reply, option);
}

void AudioRendererInServerTestFirst(const uint8_t* rawData, size_t size, std::shared_ptr<RendererInServer> renderer)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t operation_int = *reinterpret_cast<const uint32_t*>(rawData);
    operation_int = (operation_int%IOPERTAION_LENGTH) - 1;
    IOperation operation = static_cast<IOperation>(operation_int);
    renderer->OnStatusUpdate(operation);
    renderer->HandleOperationFlushed();
    size_t length = *reinterpret_cast<const size_t*>(rawData);
    renderer->OnWriteData(length);
    std::shared_ptr<OHAudioBuffer> buffer = nullptr;
    renderer->ResolveBuffer(buffer);
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    renderer->GetSessionId(sessionId);
    uint64_t framePos = *reinterpret_cast<const uint64_t*>(rawData);
    uint64_t timeStamp = *reinterpret_cast<const uint64_t*>(rawData);
    renderer->GetAudioTime(framePos, timeStamp);
    renderer->GetAudioPosition(framePos, timeStamp);
    uint64_t latency = *reinterpret_cast<const uint64_t*>(rawData);
    renderer->GetLatency(latency);
    int32_t rate = *reinterpret_cast<const int32_t*>(rawData);
    renderer->SetRate(rate);
    float volume = *reinterpret_cast<const float*>(rawData);
    renderer->SetLowPowerVolume(volume);
    renderer->GetLowPowerVolume(volume);
    int32_t effectMode = *reinterpret_cast<const int32_t*>(rawData);
    renderer->SetAudioEffectMode(effectMode);
    renderer->GetAudioEffectMode(effectMode);
    int32_t privacyType = *reinterpret_cast<const int32_t*>(rawData);
    renderer->SetPrivacyType(privacyType);
    renderer->GetPrivacyType(privacyType);
    int32_t state = *reinterpret_cast<const int32_t*>(rawData);
    bool isAppBack = *reinterpret_cast<const bool*>(rawData);
    renderer->SetOffloadMode(state, isAppBack);
    renderer->UnsetOffloadMode();
    renderer->OffloadSetVolume(volume);
}

void AudioRendererInServerTestSecond(const uint8_t* rawData, size_t size, std::shared_ptr<RendererInServer> renderer)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint32_t)) {
        return;
    }

    bool isAppBack = *reinterpret_cast<const bool*>(rawData);
    bool headTrackingEnabled = *reinterpret_cast<const bool*>(rawData);
    renderer->UpdateSpatializationState(isAppBack, headTrackingEnabled);
    renderer->WriterRenderStreamStandbySysEvent();
    uint64_t timeStamp = *reinterpret_cast<const uint64_t*>(rawData);
    renderer->GetOffloadApproximatelyCacheTime(timeStamp, timeStamp, timeStamp, timeStamp);
    size_t length = *reinterpret_cast<const size_t*>(rawData);
    renderer->DequeueBuffer(length);
    BufferDesc desc;
    desc.buffer = nullptr;
    desc.bufLength = 0;
    desc.dataLength =0;
    renderer->VolumeHandle(desc);
    renderer->WriteData();
    renderer->WriteEmptyData();
    renderer->DrainAudioBuffer();
    renderer->EnableInnerCap();
    renderer->DisableInnerCap();
    renderer->InitDupStream();
    renderer->EnableDualTone();
    renderer->DisableDualTone();
    renderer->InitDualToneStream();
    renderer->GetStreamManagerType();
    renderer->SetSilentModeAndMixWithOthers(isAppBack);
    renderer->SetClientVolume();
    uint32_t operation_int = *reinterpret_cast<const uint32_t*>(rawData);
    operation_int = (operation_int%IOPERTAION_LENGTH) - 1;
    IOperation operation = static_cast<IOperation>(operation_int);
    renderer->OnDataLinkConnectionUpdate(operation);
    std::string dumpString(reinterpret_cast<const char*>(rawData), size-1);
    renderer->Dump(dumpString);
    renderer->Pause();
    renderer->Flush();
    renderer->Drain(headTrackingEnabled);
    renderer->Stop();
    renderer->Release();
}

void AudioRendererInServerTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStreamInServer = IpcStreamInServer::Create(config, ret);
    if (ipcStreamInServer == nullptr) {
        return;
    }
    std::shared_ptr<RendererInServer> renderer = ipcStreamInServer->GetRenderer();
    renderer->Init();
    renderer->Start();
    renderer->ConfigServerBuffer();
    renderer->InitBufferStatus();
    renderer->UpdateWriteIndex();
    
    AudioRendererInServerTestFirst(rawData, size, renderer);
    AudioRendererInServerTestSecond(rawData, size, renderer);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerCaptureSilentlyFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerOffloadSetVolumeFuzzTest(data, size);
    OHOS::AudioStandard::AudioServerNotifyStreamVolumeChangedFuzzTest(data, size);
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
    OHOS::AudioStandard::AudioServerUpdateLatencyTimestampTest(data, size);
    OHOS::AudioStandard::AudioServerSetSpatializationSceneTypeTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateSpatialDeviceTypeTest(data, size);
    OHOS::AudioStandard::AudioServerSetCaptureSilentStateTest(data, size);
    OHOS::AudioStandard::AudioServerLoadConfigurationTest(data, size);
    OHOS::AudioStandard::AudioServerGetExtarAudioParametersTest(data, size);
    OHOS::AudioStandard::AudioServerSetExtraAudioParametersTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateRouteReqTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateActiveDevicesRouteTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateDualToneStateTest(data, size);
    OHOS::AudioStandard::AudioServerGetTransactionIdTest(data, size);
    OHOS::AudioStandard::AudioSetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioGetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioCreateAudioProcessTest(data, size);
    OHOS::AudioStandard::AudioLoadAudioEffectLibrariesTest(data, size);
    OHOS::AudioStandard::AudioRendererInServerTest(data, size);
    return 0;
}
