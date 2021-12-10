/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "media_log.h"
#include "system_ability_definition.h"
#include "audio_errors.h"
#include "audio_manager_proxy.h"
#include "audio_stream.h"
#include "audio_system_manager.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
static sptr<IStandardAudioService> g_sProxy = nullptr;

AudioSystemManager::AudioSystemManager()
{
    MEDIA_DEBUG_LOG("AudioSystemManager start");
    init();
}

AudioSystemManager::~AudioSystemManager()
{
    // Deactivate to be called after unset in destructor
    UnsetAudioManagerCallback(cbStreamType_);
    DeactivateAudioInterrupt(audioInterrupt_);
    MEDIA_DEBUG_LOG("AudioSystemManager::~AudioSystemManager");
}

AudioSystemManager *AudioSystemManager::GetInstance()
{
    static AudioSystemManager audioManager;
    return &audioManager;
}

void AudioSystemManager::init()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager::init failed");
        return;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    if (object == nullptr) {
        MEDIA_DEBUG_LOG("AudioSystemManager::object is NULL.");
    }
    g_sProxy = iface_cast<IStandardAudioService>(object);
    if (g_sProxy == nullptr) {
        MEDIA_DEBUG_LOG("AudioSystemManager::init g_sProxy is NULL.");
    } else {
        MEDIA_DEBUG_LOG("AudioSystemManager::init g_sProxy is assigned.");
    }
}

bool AudioSystemManager::SetRingerMode(AudioRingerMode ringMode) const
{
    /* Call Audio Policy SetRingerMode */
    return AudioPolicyManager::GetInstance().SetRingerMode(ringMode);
}

AudioRingerMode AudioSystemManager::GetRingerMode() const
{
    /* Call Audio Policy GetRingerMode */
    return (AudioPolicyManager::GetInstance().GetRingerMode());
}

int32_t AudioSystemManager::SetDeviceActive(ActiveDeviceType deviceType, bool flag) const
{
    switch (deviceType) {
        case SPEAKER:
        case BLUETOOTH_SCO:
            break;
        default:
            MEDIA_ERR_LOG("SetDeviceActive device=%{public}d not supported", deviceType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetDeviceActive */
    return (AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag));
}

bool AudioSystemManager::IsDeviceActive(ActiveDeviceType deviceType) const
{
    switch (deviceType) {
        case SPEAKER:
        case BLUETOOTH_SCO:
            break;
        default:
            MEDIA_ERR_LOG("IsDeviceActive device=%{public}d not supported", deviceType);
            return false;
    }

    /* Call Audio Policy IsDeviceActive */
    return (AudioPolicyManager::GetInstance().IsDeviceActive(static_cast<InternalDeviceType>(deviceType)));
}

bool AudioSystemManager::IsStreamActive(AudioSystemManager::AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
            break;
        default:
            MEDIA_ERR_LOG("IsStreamActive volumeType=%{public}d not supported", volumeType);
            return false;
    }

    AudioStreamType StreamVolType = (AudioStreamType)volumeType;
    return AudioPolicyManager::GetInstance().IsStreamActive(StreamVolType);
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key) const
{
    return g_sProxy->GetAudioParameter(key);
}

void AudioSystemManager::SetAudioParameter(const std::string key, const std::string value) const
{
    g_sProxy->SetAudioParameter(key, value);
}

int32_t AudioSystemManager::SetVolume(AudioSystemManager::AudioVolumeType volumeType, int32_t volume) const
{
    /* Validate and return INVALID_PARAMS error */
    if ((volume < MIN_VOLUME_LEVEL) || (volume > MAX_VOLUME_LEVEL)) {
        MEDIA_ERR_LOG("Invalid Volume Input!");
        return ERR_INVALID_PARAM;
    }

    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
            break;
        default:
            MEDIA_ERR_LOG("SetVolume volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetStreamVolume */
    AudioStreamType StreamVolType = (AudioStreamType)volumeType;
    float volumeToHdi = MapVolumeToHDI(volume);
    return AudioPolicyManager::GetInstance().SetStreamVolume(StreamVolType, volumeToHdi);
}

int32_t AudioSystemManager::GetVolume(AudioSystemManager::AudioVolumeType volumeType) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
            break;
        default:
            MEDIA_ERR_LOG("GetVolume volumeType=%{public}d not supported", volumeType);
            return (float)ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetStreamMute */
    AudioStreamType StreamVolType = (AudioStreamType)volumeType;
    float volumeFromHdi = AudioPolicyManager::GetInstance().GetStreamVolume(StreamVolType);

    return MapVolumeFromHDI(volumeFromHdi);
}

float AudioSystemManager::MapVolumeToHDI(int32_t volume)
{
    float value = (float)volume / MAX_VOLUME_LEVEL;
    float roundValue = (int)(value * CONST_FACTOR);

    return (float)roundValue / CONST_FACTOR;
}

