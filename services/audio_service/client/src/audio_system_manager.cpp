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

#include "audio_system_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_interface.h"

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_manager_base.h"
#include "audio_manager_proxy.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_manager.h"
#include "audio_volume_key_event_callback_stub.h"
#include "audio_utils.h"
#include "audio_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::streamTypeMap_
    = AudioSystemManager::CreateStreamMap();
mutex g_asProxyMutex;
sptr<IStandardAudioService> g_asProxy = nullptr;

AudioSystemManager::AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager start");
}

AudioSystemManager::~AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager::~AudioSystemManager");
    if (cbClientId_ != -1) {
        UnsetRingerModeCallback(cbClientId_);
    }

    if (volumeChangeClientPid_ != -1) {
        AUDIO_DEBUG_LOG("AudioSystemManager::~AudioSystemManager UnregisterVolumeKeyEventCallback");
        (void)UnregisterVolumeKeyEventCallback(volumeChangeClientPid_);
    }
}

AudioSystemManager *AudioSystemManager::GetInstance()
{
    static AudioSystemManager audioManager;
    return &audioManager;
}

uint32_t AudioSystemManager::GetCallingPid()
{
    return getpid();
}

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::CreateStreamMap()
{
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    // Only use stream usage to choose stream type
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;

    return streamMap;
}

AudioStreamType AudioSystemManager::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    } else {
        AUDIO_ERR_LOG("The pair of contentType and streamUsage is not in design. Use the default stream type");
    }

    if (streamType == AudioStreamType::STREAM_MEDIA) {
        streamType = AudioStreamType::STREAM_MUSIC;
    }

    return streamType;
}

inline const sptr<IStandardAudioService> GetAudioSystemManagerProxy()
{
    lock_guard<mutex> lock(g_asProxyMutex);
    if (g_asProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            AUDIO_ERR_LOG("GetAudioSystemManagerProxy: get sa manager failed");
            return nullptr;
        }
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        if (object == nullptr) {
            AUDIO_ERR_LOG("GetAudioSystemManagerProxy: get audio service remote object failed");
            return nullptr;
        }
        g_asProxy = iface_cast<IStandardAudioService>(object);
        if (g_asProxy == nullptr) {
            AUDIO_ERR_LOG("GetAudioSystemManagerProxy: get audio service proxy failed");
            return nullptr;
        }

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient = new(std::nothrow) AudioServerDeathRecipient(getpid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb(std::bind(&AudioSystemManager::AudioServerDied,
                std::placeholders::_1));
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("GetAudioSystemManagerProxy: failed to add deathRecipient");
            }
        }
    }
    sptr<IStandardAudioService> gasp = g_asProxy;
    return gasp;
}

void AudioSystemManager::AudioServerDied(pid_t pid)
{
    AUDIO_INFO_LOG("audio server died, will restore proxy in next call");
    lock_guard<mutex> lock(g_asProxyMutex);
    g_asProxy = nullptr;
}

int32_t AudioSystemManager::SetRingerMode(AudioRingerMode ringMode)
{
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    ringModeBackup_ = ringMode;
    if (ringerModeCallback_ != nullptr) {
        ringerModeCallback_->OnRingerModeUpdated(ringModeBackup_);
    }

    return SUCCESS;
}

AudioRingerMode AudioSystemManager::GetRingerMode()
{
    return ringModeBackup_;
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    AUDIO_DEBUG_LOG("SetAudioScene audioScene_=%{public}d done", scene);
    return AudioPolicyManager::GetInstance().SetAudioScene(scene);
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    return AudioPolicyManager::GetInstance().GetAudioScene();
}

int32_t AudioSystemManager::SetDeviceActive(ActiveDeviceType deviceType, bool flag) const
{
    AUDIO_INFO_LOG("SetDeviceActive device: %{public}d", deviceType);
    switch (deviceType) {
        case EARPIECE:
        case SPEAKER:
        case BLUETOOTH_SCO:
        case FILE_SINK_DEVICE:
            break;
        default:
            AUDIO_ERR_LOG("SetDeviceActive device=%{public}d not supported", deviceType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetDeviceActive */
    return (AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag));
}

bool AudioSystemManager::IsDeviceActive(ActiveDeviceType deviceType) const
{
    switch (deviceType) {
        case EARPIECE:
        case SPEAKER:
        case BLUETOOTH_SCO:
        case FILE_SINK_DEVICE:
            break;
        default:
            AUDIO_ERR_LOG("IsDeviceActive device=%{public}d not supported", deviceType);
            return false;
    }

    /* Call Audio Policy IsDeviceActive */
    return (AudioPolicyManager::GetInstance().IsDeviceActive(static_cast<InternalDeviceType>(deviceType)));
}

DeviceType AudioSystemManager::GetActiveOutputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveOutputDevice();
}

