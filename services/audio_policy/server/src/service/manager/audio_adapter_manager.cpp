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

#include <memory>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_log.h"
#include "parameter.h"

#include "audio_adapter_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
bool AudioAdapterManager::Init()
{
    char testMode[10] = {0}; // 10 for system parameter usage
    auto res = GetParameter("debug.audio_service.testmodeon", "0", testMode, sizeof(testMode));
    if (res == 1 && testMode[0] == '1') {
        AUDIO_DEBUG_LOG("testMode on");
        testModeOn_ = true;
    }

    // init volume before kvstore start by local prop for bootanimation
    char currentVolumeValue[3] = {0};
    auto ret = GetParameter("persist.multimedia.audio.mediavolume", "15",
        currentVolumeValue, sizeof(currentVolumeValue));
    if (ret > 0) {
        int32_t mediaVolumeLevel = atoi(currentVolumeValue);
        volumeLevelMap_[STREAM_MUSIC] = mediaVolumeLevel;
        AUDIO_INFO_LOG("Init: Get music volume to map success %{public}d", volumeLevelMap_[STREAM_MUSIC]);
    } else {
        AUDIO_ERR_LOG("Init: Get volume parameter failed %{public}d", ret);
    }

    return true;
}

bool AudioAdapterManager::ConnectServiceAdapter()
{
    std::shared_ptr<AudioAdapterManager> audioAdapterManager(this);
    std::unique_ptr<PolicyCallbackImpl> policyCallbackImpl = std::make_unique<PolicyCallbackImpl>(audioAdapterManager);
    audioServiceAdapter_ = AudioServiceAdapter::CreateAudioAdapter(std::move(policyCallbackImpl));
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("[AudioAdapterManager] Error in audio adapter initialization");
        return false;
    }

    bool result = audioServiceAdapter_->Connect();
    if (!result) {
        AUDIO_ERR_LOG("[AudioAdapterManager] Error in connecting audio adapter");
        return false;
    }

    return true;
}

void AudioAdapterManager::InitKVStore()
{
    bool isFirstBoot = false;
    InitAudioPolicyKvStore(isFirstBoot);
    InitVolumeMap(isFirstBoot);
    InitRingerMode(isFirstBoot);
    InitMuteStatusMap(isFirstBoot);
}

void AudioAdapterManager::Deinit(void)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("Deinit audio adapter null");
        return;
    }

    return audioServiceAdapter_->Disconnect();
}

int32_t AudioAdapterManager::SetAudioSessionCallback(AudioSessionCallback *callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetAudioSessionCallback callback == nullptr");
        return ERR_INVALID_PARAM;
    }

    sessionCallback_ = callback;
    return SUCCESS;
}

int32_t AudioAdapterManager::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    return MAX_VOLUME_LEVEL;
}

int32_t AudioAdapterManager::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_VOICE_ASSISTANT || volumeType == STREAM_VOICE_CALL ||
        volumeType == STREAM_ALARM || volumeType == STREAM_ACCESSIBILITY) {
        return (MIN_VOLUME_LEVEL + 1);
    }
    return MIN_VOLUME_LEVEL;
}

int32_t AudioAdapterManager::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel)
{
    if (volumeLevel == 0 &&
        (streamType == STREAM_VOICE_ASSISTANT || streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("SetSystemVolumeLevel this type can not set mute");
        return SUCCESS;
    }

    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SetSystemVolumeLevel audio adapter null");
        return ERR_OPERATION_FAILED;
    }

    // In case if KvStore didnot connect during bootup
    if (audioPolicyKvStore_ == nullptr) {
        bool isFirstBoot = false;
        InitAudioPolicyKvStore(isFirstBoot);
    }

    if (streamType == STREAM_RING) {
        if (volumeLevel > 0 && (ringerMode_ == RINGER_MODE_SILENT || ringerMode_ == RINGER_MODE_VIBRATE)) {
            // ring volume > 0, change the ringer mode to RINGER_MODE_NORMAL
            SetRingerMode(RINGER_MODE_NORMAL);
        } else if (volumeLevel == 0 && ringerMode_ == RINGER_MODE_NORMAL) {
            // ring volume == 0, change the ringer mode to RINGER_MODE_VIBRATE and don't wirte ring volume data
            SetRingerMode(RINGER_MODE_VIBRATE);
            return audioServiceAdapter_->SetVolumeDb(streamType, CalculateVolumeDb(volumeLevel));
        }
    }

    if (volumeLevel > 0 && GetStreamMute(streamType)) {
        SetStreamMute(streamType, false);
    }

    AudioStreamType streamForVolumeMap = GetStreamForVolumeMap(streamType);
    volumeLevelMap_[streamForVolumeMap] = volumeLevel;
    WriteVolumeToKvStore(currentActiveDevice_, streamType, volumeLevel);

    // Save volume in local prop for bootanimation
    if (streamType == STREAM_MUSIC) {
        int ret = SetParameter("persist.multimedia.audio.mediavolume", std::to_string(volumeLevel).c_str());
        if (ret == 0) {
            AUDIO_INFO_LOG("Save media volume for boot success %{public}d", volumeLevel);
        } else {
            AUDIO_ERR_LOG("Save media volume for boot failed, result %{public}d", ret);
        }
    }

    float volumeDb = CalculateVolumeDb(volumeLevel);
    return audioServiceAdapter_->SetVolumeDb(streamType, volumeDb);
}

