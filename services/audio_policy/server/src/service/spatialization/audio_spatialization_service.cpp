/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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


#include "audio_spatialization_service.h"

#include "audio_policy_service.h"

#include "ipc_skeleton.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "parameter.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"

#include "audio_spatialization_state_change_listener_proxy.h"
#include "i_standard_spatialization_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const int32_t SPATIALIZATION_SERVICE_OK = 0;
static sptr<IStandardAudioService> g_adProxy = nullptr;
mutex g_adSpatializationProxyMutex;

AudioSpatializationService::~AudioSpatializationService()
{
    AUDIO_ERR_LOG("~AudioSpatializationService()");
}

bool AudioSpatializationService::Init(void)
{
    return true;
}

bool AudioSpatializationService::ConnectServiceAdapter()
{
    return true;
}

void AudioSpatializationService::Deinit(void)
{
    return;
}

const sptr<IStandardAudioService> AudioSpatializationService::GetAudioServerProxy()
{
    AUDIO_DEBUG_LOG("[Spatialization Service] Start get audio spatialization service proxy.");
    lock_guard<mutex> lock(g_adSpatializationProxyMutex);

    if (g_adProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            AUDIO_ERR_LOG("[Spatialization Service] Get samgr failed.");
            return nullptr;
        }

        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        if (object == nullptr) {
            AUDIO_ERR_LOG("[Spatialization Service] audio service remote object is NULL.");
            return nullptr;
        }

        g_adProxy = iface_cast<IStandardAudioService>(object);
        if (g_adProxy == nullptr) {
            AUDIO_ERR_LOG("[Spatialization Service] init g_adProxy is NULL.");
            return nullptr;
        }
    }
    const sptr<IStandardAudioService> gsp = g_adProxy;
    return gsp;
}

bool AudioSpatializationService::IsSpatializationEnabled()
{
    return spatializationEnabledFlag_;
}

int32_t AudioSpatializationService::SetSpatializationEnabled(const bool enable)
{
    if (spatializationEnabledFlag_ == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    spatializationEnabledFlag_ = enable;
    if (UpdateSpatializationEnabledReal() != 0) {
        return ERROR;
    }
    if (UpdateSpatializationState() != 0) {
        return ERROR;
    }
    HandleSpatializationEnabledChange(enable);
    HandleSpatializationStateChange();
    return SPATIALIZATION_SERVICE_OK;
}

bool AudioSpatializationService::IsHeadTrackingEnabled()
{
    return headTrackingEnabledFlag_;
}

int32_t AudioSpatializationService::SetHeadTrackingEnabled(const bool enable)
{
    if (headTrackingEnabledFlag_ == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    headTrackingEnabledFlag_ = enable;
    if (UpdateHeadTrackingEnabledReal() != 0) {
        return ERROR;
    }
    if (UpdateSpatializationState() != 0) {
        return ERROR;
    }
    HandleHeadTrackingEnabledChange(enable);
    HandleSpatializationStateChange();
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::RegisterSpatializationEnabledEventListener(int32_t clientPid,
    const sptr<IRemoteObject> &object, bool hasSystemPermission)
{
    std::lock_guard<std::mutex> lock(spatializationEnabledChangeListnerMutex_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "set spatialization enabled event listener object is nullptr");
    sptr<IStandardSpatializationEnabledChangeListener> listener =
        iface_cast<IStandardSpatializationEnabledChangeListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "spatialization enabled obj cast failed");

    std::shared_ptr<AudioSpatializationEnabledChangeCallback> callback =
        std::make_shared<AudioSpatializationEnabledChangeListenerCallback>(listener, hasSystemPermission);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "failed to create spatialization enabled cb obj");

    spatializationEnabledCBMap_[clientPid] = callback;
    return SUCCESS;
}

int32_t AudioSpatializationService::RegisterHeadTrackingEnabledEventListener(int32_t clientPid,
    const sptr<IRemoteObject> &object, bool hasSystemPermission)
{
    std::lock_guard<std::mutex> lock(headTrackingEnabledChangeListnerMutex_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "set head tracking enabled event listener object is nullptr");
    sptr<IStandardHeadTrackingEnabledChangeListener> listener =
        iface_cast<IStandardHeadTrackingEnabledChangeListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "head tracking enabled obj cast failed");

    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> callback =
        std::make_shared<AudioHeadTrackingEnabledChangeListenerCallback>(listener, hasSystemPermission);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "failed to create head tracking enabled cb obj");

    headTrackingEnabledCBMap_[clientPid] = callback;
    return SUCCESS;
}

int32_t AudioSpatializationService::UnregisterSpatializationEnabledEventListener(int32_t clientPid)
{
    std::lock_guard<std::mutex> lock(spatializationEnabledChangeListnerMutex_);
    spatializationEnabledCBMap_.erase(clientPid);
    return SUCCESS;
}

int32_t AudioSpatializationService::UnregisterHeadTrackingEnabledEventListener(int32_t clientPid)
{
    std::lock_guard<std::mutex> lock(headTrackingEnabledChangeListnerMutex_);
    headTrackingEnabledCBMap_.erase(clientPid);
    return SUCCESS;
}