DeviceType AudioSystemManager::GetActiveInputDevice()
{
    return AudioPolicyManager::GetInstance().GetActiveInputDevice();
}

bool AudioSystemManager::IsStreamActive(AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        case STREAM_ULTRASONIC:
            if (!PermissionUtil::VerifySelfPermission()) {
                AUDIO_ERR_LOG("IsStreamActive: volumeType=%{public}d. No system permission", volumeType);
                return false;
            }
            break;
        case STREAM_ALL:
        default:
            AUDIO_ERR_LOG("IsStreamActive: volumeType=%{public}d not supported", volumeType);
            return false;
    }

    return AudioPolicyManager::GetInstance().IsStreamActive(volumeType);
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("GetAudioParameter::Audio service unavailable.");
        return "";
    }
    return gasp->GetAudioParameter(key);
}

void AudioSystemManager::SetAudioParameter(const std::string &key, const std::string &value)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("SetAudioParameter::Audio service unavailable.");
        return;
    }
    gasp->SetAudioParameter(key, value);
}

const char *AudioSystemManager::RetrieveCookie(int32_t &size)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("RetrieveCookie::Audio service unavailable.");
        return nullptr;
    }
    return gasp->RetrieveCookie(size);
}

uint64_t AudioSystemManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("GetTransactionId::Audio service unavailable.");
        return 0;
    }
    return gasp->GetTransactionId(deviceType, deviceRole);
}

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel) const
{
    AUDIO_DEBUG_LOG("SetVolume volumeType[%{public}d], volumeLevel[%{public}d]", volumeType, volumeLevel);

    /* Validate volumeType and return INVALID_PARAMS error */
    switch (volumeType) {
        case STREAM_VOICE_CALL:
        case STREAM_RING:
        case STREAM_MUSIC:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
        case STREAM_VOICE_ASSISTANT:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
            if (!PermissionUtil::VerifySelfPermission()) {
                AUDIO_ERR_LOG("SetVolume: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            AUDIO_ERR_LOG("SetVolume: volumeType[%{public}d] is not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetSystemVolumeLevel */
    return AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_7);
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
            if (!PermissionUtil::VerifySelfPermission()) {
                AUDIO_ERR_LOG("SetMute: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            AUDIO_ERR_LOG("GetVolume volumeType = %{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    return AudioPolicyManager::GetInstance().GetSystemVolumeLevel(volumeType);
}

int32_t AudioSystemManager::SetLowPowerVolume(int32_t streamId, float volume) const
{
    AUDIO_INFO_LOG("AudioSystemManager SetLowPowerVolume, streamId:%{public}d, vol:%{public}f.", streamId, volume);
    if ((volume < 0) || (volume > 1.0)) {
        AUDIO_ERR_LOG("Invalid Volume Input!");
        return ERR_INVALID_PARAM;
    }

    return AudioPolicyManager::GetInstance().SetLowPowerVolume(streamId, volume);
}

float AudioSystemManager::GetLowPowerVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetLowPowerVolume(streamId);
}

float AudioSystemManager::GetSingleStreamVolume(int32_t streamId) const
{
    return AudioPolicyManager::GetInstance().GetSingleStreamVolume(streamId);
}

int32_t AudioSystemManager::GetMaxVolume(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL) {
        if (!PermissionUtil::VerifySelfPermission()) {
            AUDIO_ERR_LOG("GetMaxVolume: No system permission");
            return ERR_PERMISSION_DENIED;
        }
    }

    if (volumeType == STREAM_ULTRASONIC) {
        if (!PermissionUtil::VerifySelfPermission()) {
            AUDIO_ERR_LOG("GetMaxVolume: STREAM_ULTRASONIC No system permission");
            return ERR_PERMISSION_DENIED;
        }
    }

    return AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
}

int32_t AudioSystemManager::GetMinVolume(AudioVolumeType volumeType)
{
    if (volumeType == STREAM_ALL) {
        if (!PermissionUtil::VerifySelfPermission()) {
            AUDIO_ERR_LOG("GetMinVolume: No system permission");
            return ERR_PERMISSION_DENIED;
        }
    }

    if (volumeType == STREAM_ULTRASONIC) {
        if (!PermissionUtil::VerifySelfPermission()) {
            AUDIO_ERR_LOG("GetMinVolume: STREAM_ULTRASONIC No system permission");
            return ERR_PERMISSION_DENIED;
        }
    }

    return AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
}

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute) const
{
    AUDIO_DEBUG_LOG("AudioSystemManager SetMute for volumeType=%{public}d", volumeType);
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
            if (!PermissionUtil::VerifySelfPermission()) {
                AUDIO_ERR_LOG("SetMute: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            AUDIO_ERR_LOG("SetMute volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetStreamMute */
    return AudioPolicyManager::GetInstance().SetStreamMute(volumeType, mute, API_7);
}

bool AudioSystemManager::IsStreamMute(AudioVolumeType volumeType) const
{
    AUDIO_DEBUG_LOG("AudioSystemManager::GetMute Client");

    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
            if (!PermissionUtil::VerifySelfPermission()) {
                AUDIO_ERR_LOG("IsStreamMute: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            AUDIO_ERR_LOG("IsStreamMute volumeType=%{public}d not supported", volumeType);
            return false;
    }

    return AudioPolicyManager::GetInstance().GetStreamMute(volumeType);
}

int32_t AudioSystemManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetDeviceChangeCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    int32_t clientId = static_cast<int32_t>(GetCallingPid());
    return AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
}

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    int32_t clientId = static_cast<int32_t>(GetCallingPid());
    return AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId, flag);
}

int32_t AudioSystemManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    std::lock_guard<std::mutex> lockSet(ringerModeCallbackMutex_);
    if (!PermissionUtil::VerifySelfPermission()) {
        AUDIO_ERR_LOG("SetRingerModeCallback: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    if (callback == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    cbClientId_ = clientId;
    ringerModeCallback_ = callback;

    return SUCCESS;
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    if (clientId != cbClientId_) {
        return ERR_INVALID_OPERATION;
    }

    return SUCCESS;
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute)
{
    return AudioPolicyManager::GetInstance().SetMicrophoneMute(isMute);
}

bool AudioSystemManager::IsMicrophoneMute(API_VERSION api_v)
{
    std::shared_ptr<AudioGroupManager> groupManager = GetGroupManager(DEFAULT_VOLUME_GROUP_ID);
    if (groupManager == nullptr) {
        AUDIO_ERR_LOG("IsMicrophoneMute failed, groupManager is null");
        return false;
    }
    return groupManager->IsMicrophoneMute(api_v);
}

int32_t AudioSystemManager::SelectOutputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    if (audioDeviceDescriptors.size() != 1 || audioDeviceDescriptors[0] == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid parameter");
        return ERR_INVALID_PARAM;
    }
    if (audioDeviceDescriptors[0]->deviceRole_ != DeviceRole::OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("SelectOutputDevice: not an output device.");
        return ERR_INVALID_OPERATION;
    }
    size_t validSize = 64;
    if (audioDeviceDescriptors[0]->networkId_ != LOCAL_NETWORK_ID &&
        audioDeviceDescriptors[0]->networkId_.size() != validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid networkId.");
        return ERR_INVALID_PARAM;
    }
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    return ret;
}

int32_t AudioSystemManager::SelectInputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    if (audioDeviceDescriptors.size() != 1 || audioDeviceDescriptors[0] == nullptr) {
        AUDIO_ERR_LOG("SelectInputDevice: invalid parameter");
        return ERR_INVALID_PARAM;
    }
    if (audioDeviceDescriptors[0]->deviceRole_ != DeviceRole::INPUT_DEVICE) {
        AUDIO_ERR_LOG("SelectInputDevice: not an output device.");
        return ERR_INVALID_OPERATION;
    }
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

std::string AudioSystemManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const
{
    return AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioSystemManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    // basic check
    if (audioRendererFilter == nullptr || audioDeviceDescriptors.size() == 0) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid parameter");
        return ERR_INVALID_PARAM;
    }

    size_t validDeviceSize = 1;
    if (audioDeviceDescriptors.size() > validDeviceSize || audioDeviceDescriptors[0] == nullptr) {
        AUDIO_ERR_LOG("SelectOutputDevice: device error");
        return ERR_INVALID_OPERATION;
    }
    audioRendererFilter->streamType = AudioSystemManager::GetStreamType(audioRendererFilter->rendererInfo.contentType,
        audioRendererFilter->rendererInfo.streamUsage);
    // operation chack
    if (audioDeviceDescriptors[0]->deviceRole_ != DeviceRole::OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("SelectOutputDevice: not an output device.");
        return ERR_INVALID_OPERATION;
    }
    size_t validSize = 64;
    if (audioDeviceDescriptors[0]->networkId_ != LOCAL_NETWORK_ID &&
        audioDeviceDescriptors[0]->networkId_.size() != validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid networkId.");
        return ERR_INVALID_PARAM;
    }
    if (audioRendererFilter->uid < 0) {
        AUDIO_ERR_LOG("SelectOutputDevice: invalid uid.");
        return ERR_INVALID_PARAM;
    }
    AUDIO_DEBUG_LOG("[%{public}d] SelectOutputDevice: uid<%{public}d> streamType<%{public}d> device<name:%{public}s>",
        getpid(), audioRendererFilter->uid, static_cast<int32_t>(audioRendererFilter->streamType),
        (audioDeviceDescriptors[0]->networkId_.c_str()));

    return AudioPolicyManager::GetInstance().SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    // basic check
    if (audioCapturerFilter == nullptr || audioDeviceDescriptors.size() == 0) {
        AUDIO_ERR_LOG("SelectInputDevice: invalid parameter");
        return ERR_INVALID_PARAM;
    }

    size_t validDeviceSize = 1;
    if (audioDeviceDescriptors.size() > validDeviceSize || audioDeviceDescriptors[0] == nullptr) {
        AUDIO_ERR_LOG("SelectInputDevice: device error.");
        return ERR_INVALID_OPERATION;
    }
    // operation chack
    if (audioDeviceDescriptors[0]->deviceRole_ != DeviceRole::INPUT_DEVICE) {
        AUDIO_ERR_LOG("SelectInputDevice: not an input device");
        return ERR_INVALID_OPERATION;
    }
    if (audioCapturerFilter->uid < 0) {
        AUDIO_ERR_LOG("SelectInputDevice: invalid uid.");
        return ERR_INVALID_PARAM;
    }
    AUDIO_DEBUG_LOG("[%{public}d] SelectInputDevice: uid<%{public}d> device<type:%{public}d>",
        getpid(), audioCapturerFilter->uid, static_cast<int32_t>(audioDeviceDescriptors[0]->deviceType_));

    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
{
    return AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
}

std::vector<sptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    AudioRendererInfo rendererInfo;
    return AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
}

int32_t AudioSystemManager::GetPreferredInputDeviceDescriptors()
{
    AudioCapturerInfo capturerInfo;
    auto dec = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    if (dec.size() <= 0) {
        return ERROR_INVALID_PARAM;
    }
    return SUCCESS;
}


int32_t AudioSystemManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    return AudioPolicyManager::GetInstance().GetAudioFocusInfoList(focusInfoList);
}