int32_t AudioAdapterManager::GetSystemVolumeLevel(AudioStreamType streamType)
{
    if (GetStreamMute(streamType)) {
        return MIN_VOLUME_LEVEL;
    }
    AudioStreamType streamForVolumeMap = GetStreamForVolumeMap(streamType);

    return volumeLevelMap_[streamForVolumeMap];
}

float AudioAdapterManager::GetSystemVolumeDb(AudioStreamType streamType)
{
    AudioStreamType streamForVolumeMap = GetStreamForVolumeMap(streamType);

    int32_t volumeLevel = volumeLevelMap_[streamForVolumeMap];
    return CalculateVolumeDb(volumeLevel);
}

int32_t AudioAdapterManager::SetStreamMute(AudioStreamType streamType, bool mute)
{
    if (mute &&
        (streamType == STREAM_VOICE_ASSISTANT || streamType == STREAM_VOICE_CALL ||
        streamType == STREAM_ALARM || streamType == STREAM_ACCESSIBILITY)) {
        // these types can not set to mute, but don't return error
        AUDIO_ERR_LOG("SetStreamMute: this type can not set mute");
        return SUCCESS;
    }
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SetStreamMute: audio adapter null");
        return ERR_OPERATION_FAILED;
    }
    AudioStreamType streamForVolumeMap = GetStreamForVolumeMap(streamType);
    muteStatusMap_[streamForVolumeMap] = mute;
    WriteMuteStatusToKvStore(currentActiveDevice_, streamType, mute);
    return audioServiceAdapter_->SetMute(streamType, mute);
}

int32_t AudioAdapterManager::SetSourceOutputStreamMute(int32_t uid, bool setMute)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SetSourceOutputStreamMute audio adapter null");
        return ERR_OPERATION_FAILED;
    }
    return audioServiceAdapter_->SetSourceOutputMute(uid, setMute);
}

bool AudioAdapterManager::GetStreamMute(AudioStreamType streamType)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetStreamMute audio adapter null");
        return false;
    }
    AudioStreamType streamForVolumeMap = GetStreamForVolumeMap(streamType);
    return muteStatusMap_[streamForVolumeMap];
}

bool AudioAdapterManager::IsStreamActive(AudioStreamType streamType)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("IsStreamActive audio adapter null");
        return false;
    }

    return audioServiceAdapter_->IsStreamActive(streamType);
}

vector<SinkInput> AudioAdapterManager::GetAllSinkInputs()
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetAllSinkInputs audio adapter null");
        vector<SinkInput> sinkInputList;
        return sinkInputList;
    }

    return audioServiceAdapter_->GetAllSinkInputs();
}

vector<SourceOutput> AudioAdapterManager::GetAllSourceOutputs()
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("GetAllSourceOutputs audio adapter null");
        vector<SourceOutput> sourceOutputList;
        return sourceOutputList;
    }

    return audioServiceAdapter_->GetAllSourceOutputs();
}

int32_t AudioAdapterManager::SuspendAudioDevice(std::string &portName, bool isSuspend)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SuspendAudioDevice audio adapter null");
        return ERR_OPERATION_FAILED;
    }

    return audioServiceAdapter_->SuspendAudioDevice(portName, isSuspend);
}

