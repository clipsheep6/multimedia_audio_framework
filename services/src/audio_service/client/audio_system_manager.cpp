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
#include "audio_manager_proxy.h"
#include "audio_stream.h"
#include "audio_policy_manager.h"
#include "audio_volume_key_event_callback_stub.h"

#include "iservice_registry.h"
#include "media_log.h"
#include "system_ability_definition.h"

#include "audio_system_manager.h"

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
    MEDIA_DEBUG_LOG("AudioSystemManager::~AudioSystemManager");
    if (cbClientId_ != -1) {
        UnsetRingerModeCallback(cbClientId_);
    }

    if (volumeChangeClientPid_ != -1) {
        MEDIA_DEBUG_LOG("AudioSystemManager::~AudioSystemManager UnregisterVolumeKeyEventCallback");
        (void)UnregisterVolumeKeyEventCallback(volumeChangeClientPid_);
    }
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
        return;
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

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    MEDIA_DEBUG_LOG("SetAudioScene audioScene_=%{public}d done", scene);
    return AudioPolicyManager::GetInstance().SetAudioScene(scene);
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    return AudioPolicyManager::GetInstance().GetAudioScene();
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
        case STREAM_VOICE_CALL:
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

void AudioSystemManager::SetAudioParameter(const std::string &key, const std::string &value) const
{
    g_sProxy->SetAudioParameter(key, value);
}

const char *AudioSystemManager::RetrieveCookie(int32_t &size) const
{
    return g_sProxy->RetrieveCookie(size);
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
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
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
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
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
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
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
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
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

int32_t AudioSystemManager::SetDeviceChangeCallback(const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback)
{
    MEDIA_INFO_LOG("Entered AudioSystemManager::%{public}s", __func__);
    if (callback == nullptr) {
        MEDIA_ERR_LOG("SetDeviceChangeCallback: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    return AudioPolicyManager::GetInstance().SetDeviceChangeCallback(callback);
}

int32_t AudioSystemManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    cbClientId_ = clientId;

    return AudioPolicyManager::GetInstance().SetRingerModeCallback(clientId, callback);
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    return AudioPolicyManager::GetInstance().UnsetRingerModeCallback(clientId);
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
{
    return AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
                                                           const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    MEDIA_DEBUG_LOG("AudioSystemManager RegisterVolumeKeyEventCallback");

    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager::RegisterVolumeKeyEventCallbackcallback is nullptr");
        return ERR_INVALID_PARAM;
    }
    volumeChangeClientPid_ = clientPid;

    return AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback);
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid)
{
    MEDIA_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback");
    int32_t ret = AudioPolicyManager::GetInstance().UnsetVolumeKeyEventCallback(clientPid);
    if (!ret) {
        MEDIA_DEBUG_LOG("AudioSystemManager::UnregisterVolumeKeyEventCallback success");
        volumeChangeClientPid_ = -1;
    }

    return ret;
}

// Below stub implementation is added to handle compilation error in call manager
// Once call manager adapt to new interrupt implementation, this will be removed
int32_t AudioSystemManager::SetAudioManagerCallback(const AudioSystemManager::AudioVolumeType streamType,
                                                    const std::shared_ptr<AudioManagerCallback> &callback)
{
    MEDIA_DEBUG_LOG("AudioSystemManager SetAudioManagerCallback stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioSystemManager::AudioVolumeType streamType) const
{
    MEDIA_DEBUG_LOG("AudioSystemManager UnsetAudioManagerCallback stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    MEDIA_DEBUG_LOG("AudioSystemManager ActivateAudioInterrupt stub implementation");
    return SUCCESS;
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const
{
    MEDIA_DEBUG_LOG("AudioSystemManager DeactivateAudioInterrupt stub implementation");
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