int32_t AudioSystemManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::callback is null");
        return ERR_INVALID_PARAM;
    }

    int32_t clientId = GetCallingPid();
    AUDIO_DEBUG_LOG("RegisterFocusInfoChangeCallback clientId:%{public}d", clientId);
    if (audioFocusInfoCallback_ == nullptr) {
        audioFocusInfoCallback_ = std::make_shared<AudioFocusInfoChangeCallbackImpl>();
        if (audioFocusInfoCallback_ == nullptr) {
            AUDIO_ERR_LOG("AudioSystemManager::Failed to allocate memory for audioInterruptCallback");
            return ERROR;
        }
        int32_t ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId,
            audioFocusInfoCallback_);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("AudioSystemManager::Failed set callback");
            return ERROR;
        }
    }

    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    if (cbFocusInfo == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::cbFocusInfo is nullptr");
        return ERROR;
    }
    cbFocusInfo->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioSystemManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback> &callback)
{
    int32_t clientId = GetCallingPid();
    int32_t ret = 0;

    if (callback == nullptr) {
        ret = AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
        audioFocusInfoCallback_.reset();
        audioFocusInfoCallback_ = nullptr;
        if (!ret) {
            AUDIO_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback success");
        }
        return ret;
    }
    if (audioFocusInfoCallback_ == nullptr) {
        AUDIO_ERR_LOG("Failed to allocate memory for audioInterruptCallback");
        return ERROR;
    }
    std::shared_ptr<AudioFocusInfoChangeCallbackImpl> cbFocusInfo =
        std::static_pointer_cast<AudioFocusInfoChangeCallbackImpl>(audioFocusInfoCallback_);
    cbFocusInfo->RemoveCallback(callback);

    return ret;
}

