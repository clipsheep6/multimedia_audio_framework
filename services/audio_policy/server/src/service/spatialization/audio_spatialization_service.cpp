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

// std::shared_ptr<DataShare::DataShareHelper> g_dataShareHelper = nullptr;
// static sptr<IStandardAudioService> g_adProxy = nullptr;
// mutex g_adProxyMutex;
// mutex g_dataShareHelperMutex;

AudioSpatializationService::~AudioSpatializationService()
{
    AUDIO_ERR_LOG("~AudioSpatializationService()");
    // Deinit();
}

bool AudioSpatializationService::Init(void)
{
    return true;
}

// const sptr<IStandardAudioService> AudioSpatializationService::GetAudioServerProxy()
// {
//     AUDIO_DEBUG_LOG("[Policy Service] Start get audio policy service proxy.");
//     lock_guard<mutex> lock(g_adProxyMutex);

//     if (g_adProxy == nullptr) {
//         auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
//         if (samgr == nullptr) {
//             AUDIO_ERR_LOG("[Policy Service] Get samgr failed.");
//             return nullptr;
//         }

//         sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
//         if (object == nullptr) {
//             AUDIO_ERR_LOG("[Policy Service] audio service remote object is NULL.");
//             return nullptr;
//         }

//         g_adProxy = iface_cast<IStandardAudioService>(object);
//         if (g_adProxy == nullptr) {
//             AUDIO_ERR_LOG("[Policy Service] init g_adProxy is NULL.");
//             return nullptr;
//         }
//     }
//     const sptr<IStandardAudioService> gsp = g_adProxy;
//     return gsp;
// }

bool AudioSpatializationService::ConnectServiceAdapter()
{
    return true;
}

void AudioSpatializationService::Deinit(void)
{
    return;
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
    HandleSpatializationEnabledChange(enable);
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
    HandleHeadTrackingEnabledChange(enable);
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
} // namespace AudioStandard
} // namespace OHOS