int32_t AudioAdapterManager::SelectDevice(DeviceRole deviceRole, InternalDeviceType deviceType, std::string name)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SelectDevice audio adapter null");
        return ERR_OPERATION_FAILED;
    }
    switch (deviceRole) {
        case DeviceRole::INPUT_DEVICE:
            return audioServiceAdapter_->SetDefaultSource(name);
        case DeviceRole::OUTPUT_DEVICE: {
            SetVolumeForSwitchDevice(deviceType);
            AUDIO_INFO_LOG("SetDefaultSink %{public}d", deviceType);
            return audioServiceAdapter_->SetDefaultSink(name);
        }
        default:
            AUDIO_ERR_LOG("SelectDevice error deviceRole %{public}d", deviceRole);
            return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::SetDeviceActive(AudioIOHandle ioHandle, InternalDeviceType deviceType,
    std::string name, bool active)
{
    if (!audioServiceAdapter_) {
        AUDIO_ERR_LOG("SetDeviceActive audio adapter null");
        return ERR_OPERATION_FAILED;
    }

    switch (deviceType) {
        case InternalDeviceType::DEVICE_TYPE_EARPIECE:
        case InternalDeviceType::DEVICE_TYPE_SPEAKER:
        case InternalDeviceType::DEVICE_TYPE_FILE_SINK:
        case InternalDeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case InternalDeviceType::DEVICE_TYPE_USB_HEADSET:
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_SCO: {
            SetVolumeForSwitchDevice(deviceType);
            AUDIO_INFO_LOG("SetDefaultSink %{public}d", deviceType);
            return audioServiceAdapter_->SetDefaultSink(name);
        }
        case InternalDeviceType::DEVICE_TYPE_FILE_SOURCE:
        case InternalDeviceType::DEVICE_TYPE_MIC: {
            AUDIO_INFO_LOG("SetDefaultSource %{public}d", deviceType);
            return audioServiceAdapter_->SetDefaultSource(name);
        }
        default:
            break;
    }
    return SUCCESS;
}

void AudioAdapterManager::SetVolumeForSwitchDevice(InternalDeviceType deviceType)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("SetVolumeForSwitchDevice audioPolicyKvStore_ is null!");
        return;
    }
    currentActiveDevice_ = deviceType;
    LoadVolumeMap();
    std::vector<AudioStreamType> streamTypeList = {
        STREAM_MUSIC,
        STREAM_RING,
        STREAM_VOICE_CALL,
        STREAM_VOICE_ASSISTANT,
        STREAM_ALARM,
        STREAM_ACCESSIBILITY,
        STREAM_ULTRASONIC
    };
    auto iter = streamTypeList.begin();
    while (iter != streamTypeList.end()) {
        Key key = GetStreamNameByStreamType(deviceType, *iter);
        Value value = Value(TransferTypeToByteArray<int>(0));
        Status status = audioPolicyKvStore_->Get(key, value);
        if (status == SUCCESS) {
            int32_t volumeLevel = TransferByteArrayToType<int>(value.Data());
            SetSystemVolumeLevel(*iter, volumeLevel);
        }
        iter++;
    }
}