AudioFocusInfoChangeCallbackImpl::AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl constructor");
}

AudioFocusInfoChangeCallbackImpl::~AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl: destroy");
}

void AudioFocusInfoChangeCallbackImpl::SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    bool hasCallback = false;
    for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
        if ((*it).lock() == callback.lock()) {
            hasCallback = true;
        }
    }
    if (!hasCallback) {
        callbackList_.push_back(callback);
    }
}

void AudioFocusInfoChangeCallbackImpl::RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    callbackList_.remove_if([&callback](std::weak_ptr<AudioFocusInfoChangeCallback> &callback_) {
        return callback_.lock() == callback.lock();
    });
}

void AudioFocusInfoChangeCallbackImpl::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("on callback Entered AudioFocusInfoChangeCallbackImpl %{public}s", __func__);

    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
        cb_ = (*callback).lock();
        if (cb_ != nullptr) {
            AUDIO_DEBUG_LOG("OnAudioFocusInfoChange : Notify event to app complete");
            cb_->OnAudioFocusInfoChange(focusInfoList);
        } else {
            AUDIO_ERR_LOG("OnAudioFocusInfoChange: callback is null");
        }
    }
    return;
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    AUDIO_DEBUG_LOG("AudioSystemManager RegisterVolumeKeyEventCallback");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::RegisterVolumeKeyEventCallbackcallback is nullptr");
        return ERR_INVALID_PARAM;
    }
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback, api_v);
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid)
{
    AUDIO_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(clientPid);
    if (!ret) {
        AUDIO_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback success");
        volumeChangeClientPid_ = -1;
    }

    return ret;
}