void AudioSpatializationService::HandleSpatializationEnabledChange(const bool &enabled)
{
    std::lock_guard<std::mutex> lock(spatializationEnabledChangeListnerMutex_);
    for (auto it = spatializationEnabledCBMap_.begin(); it != spatializationEnabledCBMap_.end(); ++it) {
        std::shared_ptr<AudioSpatializationEnabledChangeCallback> spatializationEnabledChangeCb = it->second;
        if (spatializationEnabledChangeCb == nullptr) {
            AUDIO_ERR_LOG("spatializationEnabledChangeCb : nullptr for client : %{public}d", it->first);
            it = spatializationEnabledCBMap_.erase(it);
            continue;
        }
        spatializationEnabledChangeCb->OnSpatializationEnabledChange(enabled);
    }
}

void AudioSpatializationService::HandleHeadTrackingEnabledChange(const bool &enabled)
{
    std::lock_guard<std::mutex> lock(headTrackingEnabledChangeListnerMutex_);
    for (auto it = headTrackingEnabledCBMap_.begin(); it != headTrackingEnabledCBMap_.end(); ++it) {
        std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> headTrackingEnabledChangeCb = it->second;
        if (headTrackingEnabledChangeCb == nullptr) {
            AUDIO_ERR_LOG("headTrackingEnabledChangeCb : nullptr for client : %{public}d", it->first);
            it = headTrackingEnabledCBMap_.erase(it);
            continue;
        }
        headTrackingEnabledChangeCb->OnHeadTrackingEnabledChange(enabled);
    }
}

std::vector<bool> AudioSpatializationService::GetSpatializationState(const StreamUsage streamUsage)
{
    std::vector<bool> spatializationState;
    spatializationState.push_back(spatializationEnabledReal_);
    spatializationState.push_back(headTrackingEnabledReal_);
    return spatializationState;
}

bool AudioSpatializationService::IsSpatializationSupported()
{
    return true;
}

bool AudioSpatializationService::IsSpatializationSupportedForDevice(const std::string address)
{
    if (!addressToSpatialDeviceStateMap_.count(address)) {
        return false;
    }
    return addressToSpatialDeviceStateMap_[address].isSpatializationSupported;
}

bool AudioSpatializationService::IsHeadTrackingSupported()
{
    return true;
}

bool AudioSpatializationService::IsHeadTrackingSupportedForDevice(const std::string address)
{
    if (!addressToSpatialDeviceStateMap_.count(address)) {
        return false;
    }
    return addressToSpatialDeviceStateMap_[address].isHeadTrackingSupported;
}

int32_t AudioSpatializationService::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    if (!addressToSpatialDeviceStateMap_.count(audioSpatialDeviceState.address)) {
        addressToSpatialDeviceStateMap_.insert(std::make_pair(audioSpatialDeviceState.address,
            audioSpatialDeviceState));
            AudioSpatialDeviceState res = addressToSpatialDeviceStateMap_[audioSpatialDeviceState.address];
        return SPATIALIZATION_SERVICE_OK;
    }
    addressToSpatialDeviceStateMap_[audioSpatialDeviceState.address] = audioSpatialDeviceState;
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::UpdateSpatializationState()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("Service proxy unavailable: g_adProxy null");
        return -1;
    }
    std::vector<bool> spatializationState;
    spatializationState.push_back(spatializationEnabledReal_);
    spatializationState.push_back(headTrackingEnabledReal_);
    int32_t ret = gsp->UpdateSpatializationState(spatializationState);
    if (ret != 0) {
        AUDIO_ERR_LOG("UpdateSpatializationState fail");
        return -1;
    } else {
        return SPATIALIZATION_SERVICE_OK;
    }
}

int32_t AudioSpatializationService::UpdateSpatializationEnabledReal()
{
    spatializationEnabledReal_ = spatializationEnabledFlag_;
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::UpdateHeadTrackingEnabledReal()
{
    headTrackingEnabledReal_ = headTrackingEnabledFlag_;
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(spatializationStateChangeListnerMutex_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "set spatialization state event listener object is nullptr");
    sptr<IStandardSpatializationStateChangeListener> listener =
        iface_cast<IStandardSpatializationStateChangeListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "spatialization state obj cast failed");

    std::shared_ptr<AudioSpatializationStateChangeCallback> callback =
        std::make_shared<AudioSpatializationStateChangeListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "failed to create spatialization state cb obj");

    spatializationStateCBMap_[sessionID] = callback;
    return SUCCESS;
}

void AudioSpatializationService::HandleSpatializationStateChange()
{
    std::lock_guard<std::mutex> lock(spatializationStateChangeListnerMutex_);
    std::vector<bool> spatializationState;
    spatializationState.push_back(spatializationEnabledReal_);
    spatializationState.push_back(headTrackingEnabledReal_);
    for (auto it = spatializationStateCBMap_.begin(); it != spatializationStateCBMap_.end(); ++it) {
        std::shared_ptr<AudioSpatializationStateChangeCallback> spatializationStateChangeCb = it->second;
        if (spatializationStateChangeCb == nullptr) {
            AUDIO_ERR_LOG("spatializationStateChangeCb : nullptr for sessionID : %{public}d",
                static_cast<int32_t>(it->first));
            it = spatializationStateCBMap_.erase(it);
            continue;
        }
        spatializationStateChangeCb->OnSpatializationStateChange(spatializationState);
    }
}
} // namespace AudioStandard
} // namespace OHOS