int32_t AudioAdapterManager::MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName)
{
    return audioServiceAdapter_->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

int32_t AudioAdapterManager::MoveSourceOutputByIndexOrName(uint32_t sourceOutputId, uint32_t sourceIndex,
    std::string sourceName)
{
    return audioServiceAdapter_->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
}

int32_t AudioAdapterManager::SetRingerMode(AudioRingerMode ringerMode)
{
    AUDIO_INFO_LOG("SetRingerMode: %{public}d", ringerMode);
    ringerMode_ = ringerMode;

    switch (ringerMode) {
        case RINGER_MODE_SILENT:
        case RINGER_MODE_VIBRATE:
            SetStreamMute(STREAM_RING, true);
            break;
        case RINGER_MODE_NORMAL:
            SetStreamMute(STREAM_RING, false);
            break;
        default:
            break;
    }

    // In case if KvStore didnot connect during bootup
    if (audioPolicyKvStore_ == nullptr) {
        bool isFirstBoot = false;
        InitAudioPolicyKvStore(isFirstBoot);
    }

    WriteRingerModeToKvStore(ringerMode);
    return SUCCESS;
}

AudioRingerMode AudioAdapterManager::GetRingerMode() const
{
    return ringerMode_;
}

AudioIOHandle AudioAdapterManager::OpenAudioPort(const AudioModuleInfo &audioModuleInfo)
{
    std::string moduleArgs = GetModuleArgs(audioModuleInfo);
    AUDIO_INFO_LOG("[Adapter load-module] %{public}s %{public}s", audioModuleInfo.lib.c_str(), moduleArgs.c_str());

    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    return audioServiceAdapter_->OpenAudioPort(audioModuleInfo.lib, moduleArgs.c_str());
}

int32_t AudioAdapterManager::CloseAudioPort(AudioIOHandle ioHandle)
{
    CHECK_AND_RETURN_RET_LOG(audioServiceAdapter_ != nullptr, ERR_OPERATION_FAILED, "ServiceAdapter is null");
    return audioServiceAdapter_->CloseAudioPort(ioHandle);
}

void UpdateCommonArgs(const AudioModuleInfo &audioModuleInfo, std::string &args)
{
    if (!audioModuleInfo.rate.empty()) {
        args = "rate=";
        args.append(audioModuleInfo.rate);
    }

    if (!audioModuleInfo.channels.empty()) {
        args.append(" channels=");
        args.append(audioModuleInfo.channels);
    }

    if (!audioModuleInfo.bufferSize.empty()) {
        args.append(" buffer_size=");
        args.append(audioModuleInfo.bufferSize);
    }

    if (!audioModuleInfo.format.empty()) {
        args.append(" format=");
        args.append(audioModuleInfo.format);
        AUDIO_INFO_LOG("[PolicyManager] format: %{public}s", args.c_str());
    }

    if (!audioModuleInfo.fixedLatency.empty()) {
        args.append(" fixed_latency=");
        args.append(audioModuleInfo.fixedLatency);
    }

    if (!audioModuleInfo.renderInIdleState.empty()) {
        args.append(" render_in_idle_state=");
        args.append(audioModuleInfo.renderInIdleState);
    }

    if (!audioModuleInfo.OpenMicSpeaker.empty()) {
        args.append(" open_mic_speaker=");
        args.append(audioModuleInfo.OpenMicSpeaker);
    }
}

// Private Members
std::string AudioAdapterManager::GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const
{
    std::string args;

    if (audioModuleInfo.lib == HDI_SINK) {
        UpdateCommonArgs(audioModuleInfo, args);
        if (!audioModuleInfo.name.empty()) {
            args.append(" sink_name=");
            args.append(audioModuleInfo.name);
        }

        if (!audioModuleInfo.adapterName.empty()) {
            args.append(" adapter_name=");
            args.append(audioModuleInfo.adapterName);
        }

        if (!audioModuleInfo.className.empty()) {
            args.append(" device_class=");
            args.append(audioModuleInfo.className);
        }

        if (!audioModuleInfo.fileName.empty()) {
            args.append(" file_path=");
            args.append(audioModuleInfo.fileName);
        }
        if (!audioModuleInfo.sinkLatency.empty()) {
            args.append(" sink_latency=");
            args.append(audioModuleInfo.sinkLatency);
        }
        if (testModeOn_) {
            args.append(" test_mode_on=");
            args.append("1");
        }
        if (!audioModuleInfo.networkId.empty()) {
            args.append(" network_id=");
            args.append(audioModuleInfo.networkId);
        } else {
            args.append(" network_id=LocalDevice");
        }

        if (!audioModuleInfo.deviceType.empty()) {
            args.append(" device_type=");
            args.append(audioModuleInfo.deviceType);
        }
    } else if (audioModuleInfo.lib == HDI_SOURCE) {
        UpdateCommonArgs(audioModuleInfo, args);
        if (!audioModuleInfo.name.empty()) {
            args.append(" source_name=");
            args.append(audioModuleInfo.name);
        }

        if (!audioModuleInfo.adapterName.empty()) {
            args.append(" adapter_name=");
            args.append(audioModuleInfo.adapterName);
        }

        if (!audioModuleInfo.className.empty()) {
            args.append(" device_class=");
            args.append(audioModuleInfo.className);
        }

        if (!audioModuleInfo.fileName.empty()) {
            args.append(" file_path=");
            args.append(audioModuleInfo.fileName);
        }

        if (!audioModuleInfo.networkId.empty()) {
            args.append(" network_id=");
            args.append(audioModuleInfo.networkId);
        } else {
            args.append(" network_id=LocalDevice");
        }

        if (!audioModuleInfo.deviceType.empty()) {
            args.append(" device_type=");
            args.append(audioModuleInfo.deviceType);
        }
    } else if (audioModuleInfo.lib == PIPE_SINK) {
        if (!audioModuleInfo.fileName.empty()) {
            args = "file=";
            args.append(audioModuleInfo.fileName);
        }
    } else if (audioModuleInfo.lib == PIPE_SOURCE) {
        if (!audioModuleInfo.fileName.empty()) {
            args = "file=";
            args.append(audioModuleInfo.fileName);
        }
    }
    return args;
}

std::string AudioAdapterManager::GetStreamNameByStreamType(DeviceType deviceType, AudioStreamType streamType)
{
    std::string type;
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
            type = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            type = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
            type = "wired";
            break;
        default:
            AUDIO_ERR_LOG("[GetStreamNameByStreamType] deivice %{public}d is not supported for kvStore", deviceType);
            return "";
    }

    switch (streamType) {
        case STREAM_MUSIC:
            return type + "music";
        case STREAM_RING:
            return type + "ring";
        case STREAM_SYSTEM:
            return type + "system";
        case STREAM_NOTIFICATION:
            return type + "notification";
        case STREAM_ALARM:
            return type + "alarm";
        case STREAM_DTMF:
            return type + "dtmf";
        case STREAM_VOICE_CALL:
            return type + "voice_call";
        case STREAM_VOICE_ASSISTANT:
            return type + "voice_assistant";
        case STREAM_ACCESSIBILITY:
            return type + "accessibility";
        case STREAM_ULTRASONIC:
            return type + "ultrasonic";
        default:
            return "";
    }
}