void AudioSystemManager::SetAudioMonoState(bool monoState)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("SetAudioMonoState::Audio service unavailable.");
        return;
    }
    gasp->SetAudioMonoState(monoState);
}

void AudioSystemManager::SetAudioBalanceValue(float balanceValue)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("SetAudioBalanceValue::Audio service unavailable.");
        return;
    }
    gasp->SetAudioBalanceValue(balanceValue);
}

int32_t AudioSystemManager::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    return AudioPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
}

std::string AudioSystemManager::GetSystemSoundUri(const std::string &key)
{
    return AudioPolicyManager::GetInstance().GetSystemSoundUri(key);
}

// Below stub implementation is added to handle compilation error in call manager
// Once call manager adapt to new interrupt implementation, this will be removed
int32_t AudioSystemManager::SetAudioManagerCallback(const AudioVolumeType streamType,
                                                    const std::shared_ptr<AudioManagerCallback> &callback)
{
    AUDIO_DEBUG_LOG("AudioSystemManager SetAudioManagerCallback stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioVolumeType streamType) const
{
    AUDIO_DEBUG_LOG("AudioSystemManager UnsetAudioManagerCallback stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    AUDIO_DEBUG_LOG("AudioSystemManager ActivateAudioInterrupt stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    AUDIO_DEBUG_LOG("AudioSystemManager DeactivateAudioInterrupt stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("SetAudioManagerInterruptCallback client id: %{public}d", clientId);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::callback is null");
        return ERR_INVALID_PARAM;
    }

    if (audioInterruptCallback_ != nullptr) {
        callback->cbMutex_.lock();
        AUDIO_DEBUG_LOG("AudioSystemManager reset existing callback object");
        AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
        callback->cbMutex_.unlock();
    }

    audioInterruptCallback_ = std::make_shared<AudioManagerInterruptCallbackImpl>();
    if (audioInterruptCallback_ == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::Failed to allocate memory for audioInterruptCallback");
        return ERROR;
    }

    int32_t ret = AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, audioInterruptCallback_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioSystemManager::Failed set callback");
        return ERROR;
    }

    std::shared_ptr<AudioManagerInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioManagerInterruptCallbackImpl>(audioInterruptCallback_);
    if (cbInterrupt == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager::cbInterrupt is nullptr");
        return ERROR;
    }
    cbInterrupt->SaveCallback(callback);

    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerInterruptCallback()
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("UnsetAudioManagerInterruptCallback client id: %{public}d", clientId);

    int32_t ret = AudioPolicyManager::GetInstance().UnsetAudioManagerInterruptCallback(clientId);
    if (audioInterruptCallback_ != nullptr) {
        audioInterruptCallback_.reset();
        audioInterruptCallback_ = nullptr;
    }

    return ret;
}

int32_t AudioSystemManager::RequestAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("RequestAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().RequestAudioFocus(clientId, audioInterrupt);
}

int32_t AudioSystemManager::AbandonAudioFocus(const AudioInterrupt &audioInterrupt)
{
    int32_t clientId = GetCallingPid();
    AUDIO_INFO_LOG("AbandonAudioFocus client id: %{public}d", clientId);
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.contentType >= CONTENT_TYPE_UNKNOWN &&
        audioInterrupt.contentType <= CONTENT_TYPE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid content type");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.streamUsage >= STREAM_USAGE_UNKNOWN &&
        audioInterrupt.streamUsage <= STREAM_USAGE_ULTRASONIC, ERR_INVALID_PARAM, "Invalid stream usage");
    CHECK_AND_RETURN_RET_LOG(audioInterrupt.audioFocusType.streamType >= STREAM_VOICE_CALL &&
        audioInterrupt.audioFocusType.streamType <= STREAM_TYPE_MAX, ERR_INVALID_PARAM, "Invalid stream type");
    return AudioPolicyManager::GetInstance().AbandonAudioFocus(clientId, audioInterrupt);
}