int32_t AudioSystemManager::MapVolumeFromHDI(float volume)
{
    float value = (float)volume * MAX_VOLUME_LEVEL;
    return nearbyint(value);
}

int32_t AudioSystemManager::GetMaxVolume(AudioSystemManager::AudioVolumeType volumeType) const
{
    return g_sProxy->GetMaxVolume(volumeType);
}

int32_t AudioSystemManager::GetMinVolume(AudioSystemManager::AudioVolumeType volumeType) const
{
    return g_sProxy->GetMinVolume(volumeType);
}

int32_t AudioSystemManager::SetMute(AudioSystemManager::AudioVolumeType volumeType, bool mute) const
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
            break;
        default:
            MEDIA_ERR_LOG("SetMute volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }

    /* Call Audio Policy SetStreamMute */
    AudioStreamType StreamVolType = (AudioStreamType)volumeType;
    return AudioPolicyManager::GetInstance().SetStreamMute(StreamVolType, mute);
}

bool AudioSystemManager::IsStreamMute(AudioSystemManager::AudioVolumeType volumeType) const
{
    MEDIA_DEBUG_LOG("AudioSystemManager::GetMute Client");

    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
            break;
        default:
            MEDIA_ERR_LOG("IsStreamMute volumeType=%{public}d not supported", volumeType);
            return false;
    }

    /* Call Audio Policy SetStreamVolume */
    AudioStreamType StreamVolType = (AudioStreamType)volumeType;
    return AudioPolicyManager::GetInstance().GetStreamMute(StreamVolType);
}

int32_t AudioSystemManager::SetAudioManagerCallback(const AudioSystemManager::AudioVolumeType streamType,
                                                    const std::shared_ptr<AudioManagerCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }
    callback_ = callback;
    cbStreamType_ = streamType;

    return AudioPolicyManager::GetInstance().SetAudioManagerCallback(static_cast<AudioStreamType>(streamType),
                                                                     callback);
}

int32_t AudioSystemManager::SetDeviceChangeCallback(const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }
    deviceChangeCallback_ = callback;

    MEDIA_ERR_LOG("AudioSystemManager: SetDeviceChangeCallback set deviceChangeCallback_ ok");

    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioSystemManager::AudioVolumeType streamType) const
{
    return AudioPolicyManager::GetInstance().UnsetAudioManagerCallback(static_cast<AudioStreamType>(streamType));
}

int32_t AudioSystemManager::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    audioInterrupt_.streamUsage = audioInterrupt.streamUsage;
    audioInterrupt_.contentType = audioInterrupt.contentType;
    audioInterrupt_.streamType = audioInterrupt.streamType;
    audioInterrupt_.sessionID = audioInterrupt.sessionID;

    return AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    return AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute) const
{
    return g_sProxy->SetMicrophoneMute(isMute);
}

bool AudioSystemManager::IsMicrophoneMute() const
{
    return g_sProxy->IsMicrophoneMute();
}

std::vector<sptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
    const
{
    return g_sProxy->GetDevices(deviceFlag);
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    audioScene_ = scene;
    MEDIA_DEBUG_LOG("SetAudioScene audioScene_=%{public}d done", audioScene_);
    return SUCCESS;
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    MEDIA_DEBUG_LOG("GetAudioScene audioScene_=%{public}d done", audioScene_);
    return audioScene_;
}

/**
 * @brief The AudioDeviceDescriptor provides
 *         different sets of audio devices and their roles
 */
AudioDeviceDescriptor::AudioDeviceDescriptor()
{
    MEDIA_DEBUG_LOG("AudioDeviceDescriptor constructor");
    deviceType_ = DEVICE_TYPE_NONE;
    deviceRole_ = DEVICE_ROLE_NONE;
}

AudioDeviceDescriptor::~AudioDeviceDescriptor()
{
    MEDIA_DEBUG_LOG("AudioDeviceDescriptor::~AudioDeviceDescriptor");
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel) const
{
    MEDIA_DEBUG_LOG("AudioDeviceDescriptor::Marshalling called");
    return parcel.WriteInt32(deviceType_) && parcel.WriteInt32(deviceRole_);
}

AudioDeviceDescriptor *AudioDeviceDescriptor::Unmarshalling(Parcel &in)
{
    MEDIA_DEBUG_LOG("AudioDeviceDescriptor::Unmarshalling called");
    AudioDeviceDescriptor *audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (audioDeviceDescriptor == nullptr) {
        return nullptr;
    }
    audioDeviceDescriptor->deviceType_ = static_cast<DeviceType>(in.ReadInt32());
    audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(in.ReadInt32());
    return audioDeviceDescriptor;
}
} // namespace AudioStandard
} // namespace OHOS