AudioStreamType AudioAdapterManager::GetStreamIDByType(std::string streamType)
{
    AudioStreamType stream = STREAM_MUSIC;

    if (!streamType.compare(std::string("music")))
        stream = STREAM_MUSIC;
    else if (!streamType.compare(std::string("ring")))
        stream = STREAM_RING;
    else if (!streamType.compare(std::string("voice_call")))
        stream = STREAM_VOICE_CALL;
    else if (!streamType.compare(std::string("system")))
        stream = STREAM_SYSTEM;
    else if (!streamType.compare(std::string("notification")))
        stream = STREAM_NOTIFICATION;
    else if (!streamType.compare(std::string("alarm")))
        stream = STREAM_ALARM;
    else if (!streamType.compare(std::string("voice_assistant")))
        stream = STREAM_VOICE_ASSISTANT;
    else if (!streamType.compare(std::string("accessibility")))
        stream = STREAM_ACCESSIBILITY;
    else if (!streamType.compare(std::string("ultrasonic")))
        stream = STREAM_ULTRASONIC;

    return stream;
}

AudioStreamType AudioAdapterManager::GetStreamForVolumeMap(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_MUSIC:
            return STREAM_MUSIC;
        case STREAM_NOTIFICATION:
        case STREAM_DTMF:
        case STREAM_SYSTEM:
        case STREAM_RING:
            return STREAM_RING;
        case STREAM_ALARM:
            return STREAM_ALARM;
        case STREAM_VOICE_CALL:
            return STREAM_VOICE_CALL;
        case STREAM_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_ACCESSIBILITY:
            return STREAM_ACCESSIBILITY;
        case STREAM_ULTRASONIC:
            return STREAM_ULTRASONIC;
        default:
            return STREAM_MUSIC;
    }
}

bool AudioAdapterManager::InitAudioPolicyKvStore(bool& isFirstBoot)
{
    DistributedKvDataManager manager;
    Options options;

    AppId appId;
    appId.appId = "audio_policy_manager";

    options.securityLevel = S1;
    options.createIfMissing = true;
    options.encrypt = false;
    options.autoSync = false;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;
    options.area = EL1;
    options.baseDir = std::string("/data/service/el1/public/database/") + appId.appId;

    StoreId storeId;
    storeId.storeId = "audiopolicy";
    Status status = Status::SUCCESS;

    // open and initialize kvstore instance.
    if (audioPolicyKvStore_ == nullptr) {
        uint32_t retries = 0;

        do {
            status = manager.GetSingleKvStore(options, appId, storeId, audioPolicyKvStore_);
            if (status == Status::STORE_NOT_FOUND) {
                AUDIO_ERR_LOG("InitAudioPolicyKvStore: STORE_NOT_FOUND!");
            }

            if ((status == Status::SUCCESS) || (status == Status::STORE_NOT_FOUND)) {
                break;
            } else {
                AUDIO_ERR_LOG("InitAudioPolicyKvStore: Kvstore Connect failed! Retrying.");
                retries++;
                usleep(KVSTORE_CONNECT_RETRY_DELAY_TIME);
            }
        } while (retries <= KVSTORE_CONNECT_RETRY_COUNT);
    }

    if (audioPolicyKvStore_ == nullptr) {
        if (status == Status::STORE_NOT_FOUND) {
            AUDIO_INFO_LOG("First Boot: Create AudioPolicyKvStore");
            options.createIfMissing = true;
            // [create and] open and initialize kvstore instance.
            status = manager.GetSingleKvStore(options, appId, storeId, audioPolicyKvStore_);
            if (status == Status::SUCCESS) {
                isFirstBoot = true;
            } else {
                AUDIO_ERR_LOG("Create AudioPolicyKvStore Failed!");
            }
        }
    }

    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("InitAudioPolicyKvStore: Failed!");
        return false;
    }

    return true;
}