int32_t AudioSystemManager::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
    return AudioPolicyManager::GetInstance().ReconfigureAudioChannel(count, deviceType);
}

int32_t AudioSystemManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    return AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
}

std::shared_ptr<AudioGroupManager> AudioSystemManager::GetGroupManager(int32_t groupId)
{
    std::vector<std::shared_ptr<AudioGroupManager>>::iterator iter = groupManagerMap_.begin();
    while (iter != groupManagerMap_.end()) {
        if ((*iter)->GetGroupId() == groupId) {
            return *iter;
        } else {
            iter++;
        }
    }

    std::shared_ptr<AudioGroupManager> groupManager = std::make_shared<AudioGroupManager>(groupId);
    if (groupManager->Init() == SUCCESS) {
        groupManagerMap_.push_back(groupManager);
    } else {
        groupManager = nullptr;
    }
    return groupManager;
}

AudioManagerInterruptCallbackImpl::AudioManagerInterruptCallbackImpl()
{
    AUDIO_INFO_LOG("AudioManagerInterruptCallbackImpl constructor");
}

AudioManagerInterruptCallbackImpl::~AudioManagerInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioManagerInterruptCallbackImpl: instance destroy");
}

void AudioManagerInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback)
{
    auto wp = callback.lock();
    if (wp != nullptr) {
        callback_ = callback;
    } else {
        AUDIO_ERR_LOG("AudioManagerInterruptCallbackImpl::SaveCallback: callback is nullptr");
    }
}

void AudioManagerInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    cb_ = callback_.lock();
    if (cb_ != nullptr) {
        cb_->cbMutex_.lock();
        InterruptAction interruptAction = {};
        interruptAction.actionType = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN)
            ? TYPE_INTERRUPT : TYPE_ACTIVATED;
        interruptAction.interruptType = interruptEvent.eventType;
        interruptAction.interruptHint = interruptEvent.hintType;
        interruptAction.activated = (interruptEvent.eventType == INTERRUPT_TYPE_BEGIN) ? false : true;
        cb_->OnInterrupt(interruptAction);
        AUDIO_DEBUG_LOG("AudioManagerInterruptCallbackImpl: OnInterrupt : Notify event to app complete");
        cb_->cbMutex_.unlock();
    } else {
        AUDIO_ERR_LOG("AudioManagerInterruptCallbackImpl: callback is null");
    }

    return;
}

