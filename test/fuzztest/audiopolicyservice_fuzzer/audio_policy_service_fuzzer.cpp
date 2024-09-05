/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_policy_server.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
constexpr int32_t OFFSET = 4;
const int32_t MOD_NUM_TWO = 2;
const int32_t CONNECTING_NUMBER = 10;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_ONE = 0;
const uint32_t LIMIT_TWO = 30;
const uint32_t LIMIT_THREE = 60;
const uint32_t LIMIT_FOUR = static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX);
bool g_hasServerInit = false;

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
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

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioPolicyServiceDumpTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_TWO - LIMIT_ONE + 1) + LIMIT_ONE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    std::string dumpStr = "";
    GetServerPtr()->audioPolicyService_.DevicesInfoDump(dumpStr);
    GetServerPtr()->audioPolicyService_.AudioModeDump(dumpStr);
    GetServerPtr()->audioPolicyService_.AudioPolicyParserDump(dumpStr);
    GetServerPtr()->audioPolicyService_.XmlParsedDataMapDump(dumpStr);
    GetServerPtr()->audioPolicyService_.StreamVolumesDump(dumpStr);
    std::map<DeviceVolumeType, std::shared_ptr<DeviceVolumeInfo>> deviceVolumeInfoMap;
    GetServerPtr()->audioPolicyService_.DeviceVolumeInfosDump(dumpStr, deviceVolumeInfoMap);
    GetServerPtr()->audioPolicyService_.AudioStreamDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetGroupInfoDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetCallStatusDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetRingerModeDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetMicrophoneDescriptorsDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetCapturerStreamDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetSafeVolumeDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetOffloadStatusDump(dumpStr);
    GetServerPtr()->audioPolicyService_.EffectManagerInfoDump(dumpStr);
    GetServerPtr()->audioPolicyService_.MicrophoneMuteInfoDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetVolumeConfigDump(dumpStr);
    GetServerPtr()->audioPolicyService_.GetVolumeConfigDump(dumpStr);

    GetServerPtr()->interruptService_->AudioInterruptZoneDump(dumpStr);
}

void AudioPolicyServiceDeviceTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint8_t num = *reinterpret_cast<const uint8_t *>(rawData);
    DeviceType deviceType = static_cast<DeviceType>(num % DEVICE_TYPE_FILE_SOURCE);
    bool isConnected = static_cast<bool>(num % MOD_NUM_TWO);
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(deviceType, isConnected);

    GetServerPtr()->audioPolicyService_.IsA2dpOffloadConnected();
    int32_t state = (num % MOD_NUM_TWO) + CONNECTING_NUMBER; // DATA_LINK_CONNECTING = 10, DATA_LINK_CONNECTED = 11;
    GetServerPtr()->audioPolicyService_.UpdateSessionConnectionState(num, state);

    std::string macAddress = "11:22:33:44:55:66";
    GetServerPtr()->audioPolicyService_.SetCallDeviceActive(deviceType, isConnected, macAddress);

    GetServerPtr()->audioPolicyService_.UpdateOffloadWhenActiveDeviceSwitchFromA2dp();
    GetServerPtr()->audioPolicyService_.GetA2dpOffloadCodecAndSendToDsp();
    GetServerPtr()->audioPolicyService_.UpdateAudioCapturerMicrophoneDescriptor(deviceType);

    // the max value of BluetoothOffloadState is A2DP_OFFLOAD.
    BluetoothOffloadState flag = static_cast<BluetoothOffloadState>(num % (A2DP_OFFLOAD + 1));
    GetServerPtr()->audioPolicyService_.HandleA2dpDeviceInOffload(flag);
    GetServerPtr()->audioPolicyService_.HandleA2dpDeviceOutOffload(flag);

    bool captureSilentState = static_cast<bool>(num % MOD_NUM_TWO);
    GetServerPtr()->audioPolicyService_.SetCaptureSilentState(captureSilentState);

    uint32_t sessionId = static_cast<uint32_t>(num);
    // the max value of AudioPipeType is PIPE_TYPE_DIRECT_VOIP.
    AudioPipeType pipeType = static_cast<AudioPipeType>(num % (PIPE_TYPE_DIRECT_VOIP + 1));
    GetServerPtr()->audioPolicyService_.MoveToNewPipeInner(sessionId, pipeType);

    GetServerPtr()->audioPolicyService_.LoadMchModule();
    GetServerPtr()->audioPolicyService_.ConstructMchAudioModuleInfo(deviceType);
    GetServerPtr()->audioPolicyService_.LoadOffloadModule();
    GetServerPtr()->audioPolicyService_.UnloadOffloadModule();
    GetServerPtr()->audioPolicyService_.ConstructOffloadAudioModuleInfo(deviceType);
}

void AudioPolicyServiceAccountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint8_t num = *reinterpret_cast<const uint8_t *>(rawData);
    GetServerPtr()->audioPolicyService_.GetCurActivateCount();
    GetServerPtr()->audioPolicyService_.NotifyAccountsChanged(num);
}

void AudioPolicyServiceSafeVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t code = Convert2Uint32(rawData) % (LIMIT_TWO - LIMIT_ONE + 1) + LIMIT_ONE;

    rawData = rawData + OFFSET;
    size = size - OFFSET;

    GetServerPtr()->audioPolicyService_.SetDeviceSafeVolumeStatus();
    GetServerPtr()->audioPolicyService_.DisableSafeMediaVolume();
    GetServerPtr()->audioPolicyService_.SafeVolumeDialogDisapper();
}

void AudioPolicyServiceInterfaceTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    bool fuzzBool = *reinterpret_cast<const bool *>(rawData);
    int32_t fuzzInt32One = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzInt32Two = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzInt32Three = *reinterpret_cast<const int8_t *>(rawData);
    int32_t fuzzFloat = *reinterpret_cast<const float *>(rawData);
    std::string fuzzNetworkId = "FUZZNETWORKID";
    std::string fuzzString(reinterpret_cast<const char*>(rawData), size - 1);

    AudioStreamType fuzzAudioStreamType = *reinterpret_cast<const AudioStreamType *>(rawData);
    DeviceType fuzzDeviceType = *reinterpret_cast<const DeviceType *>(rawData);
    DeviceRole fuzzDeviceRole = *reinterpret_cast<const DeviceRole *>(rawData);
    StreamUsage fuzzStreamUsage = *reinterpret_cast<const StreamUsage *>(rawData);

    AudioDeviceDescriptor fuzzAudioDeviceDescriptor;
    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> fuzzAudioDeviceDescriptorSptrVector;
    fuzzAudioDeviceDescriptorSptrVector.push_back(fuzzAudioDeviceDescriptorSptr);
    sptr<AudioRendererFilter> fuzzAudioRendererFilter = new AudioRendererFilter();

    GetServerPtr()->audioPolicyService_.isOffloadAvailable_ = true; // set offload support on for covery

    GetServerPtr()->audioPolicyService_.HandleRecoveryPreferredDevices(fuzzInt32One, fuzzInt32Two, fuzzInt32Three);
    GetServerPtr()->audioPolicyService_.GetVolumeGroupType(fuzzDeviceType);
    GetServerPtr()->audioPolicyService_.GetSystemVolumeDb(fuzzAudioStreamType);
    GetServerPtr()->audioPolicyService_.SetLowPowerVolume(fuzzInt32One, fuzzFloat);
    GetServerPtr()->audioPolicyService_.SetOffloadMode();
    GetServerPtr()->audioPolicyService_.ResetOffloadMode(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.OffloadStreamReleaseCheck(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.RemoteOffloadStreamRelease(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.CheckActiveOutputDeviceSupportOffload();
    GetServerPtr()->audioPolicyService_.GetOffloadAvailableFromXml();
    GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(fuzzInt32One, fuzzBool);
    GetServerPtr()->audioPolicyService_.NotifyRemoteRenderState(fuzzNetworkId, fuzzString, fuzzString);
    GetServerPtr()->audioPolicyService_.IsArmUsbDevice(fuzzAudioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.IsDeviceConnected(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.DeviceParamsCheck(fuzzDeviceRole, fuzzAudioDeviceDescriptorSptrVector);
    GetServerPtr()->audioPolicyService_.NotifyUserSelectionEventToBt(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.SetRenderDeviceForUsage(fuzzStreamUsage, fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.SelectOutputDevice(
        fuzzAudioRendererFilter, fuzzAudioDeviceDescriptorSptrVector);
    GetServerPtr()->audioPolicyService_.WriteSelectOutputSysEvents(
        fuzzAudioDeviceDescriptorSptrVector, fuzzStreamUsage);
    GetServerPtr()->audioPolicyService_.SelectFastOutputDevice(
        fuzzAudioRendererFilter, fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.FilterSinkInputs(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.FilterSourceOutputs(fuzzInt32One);
    GetServerPtr()->audioPolicyService_.RememberRoutingInfo(fuzzAudioRendererFilter, fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(fuzzDeviceType, fuzzBool);
    GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(fuzzDeviceType, fuzzBool, fuzzString, fuzzString);
}

void AudioDeviceConnectTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    // Coverage first
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_WIRED_HEADSET, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_WIRED_HEADSET, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_USB_HEADSET, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_USB_HEADSET, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_DP, true,
        "", "fuzzDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_DP, false,
        "", "fuzzDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, true,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, false,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, true,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, false,
        "08:00:20:0A:8C:6D", "fuzzBtDevice", streamInfo);

    bool fuzzBool = *reinterpret_cast<const bool *>(rawData);
    std::string fuzzString(reinterpret_cast<const char*>(rawData), size - 1);
    DeviceType fuzzDeviceType = *reinterpret_cast<const DeviceType *>(rawData);
    AudioSamplingRate fuzzAudioSamplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    AudioChannel fuzzAudioChannel = *reinterpret_cast<const AudioChannel *>(rawData);
    AudioSampleFormat fuzzAudioSampleFormat = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    streamInfo.samplingRate = fuzzAudioSamplingRate;
    streamInfo.channels = fuzzAudioChannel;
    streamInfo.format = fuzzAudioSampleFormat;

    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(fuzzDeviceType, fuzzBool, fuzzString,
        "fuzzDevice", streamInfo);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyServiceDumpTest(data, size);
    OHOS::AudioStandard::AudioPolicyServiceDeviceTest(data, size);
    OHOS::AudioStandard::AudioPolicyServiceAccountTest(data, size);
    OHOS::AudioStandard::AudioPolicyServiceSafeVolumeTest(data, size);
    OHOS::AudioStandard::AudioPolicyServiceInterfaceTest(data, size);
    OHOS::AudioStandard::AudioDeviceConnectTest(data, size);

    return 0;
}