void AudioAdapterManager::InitVolumeMap(bool isFirstBoot)
{
    if (isFirstBoot == true) {
        auto iter = deviceList_.begin();
        while (iter != deviceList_.end()) {
            WriteVolumeToKvStore(*iter, STREAM_MUSIC, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_RING, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_VOICE_CALL, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_VOICE_ASSISTANT, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_ALARM, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_ACCESSIBILITY, DEFAULT_VOLUME_LEVEL);
            WriteVolumeToKvStore(*iter, STREAM_ULTRASONIC, MAX_VOLUME_LEVEL);
            iter++;
        }
        AUDIO_INFO_LOG("Wrote default stream volumes to KvStore");
    } else {
        LoadVolumeMap();
    }
}

void AudioAdapterManager::InitRingerMode(bool isFirstBoot)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("InitRingerMode kvstore is null!");
        return;
    }

    if (isFirstBoot == true) {
        ringerMode_ = RINGER_MODE_NORMAL;
        WriteRingerModeToKvStore(RINGER_MODE_NORMAL);
        AUDIO_INFO_LOG("InitRingerMode Wrote default ringer mode to KvStore");
    } else {
        LoadRingerMode();
    }
}

bool AudioAdapterManager::LoadVolumeFromKvStore(DeviceType deviceType, AudioStreamType streamType)
{
    Key key;
    Value value;
    std::string type;

    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
            type = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            type = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
            type = "wired";
            break;
        default:
            AUDIO_ERR_LOG("LoadVolumeFromKvStore device %{public}d is not supported for kv"
                "store", deviceType);
            return false;
    }

    switch (streamType) {
        case STREAM_MUSIC:
            key = type + "music";
            break;
        case STREAM_RING:
            key = type + "ring";
            break;
        case STREAM_VOICE_CALL:
            key = type + "voice_call";
            break;
        case STREAM_VOICE_ASSISTANT:
            key = type + "voice_assistant";
            break;
        case STREAM_ALARM:
            key = type + "alarm";
            break;
        case STREAM_ACCESSIBILITY:
            key = type + "accessibility";
            break;
        case STREAM_ULTRASONIC:
            key = type + "ultrasonic";
            break;
        default:
            return false;
    }

    Status status = audioPolicyKvStore_->Get(key, value);
    if (status == Status::SUCCESS) {
        int32_t volumeLevel = TransferByteArrayToType<int>(value.Data());
        volumeLevelMap_[streamType] = volumeLevel;
        AUDIO_DEBUG_LOG("LoadVolumeFromKvStore volume from kvStore %{public}d for streamType: %{public}d",
            volumeLevel, streamType);
        return true;
    }

    return false;
}

bool AudioAdapterManager::LoadVolumeMap(void)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("LoadVolumeMap: audioPolicyKvStore_ is null!");
        return false;
    }

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_MUSIC))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for music from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_RING))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for ring from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_VOICE_CALL))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for voice_call from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_VOICE_ASSISTANT))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for voice_assistant from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_ALARM))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for alarm from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_ACCESSIBILITY))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for accessibility from kvStore");

    if (!LoadVolumeFromKvStore(currentActiveDevice_, STREAM_ULTRASONIC))
        AUDIO_ERR_LOG("LoadVolumeMap: Could not load volume for ultrasonic from kvStore");

    return true;
}

bool AudioAdapterManager::LoadRingerMode(void)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("LoadRingerMap: audioPolicyKvStore_ is null!");
        return false;
    }

    // get ringer mode value from kvstore.
    Key key = "ringermode";
    Value value;
    Status status = audioPolicyKvStore_->Get(key, value);
    if (status == Status::SUCCESS) {
        ringerMode_ = static_cast<AudioRingerMode>(TransferByteArrayToType<int>(value.Data()));
        AUDIO_DEBUG_LOG("LoadRingerMode Ringer Mode from kvStore %{public}d", ringerMode_);
    }

    return true;
}