bool AudioSystemManager::RequestIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("RequestIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = GetCallingPid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.sessionID = clientId;
    int32_t result = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);

    AUDIO_DEBUG_LOG("RequestIndependentInterrupt : result -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}
bool AudioSystemManager::AbandonIndependentInterrupt(FocusType focusType)
{
    AUDIO_INFO_LOG("AbandonIndependentInterrupt : foncusType");
    AudioInterrupt audioInterrupt;
    int32_t clientId = GetCallingPid();
    audioInterrupt.contentType = ContentType::CONTENT_TYPE_SPEECH;
    audioInterrupt.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    audioInterrupt.audioFocusType.streamType = AudioStreamType::STREAM_RECORDING;
    audioInterrupt.sessionID = clientId;
    int32_t result = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
    AUDIO_DEBUG_LOG("AbandonIndependentInterrupt : result -> %{public}d", result);
    return (result == SUCCESS) ? true:false;
}

int32_t AudioSystemManager::GetAudioLatencyFromXml() const
{
    return AudioPolicyManager::GetInstance().GetAudioLatencyFromXml();
}

uint32_t AudioSystemManager::GetSinkLatencyFromXml() const
{
    return AudioPolicyManager::GetInstance().GetSinkLatencyFromXml();
}

int32_t AudioSystemManager::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, AudioStreamType audioStreamType)
{
    AUDIO_INFO_LOG("UpdateStreamState::clientUid:%{public}d streamSetState:%{public}d",
        clientUid, streamSetState);
    int32_t result = 0;
    
    result = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid, streamSetState, audioStreamType);
    return result;
}

std::string AudioSystemManager::GetSelfBundleName()
{
    std::string bundleName = "";

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    sptr<AppExecFwk::IBundleMgr> iBundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        AUDIO_ERR_LOG("bundlemgr interface is null");
        return bundleName;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (iBundleMgr->GetBundleInfoForSelf(0, bundleInfo) == ERR_OK) {
        bundleName = bundleInfo.name;
    } else {
        AUDIO_ERR_LOG("Get bundle info failed");
    }
    return bundleName;
}

int32_t AudioSystemManager::OffloadDrain()
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("OffloadDrain Audio service unavailable.");
        return ERR_INVALID_PARAM;
    }
    return gasp->OffloadDrain();
}

int32_t AudioSystemManager::OffloadGetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("OffloadGetPresentationPosition Audio service unavailable.");
        return ERR_INVALID_PARAM;
    }
    return gasp->OffloadGetPresentationPosition(frames, timeSec, timeNanoSec);
}

int32_t AudioSystemManager::OffloadSetBufferSize(uint32_t sizeMs)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("OffloadSetBufferSize Audio service unavailable.");
        return ERR_INVALID_PARAM;
    }
    return gasp->OffloadSetBufferSize(sizeMs);
}

void AudioSystemManager::RequestThreadPriority(uint32_t tid)
{
    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("RequestThreadPriority Audio service unavailable.");
        return;
    }
    std::string bundleName = GetSelfBundleName();
    gasp->RequestThreadPriority(tid, bundleName);
}

int32_t AudioSystemManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    AUDIO_INFO_LOG("AudioSystemManager::SetDeviceAbsVolumeSupported");
    return AudioPolicyManager::GetInstance().SetDeviceAbsVolumeSupported(macAddress, support);
}

int32_t AudioSystemManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    AUDIO_INFO_LOG("AudioSystemManager::SetA2dpDeviceVolume");
    return AudioPolicyManager::GetInstance().SetA2dpDeviceVolume(macAddress, volume, updateUi);
}

