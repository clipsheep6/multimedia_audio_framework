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

#include "audio_errors.h"
#include "audio_policy_proxy.h"
#include "audio_server_death_recipient.h"
#include "iservice_registry.h"
#include "media_log.h"
#include "system_ability_definition.h"
#include "audio_policy_manager.h"

namespace OHOS {
namespace AudioStandard {
static sptr<IAudioPolicy> g_sProxy = nullptr;
bool AudioPolicyManager::serverConnected = false;

void AudioPolicyManager::Init()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager::init failed");
        return;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);
    if (object == nullptr) {
        MEDIA_DEBUG_LOG("AudioPolicyManager::object is NULL.");
    }

    g_sProxy = iface_cast<IAudioPolicy>(object);
    if (g_sProxy == nullptr) {
        MEDIA_DEBUG_LOG("AudioPolicyManager::init g_sProxy is NULL.");
    } else {
        serverConnected = true;
        MEDIA_DEBUG_LOG("AudioPolicyManager::init g_sProxy is assigned.");
    }

    RegisterAudioPolicyServerDeathRecipient();
}

void AudioPolicyManager::RegisterAudioPolicyServerDeathRecipient()
{
    MEDIA_INFO_LOG("Register audio policy server death recipient");
    pid_t pid = 0;
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(pid);
    if (deathRecipient_ != nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::AUDIO_POLICY_SERVICE_ID);
        deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyManager::AudioPolicyServerDied, this,
                                               std::placeholders::_1));
        bool result = object->AddDeathRecipient(deathRecipient_);
        if (!result) {
            MEDIA_ERR_LOG("failed to add deathRecipient");
        }
    }
}

void AudioPolicyManager::AudioPolicyServerDied(pid_t pid)
{
    MEDIA_INFO_LOG("Audio policy server died: reestablish connection");
    serverConnected = false;
}

int32_t AudioPolicyManager::SetStreamVolume(AudioStreamType streamType, float volume)
{
    return g_sProxy->SetStreamVolume(streamType, volume);
}

int32_t AudioPolicyManager::SetRingerMode(AudioRingerMode ringMode)
{
    return g_sProxy->SetRingerMode(ringMode);
}

AudioRingerMode AudioPolicyManager::GetRingerMode()
{
    return g_sProxy->GetRingerMode();
}

float AudioPolicyManager::GetStreamVolume(AudioStreamType streamType)
{
    return g_sProxy->GetStreamVolume(streamType);
}

int32_t AudioPolicyManager::SetStreamMute(AudioStreamType streamType, bool mute)
{
    return g_sProxy->SetStreamMute(streamType, mute);
}

bool AudioPolicyManager::GetStreamMute(AudioStreamType streamType)
{
    return g_sProxy->GetStreamMute(streamType);
}

bool AudioPolicyManager::IsStreamActive(AudioStreamType streamType)
{
    return g_sProxy->IsStreamActive(streamType);
}

int32_t AudioPolicyManager::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    return g_sProxy->SetDeviceActive(deviceType, active);
}

bool AudioPolicyManager::IsDeviceActive(InternalDeviceType deviceType)
{
    return g_sProxy->IsDeviceActive(deviceType);
}

int32_t AudioPolicyManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

    ringerModelistenerStub_ = new(std::nothrow) AudioRingerModeUpdateListenerStub();
    if (ringerModelistenerStub_ == nullptr || g_sProxy == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: object null");
        return ERROR;
    }
    ringerModelistenerStub_->SetCallback(callback);

    sptr<IRemoteObject> object = ringerModelistenerStub_->AsObject();
    if (object == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: listenerStub->AsObject is nullptr..");
        return ERROR;
    }

    return g_sProxy->SetRingerModeCallback(clientId, object);
}

int32_t AudioPolicyManager::UnsetRingerModeCallback(const int32_t clientId)
{
    return g_sProxy->UnsetRingerModeCallback(clientId);
}

int32_t AudioPolicyManager::SetAudioInterruptCallback(const uint32_t sessionID,
                                                      const std::shared_ptr<AudioInterruptCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: callback is nullptr");
        return ERR_INVALID_PARAM;
    }

#ifdef LISTENER_STUB_MUTEX
    // Need to lock member variable listenerStub_ as SetAudioInterruptCallback
    // can be called from different threads in multi renderer usage
    std::unique_lock<std::mutex> lock(listenerStubMutex_);
#endif // LISTENER_STUB_MUTEX
    listenerStub_ = new(std::nothrow) AudioPolicyManagerListenerStub();
    if (listenerStub_ == nullptr || g_sProxy == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: object null");
        return ERROR;
    }
    listenerStub_->SetInterruptCallback(callback);

    sptr<IRemoteObject> object = listenerStub_->AsObject();
    if (object == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: listenerStub->AsObject is nullptr..");
        return ERROR;
    }
#ifdef LISTENER_STUB_MUTEX
    lock.unlock(); // unlock once it is converted into IRemoteObject
#endif // LISTENER_STUB_MUTEX

    return g_sProxy->SetAudioInterruptCallback(sessionID, object);
}

int32_t AudioPolicyManager::UnsetAudioInterruptCallback(const uint32_t sessionID)
{
    return g_sProxy->UnsetAudioInterruptCallback(sessionID);
}

int32_t AudioPolicyManager::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    return g_sProxy->ActivateAudioInterrupt(audioInterrupt);
}

int32_t AudioPolicyManager::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    return g_sProxy->DeactivateAudioInterrupt(audioInterrupt);
}

AudioStreamType AudioPolicyManager::GetStreamInFocus()
{
    return g_sProxy->GetStreamInFocus();
}

int32_t AudioPolicyManager::SetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    if (callback == nullptr) {
        MEDIA_ERR_LOG("AudioSystemManager volume back is nullptr");
        return ERR_INVALID_PARAM;
    }

    volumeKeyEventCallback_ = callback;

    volumeKeyEventListenerStub_ = new(std::nothrow) AudioVolumeKeyEventCallbackStub();
    if (volumeKeyEventListenerStub_ == nullptr || g_sProxy == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: object null");
        return ERROR;
    }
    volumeKeyEventListenerStub_->SetOnVolumeKeyEventCallback(callback);

    sptr<IRemoteObject> object = volumeKeyEventListenerStub_->AsObject();
    if (object == nullptr) {
        MEDIA_ERR_LOG("AudioPolicyManager: volumeKeyEventListenerStub_->AsObject is nullptr..");
        return ERROR;
    }
    return g_sProxy->SetVolumeKeyEventCallback(object);
}
} // namespace AudioStandard
} // namespace OHOS