void AudioAdapterManager::WriteVolumeToKvStore(DeviceType type, AudioStreamType streamType, int32_t volumeLevel)
{
    if (audioPolicyKvStore_ == nullptr)
        return;

    Key key = GetStreamNameByStreamType(type, streamType);
    Value value = Value(TransferTypeToByteArray<int>(volumeLevel));

    Status status = audioPolicyKvStore_->Put(key, value);
    if (status == Status::SUCCESS) {
        AUDIO_INFO_LOG("WriteVolumeToKvStore volumeLevel %{public}d for %{public}s updated in kvStore",
            volumeLevel, GetStreamNameByStreamType(type, streamType).c_str());
    } else {
        AUDIO_ERR_LOG("WriteVolumeToKvStore volumeLevel %{public}d for %{public}s failed to update in kvStore!",
            volumeLevel, GetStreamNameByStreamType(type, streamType).c_str());
    }

    return;
}

void AudioAdapterManager::WriteRingerModeToKvStore(AudioRingerMode ringerMode)
{
    if (audioPolicyKvStore_ == nullptr)
        return;

    Key key = "ringermode";
    Value value = Value(TransferTypeToByteArray<int>(ringerMode));

    Status status = audioPolicyKvStore_->Put(key, value);
    if (status == Status::SUCCESS) {
        AUDIO_INFO_LOG("WriteRingerModeToKvStore Wrote RingerMode:%d to kvStore", ringerMode);
    } else {
        AUDIO_ERR_LOG("WriteRingerModeToKvStore Writing RingerMode:%d to kvStore failed!", ringerMode);
    }

    return;
}

void AudioAdapterManager::InitMuteStatusMap(bool isFirstBoot)
{
    if (isFirstBoot == true) {
        auto iter = deviceList_.begin();
        while (iter != deviceList_.end()) {
            WriteMuteStatusToKvStore(*iter, STREAM_MUSIC, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_RING, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_VOICE_CALL, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_VOICE_ASSISTANT, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_ALARM, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_ACCESSIBILITY, 0);
            WriteMuteStatusToKvStore(*iter, STREAM_ULTRASONIC, 0);
            iter++;
        }
        AUDIO_INFO_LOG("InitMuteStatusMap Wrote default mute status to KvStore");
    } else {
        LoadMuteStatusMap();
    }
}