AudioPin AudioSystemManager::GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const
{
    AudioPin pin = AUDIO_PIN_NONE;
    switch (deviceType) {
        case OHOS::AudioStandard::DEVICE_TYPE_NONE:
        case OHOS::AudioStandard::DEVICE_TYPE_INVALID:
            pin = AUDIO_PIN_NONE;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_DEFAULT:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_DAUDIO_DEFAULT;
            } else {
                pin = AUDIO_PIN_OUT_DAUDIO_DEFAULT;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_SPEAKER:
            pin = AUDIO_PIN_OUT_SPEAKER;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_MIC:
        case OHOS::AudioStandard::DEVICE_TYPE_WAKEUP:
            pin = AUDIO_PIN_IN_MIC;
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_WIRED_HEADSET:
            if (deviceRole == DeviceRole::INPUT_DEVICE) {
                pin = AUDIO_PIN_IN_HS_MIC;
            } else {
                pin = AUDIO_PIN_OUT_HEADSET;
            }
            break;
        case OHOS::AudioStandard::DEVICE_TYPE_USB_HEADSET:
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SINK:
        case OHOS::AudioStandard::DEVICE_TYPE_FILE_SOURCE:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_SCO:
        case OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP:
        case OHOS::AudioStandard::DEVICE_TYPE_MAX:
            AUDIO_INFO_LOG("GetPinValueFromType :don't supported the device type");
            break;
        default:
            AUDIO_INFO_LOG("GetPinValueFromType : invalid input parameter");
            break;
    }
    return pin;
}

DeviceType AudioSystemManager::GetTypeValueFromPin(AudioPin pin) const
{
    DeviceType type = DEVICE_TYPE_NONE;
    switch (pin) {
        case OHOS::AudioStandard::AUDIO_PIN_NONE:
            type = DEVICE_TYPE_NONE;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_SPEAKER:
            type = DEVICE_TYPE_SPEAKER;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HEADSET:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_LINEOUT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_HDMI:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_OUT_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_MIC:
            type = DEVICE_TYPE_MIC;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_HS_MIC:
            type = DEVICE_TYPE_WIRED_HEADSET;
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_LINEIN:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_USB_EXT:
            break;
        case OHOS::AudioStandard::AUDIO_PIN_IN_DAUDIO_DEFAULT:
            type = DEVICE_TYPE_DEFAULT;
            break;
        default:
            AUDIO_INFO_LOG("GetTypeValueFromPin : invalid input parameter");
            break;
    }
    return type;
}

int32_t AudioSystemManager::RegisterWakeupSourceCallback()
{
    AUDIO_INFO_LOG("RegisterWakeupSourceCallback");
    remoteWakeUpCallback_ = std::make_shared<WakeUpCallbackImpl>(this);

    auto wakeupCloseCbStub = new(std::nothrow) AudioManagerListenerStub();
    if (wakeupCloseCbStub == nullptr) {
        AUDIO_ERR_LOG("wakeupCloseCbStub is null");
        return ERROR;
    }
    wakeupCloseCbStub->SetWakeupSourceCallback(remoteWakeUpCallback_);

    const sptr<IStandardAudioService> gasp = GetAudioSystemManagerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("GetAudioParameter::Audio service unavailable.");
        return ERROR;
    }

    sptr<IRemoteObject> object = wakeupCloseCbStub->AsObject();
    if (object == nullptr) {
        AUDIO_ERR_LOG("SetWakeupCloseCallback listenerStub object is nullptr");
        delete wakeupCloseCbStub;
        return ERROR;
    }
    return gasp->SetWakeupSourceCallback(object);
}

int32_t AudioSystemManager::SetAudioCapturerSourceCallback(const std::shared_ptr<AudioCapturerSourceCallback> &callback)
{
    audioCapturerSourceCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

int32_t AudioSystemManager::SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback)
{
    audioWakeUpSourceCloseCallback_ = callback;
    return RegisterWakeupSourceCallback();
}

int32_t AudioSystemManager::SetAvailableDeviceChangeCallback(const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetAvailableDeviceChangeCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    int32_t clientId = static_cast<int32_t>(GetCallingPid());
    return AudioPolicyManager::GetInstance().SetAvailableDeviceChangeCallback(clientId, usage, callback);
}

int32_t AudioSystemManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    int32_t clientId = static_cast<int32_t>(GetCallingPid());
    return AudioPolicyManager::GetInstance().UnsetAvailableDeviceChangeCallback(clientId, usage);
}
} // namespace AudioStandard
} // namespace OHOS