bool AudioAdapterManager::LoadMuteStatusMap(void)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("LoadMuteStatusMap: audioPolicyKvStore_ is null");
        return false;
    }

    if (!LoadMuteStatusFromKvStore(STREAM_MUSIC))
        AUDIO_ERR_LOG("Could not load mute status for MUSIC from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_RING))
        AUDIO_ERR_LOG("Could not load mute status for RING from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_VOICE_CALL))
        AUDIO_ERR_LOG("Could not load mute status for VOICE_CALL from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_VOICE_ASSISTANT))
        AUDIO_ERR_LOG("Could not load mute status for VOICE_ASSISTANT from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_ALARM))
        AUDIO_ERR_LOG("Could not load mute status for ALARM from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_ACCESSIBILITY))
        AUDIO_ERR_LOG("Could not load mute status for ACCESSIBILITY from kvStore");

    if (!LoadMuteStatusFromKvStore(STREAM_ULTRASONIC))
        AUDIO_ERR_LOG("Could not load mute status for ULTRASONIC from kvStore");

    return true;
}

bool AudioAdapterManager::LoadMuteStatusFromKvStore(AudioStreamType streamType)
{
    Value value;

    switch (streamType) {
        case STREAM_MUSIC:
            break;
        case STREAM_RING:
            break;
        case STREAM_VOICE_CALL:
            break;
        case STREAM_VOICE_ASSISTANT:
            break;
        case STREAM_ALARM:
            break;
        case STREAM_ACCESSIBILITY:
            break;
        case STREAM_ULTRASONIC:
            break;
        default:
            return false;
    }
    Key key = GetStreamTypeKeyForMute(DeviceType::DEVICE_TYPE_SPEAKER, streamType);
    Status status = audioPolicyKvStore_->Get(key, value);
    if (status == Status::SUCCESS) {
        int volumeStatus = TransferByteArrayToType<int>(value.Data());
        muteStatusMap_[streamType] = volumeStatus;
        return true;
    }

    return false;
}

std::string AudioAdapterManager::GetStreamTypeKeyForMute(DeviceType deviceType, AudioStreamType streamType)
{
    std::string type = "";
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
            type = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            type = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
            type = "wired";
            break;
        default:
            AUDIO_ERR_LOG("GetStreamTypeKeyForMute device %{public}d is not supported for kv"
                "store", deviceType);
            return "";
    }

    switch (streamType) {
        case STREAM_MUSIC:
            return type + "music_mute_status";
        case STREAM_RING:
            return type + "ring_mute_status";
        case STREAM_SYSTEM:
            return type + "system_mute_status";
        case STREAM_NOTIFICATION:
            return type + "notification_mute_status";
        case STREAM_ALARM:
            return type + "alarm_mute_status";
        case STREAM_DTMF:
            return type + "dtmf_mute_status";
        case STREAM_VOICE_CALL:
            return type + "voice_call_mute_status";
        case STREAM_VOICE_ASSISTANT:
            return type + "voice_assistant_mute_status";
        case STREAM_ACCESSIBILITY:
            return type + "accessibility_mute_status";
        case STREAM_ULTRASONIC:
            return type + "unltrasonic_mute_status";
        default:
            return "";
    }
}

void AudioAdapterManager::WriteMuteStatusToKvStore(DeviceType deviceType, AudioStreamType streamType,
    bool muteStatus)
{
    if (audioPolicyKvStore_ == nullptr) {
        return;
    }

    Key key = GetStreamTypeKeyForMute(deviceType, streamType);
    Value value = Value(TransferTypeToByteArray<int>(muteStatus));

    Status status = audioPolicyKvStore_->Put(key, value);
    if (status == Status::SUCCESS) {
        AUDIO_INFO_LOG("muteStatus %{public}d for %{public}s updated to kvStore", muteStatus,
            GetStreamNameByStreamType(deviceType, streamType).c_str());
    } else {
        AUDIO_INFO_LOG("muteStatus %{public}d for %{public}s update to kvStore failed",
            muteStatus, GetStreamNameByStreamType(deviceType, streamType).c_str());
    }

    return;
}

float AudioAdapterManager::CalculateVolumeDb(int32_t volumeLevel)
{
    float value = static_cast<float>(volumeLevel) / MAX_VOLUME_LEVEL;
    float roundValue = static_cast<int>(value * CONST_FACTOR);

    return static_cast<float>(roundValue) / CONST_FACTOR;
}

int32_t AudioAdapterManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return WriteSystemSoundUriToKvStore(key, uri);
}

std::string AudioAdapterManager::GetSystemSoundUri(const std::string &key)
{
    return LoadSystemSoundUriFromKvStore(key);
}

int32_t AudioAdapterManager::WriteSystemSoundUriToKvStore(const std::string &key, const std::string &uri)
{
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("WriteSystemSoundUriToKvStore failed: audioPolicyKvStore_ is nullptr");
        return ERROR;
    }

    Status status = audioPolicyKvStore_->Put(key, uri);
    if (status == Status::SUCCESS) {
        AUDIO_INFO_LOG("WriteSystemSoundUriToKvStore: Wrote [%{public}s]: [%{public}s] to kvStore",
            key.c_str(), uri.c_str());
    } else {
        AUDIO_ERR_LOG("WriteSystemSoundUriToKvStore: Writing [%{public}s]: [%{public}s] to kvStore failed",
            key.c_str(), uri.c_str());
    }

    return SUCCESS;
}

std::string AudioAdapterManager::LoadSystemSoundUriFromKvStore(const std::string &key)
{
    std::string uri = "";
    if (audioPolicyKvStore_ == nullptr) {
        AUDIO_ERR_LOG("LoadSystemSoundUriFromKvStore failed: audioPolicyKvStore_ is nullptr");
        return uri;
    }
    Value value;
    Status status = audioPolicyKvStore_->Get(key, value);
    if (status == Status::SUCCESS) {
        uri = value.ToString();
    }
    AUDIO_INFO_LOG("LoadSystemSoundUriFromKvStore:: [%{public}s]: [%{public}s]", key.c_str(), uri.c_str());
    return uri;
}

float AudioAdapterManager::GetMinStreamVolume() const
{
    return MIN_STREAM_VOLUME;
}

float AudioAdapterManager::GetMaxStreamVolume() const
{
    return MAX_STREAM_VOLUME;
}
} // namespace AudioStandard
} // namespace OHOS
