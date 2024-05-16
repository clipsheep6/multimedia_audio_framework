/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "AudioSpatializationService"

#include "audio_spatialization_service.h"

#include <thread>
#include "ipc_skeleton.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "setting_provider.h"
#include "system_ability_definition.h"
#include "parameter.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"

#include "audio_spatialization_state_change_listener_proxy.h"
#include "i_standard_spatialization_state_change_listener.h"

#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const int32_t SPATIALIZATION_SERVICE_OK = 0;
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_BT_MUSIC";
static const std::string SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL = "SPATIALIZATION_AND_HEADTRACKING";
static const std::string SPATIALIZATION_SUPPORTED_LABEL = "SPATIALIZATION";
static const std::string HEAD_TRACKING_SUPPORTED_LABEL = "HEADTRACKING";
static const std::string SPATIALIZATION_STATE_SETTINGKEY = "spatialization_state";
static const std::string SPATIALIZATION_SCENE_SETTINGKEY = "spatialization_scene";
static sptr<IStandardAudioService> g_adProxy = nullptr;
mutex g_adSpatializationProxyMutex;

enum SpatializationStateOffset {
    SPATIALIZATION_OFFSET,
    HEADTRACKING_OFFSET
};

static void UnpackSpatializationState(uint32_t pack, AudioSpatializationState &state)
{
    state = {.spatializationEnabled = pack >> SPATIALIZATION_OFFSET & 1,
        .headTrackingEnabled = pack >> HEADTRACKING_OFFSET & 1};
}

static uint32_t PackSpatializationState(AudioSpatializationState state)
{
    uint32_t spatializationEnabled__ = 0;
    uint32_t headTrackingEnabled__ = 0;
    if (state.spatializationEnabled) {
        spatializationEnabled__ = 1;
    }
    if (state.headTrackingEnabled) {
        headTrackingEnabled__ = 1;
    }
    return (spatializationEnabled__ << SPATIALIZATION_OFFSET) | (headTrackingEnabled__ << HEADTRACKING_OFFSET);
}

static bool IsAudioSpatialDeviceStateEqual(const AudioSpatialDeviceState &a, const AudioSpatialDeviceState &b)
{
    return ((a.address == b.address) && (a.isSpatializationSupported == b.isSpatializationSupported) &&
        (a.isHeadTrackingSupported == b.isHeadTrackingSupported) && (a.spatialDeviceType == b.spatialDeviceType));
}

static bool IsSpatializationSupportedUsage(StreamUsage usage)
{
    return usage != STREAM_USAGE_GAME;
}

AudioSpatializationService::~AudioSpatializationService()
{
    AUDIO_ERR_LOG("~AudioSpatializationService()");
}

void AudioSpatializationService::Init(const std::vector<EffectChain> &effectChains)
{
    for (auto effectChain: effectChains) {
        if (effectChain.name != BLUETOOTH_EFFECT_CHAIN_NAME) {
            continue;
        }
        if (effectChain.label == SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL) {
            isSpatializationSupported_ = true;
            isHeadTrackingSupported_ = true;
        } else if (effectChain.label == SPATIALIZATION_SUPPORTED_LABEL) {
            isSpatializationSupported_ = true;
        } else if (effectChain.label == HEAD_TRACKING_SUPPORTED_LABEL) {
            isHeadTrackingSupported_ = true;
        }
    }
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
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr,
            "[Spatialization Service] Get samgr failed.");

        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr,
            "[Spatialization Service] audio service remote object is NULL.");

        g_adProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_adProxy != nullptr, nullptr,
            "[Spatialization Service] init g_adProxy is NULL.");
    }
    const sptr<IStandardAudioService> gsp = g_adProxy;
    return gsp;
}

bool AudioSpatializationService::IsSpatializationEnabled()
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    return spatializationStateFlag_.spatializationEnabled;
}

int32_t AudioSpatializationService::SetSpatializationEnabled(const bool enable)
{
    AUDIO_INFO_LOG("Spatialization enabled is set to be: %{public}d", enable);
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    if (spatializationStateFlag_.spatializationEnabled == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    spatializationStateFlag_.spatializationEnabled = enable;
    HandleSpatializationEnabledChange(enable);
    if (UpdateSpatializationStateReal(false) != 0) {
        return ERROR;
    }
    WriteSpatializationStateToDb(WRITE_SPATIALIZATION_STATE);
    return SPATIALIZATION_SERVICE_OK;
}

bool AudioSpatializationService::IsHeadTrackingEnabled()
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    return spatializationStateFlag_.headTrackingEnabled;
}

int32_t AudioSpatializationService::SetHeadTrackingEnabled(const bool enable)
{
    AUDIO_INFO_LOG("Head tracking enabled is set to be: %{public}d", enable);
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    if (spatializationStateFlag_.headTrackingEnabled == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    spatializationStateFlag_.headTrackingEnabled = enable;
    HandleHeadTrackingEnabledChange(enable);
    if (UpdateSpatializationStateReal(false) != 0) {
        return ERROR;
    }
    WriteSpatializationStateToDb(WRITE_SPATIALIZATION_STATE);
    return SPATIALIZATION_SERVICE_OK;
}

void AudioSpatializationService::HandleSpatializationEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("Spatialization enabled callback is triggered: state is %{public}d", enabled);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendSpatializatonEnabledChangeEvent(enabled);
    }
}

void AudioSpatializationService::HandleHeadTrackingEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("Head tracking enabled callback is triggered: state is %{public}d", enabled);
    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendHeadTrackingEnabledChangeEvent(enabled);
    }
}

AudioSpatializationState AudioSpatializationService::GetSpatializationState(const StreamUsage streamUsage)
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    AudioSpatializationState spatializationState = {false, false};
    if (IsSpatializationSupportedUsage(streamUsage)) {
        spatializationState.spatializationEnabled = spatializationEnabledReal_;
        spatializationState.headTrackingEnabled = headTrackingEnabledReal_;
    }
    return spatializationState;
}

bool AudioSpatializationService::IsSpatializationSupported()
{
    return isSpatializationSupported_;
}

bool AudioSpatializationService::IsSpatializationSupportedForDevice(const std::string address)
{
    std::lock_guard<std::mutex> lock(spatializationSupportedMutex_);

    if (!addressToSpatialDeviceStateMap_.count(address)) {
        AUDIO_INFO_LOG("specified address for spatialization is not in memory");
        return false;
    }
    return addressToSpatialDeviceStateMap_[address].isSpatializationSupported;
}

bool AudioSpatializationService::IsHeadTrackingSupported()
{
    return isHeadTrackingSupported_;
}

bool AudioSpatializationService::IsHeadTrackingSupportedForDevice(const std::string address)
{
    std::lock_guard<std::mutex> lock(spatializationSupportedMutex_);

    if (!addressToSpatialDeviceStateMap_.count(address)) {
        AUDIO_INFO_LOG("specified address for head tracking is not in memory");
        return false;
    }
    return addressToSpatialDeviceStateMap_[address].isHeadTrackingSupported;
}

int32_t AudioSpatializationService::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    AUDIO_INFO_LOG("UpdateSpatialDeviceState Entered");
    {
        std::lock_guard<std::mutex> lock(spatializationSupportedMutex_);
        if (addressToSpatialDeviceStateMap_.count(audioSpatialDeviceState.address) > 0 &&
            IsAudioSpatialDeviceStateEqual(addressToSpatialDeviceStateMap_[audioSpatialDeviceState.address],
            audioSpatialDeviceState)) {
            AUDIO_INFO_LOG("no need to UpdateSpatialDeviceState");
            return SPATIALIZATION_SERVICE_OK;
        }
        addressToSpatialDeviceStateMap_[audioSpatialDeviceState.address] = audioSpatialDeviceState;
    }

    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    if (UpdateSpatializationStateReal(false) != 0) {
        return ERROR;
    }
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const StreamUsage streamUsage, const sptr<IRemoteObject> &object)
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

    spatializationStateCBMap_[sessionID] = std::make_pair(callback, streamUsage);
    return SUCCESS;
}

int32_t AudioSpatializationService::UnregisterSpatializationStateEventListener(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(spatializationStateChangeListnerMutex_);
    spatializationStateCBMap_.erase(sessionID);
    return SUCCESS;
}

void AudioSpatializationService::UpdateCurrentDevice(const std::string macAddress)
{
    AUDIO_INFO_LOG("UpdateCurrentDevice Entered");
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    if (currentDeviceAddress_ == macAddress) {
        AUDIO_INFO_LOG("no need to UpdateCurrentDevice");
        return;
    }
    std::string preDeviceAddress = currentDeviceAddress_;
    currentDeviceAddress_ = macAddress;
    if (UpdateSpatializationStateReal(true, preDeviceAddress) != 0) {
        AUDIO_WARNING_LOG("UpdateSpatializationStateReal fail");
    }
}

AudioSpatializationSceneType AudioSpatializationService::GetSpatializationSceneType()
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    return spatializationSceneType_;
}

int32_t AudioSpatializationService::SetSpatializationSceneType(
    const AudioSpatializationSceneType spatializationSceneType)
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    AUDIO_INFO_LOG("spatialization scene type is set to be %{public}d", spatializationSceneType);
    spatializationSceneType_ = spatializationSceneType;
    int32_t ret = UpdateSpatializationSceneType();
    CHECK_AND_RETURN_RET_LOG(ret == SPATIALIZATION_SERVICE_OK, ret, "set spatialization scene type failed");
    WriteSpatializationStateToDb(WRITE_SPATIALIZATION_SCENE);
    return SPATIALIZATION_SERVICE_OK;
}

bool AudioSpatializationService::IsHeadTrackingDataRequested(const std::string &macAddress)
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);

    if (macAddress != currentDeviceAddress_) {
        return false;
    }

    return isHeadTrackingDataRequested_;
}

void AudioSpatializationService::UpdateRendererInfo(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfo)
{
    AUDIO_DEBUG_LOG("Start");
    {
        std::lock_guard<std::mutex> lock(rendererInfoChangingMutex_);
        AudioRendererInfoForSpatialization spatializationRendererInfo;

        spatializationRendererInfoList_.clear();
        for (const auto &it : rendererChangeInfo) {
            spatializationRendererInfo.rendererState = it->rendererState;
            spatializationRendererInfo.deviceMacAddress = it->outputDeviceInfo.macAddress;
            spatializationRendererInfo.streamUsage = it->rendererInfo.streamUsage;
            spatializationRendererInfoList_.push_back(spatializationRendererInfo);
        }
    }
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    UpdateHeadTrackingDeviceState(false);
}

int32_t AudioSpatializationService::UpdateSpatializationStateReal(bool outputDeviceChange, std::string preDeviceAddress)
{
    bool spatializationEnabled = spatializationStateFlag_.spatializationEnabled && IsSpatializationSupported() &&
        IsSpatializationSupportedForDevice(currentDeviceAddress_);
    bool headTrackingEnabled = spatializationStateFlag_.headTrackingEnabled && IsHeadTrackingSupported() &&
        IsHeadTrackingSupportedForDevice(currentDeviceAddress_) && spatializationEnabled;
    if ((spatializationEnabledReal_ == spatializationEnabled) && (headTrackingEnabledReal_ == headTrackingEnabled)) {
        AUDIO_INFO_LOG("no need to update real spatialization state");
        UpdateHeadTrackingDeviceState(outputDeviceChange, preDeviceAddress);
        return SUCCESS;
    }
    spatializationEnabledReal_ = spatializationEnabled;
    headTrackingEnabledReal_ = headTrackingEnabled;
    if (UpdateSpatializationState() != 0) {
        return ERROR;
    }
    HandleSpatializationStateChange(outputDeviceChange);
    UpdateHeadTrackingDeviceState(outputDeviceChange, preDeviceAddress);
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::UpdateSpatializationState()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("Service proxy unavailable: g_adProxy null");
        return -1;
    }
    AudioSpatializationState spatializationState = {spatializationEnabledReal_, headTrackingEnabledReal_};
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->UpdateSpatializationState(spatializationState);
    IPCSkeleton::SetCallingIdentity(identity);
    if (ret != 0) {
        AUDIO_WARNING_LOG("UpdateSpatializationState fail");
    }
    return SPATIALIZATION_SERVICE_OK;
}

int32_t AudioSpatializationService::UpdateSpatializationSceneType()
{
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    if (gsp == nullptr) {
        AUDIO_ERR_LOG("Service proxy unavailable: g_adProxy null");
        return -1;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gsp->SetSpatializationSceneType(spatializationSceneType_);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == SPATIALIZATION_SERVICE_OK, ret, "set spatialization scene type failed");
    return SPATIALIZATION_SERVICE_OK;
}

void AudioSpatializationService::HandleSpatializationStateChange(bool outputDeviceChange)
{
    AUDIO_INFO_LOG("Spatialization State callback is triggered");
    std::lock_guard<std::mutex> lock(spatializationStateChangeListnerMutex_);

    AudioSpatializationState spatializationState = {spatializationEnabledReal_, headTrackingEnabledReal_};
    AudioSpatializationState spatializationNotSupported = {false, false};
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnabledMap;

    for (auto it = spatializationStateCBMap_.begin(); it != spatializationStateCBMap_.end(); ++it) {
        std::shared_ptr<AudioSpatializationStateChangeCallback> spatializationStateChangeCb = (it->second).first;
        if (spatializationStateChangeCb == nullptr) {
            AUDIO_ERR_LOG("spatializationStateChangeCb : nullptr for sessionID : %{public}d",
                static_cast<int32_t>(it->first));
            it = spatializationStateCBMap_.erase(it);
            continue;
        }
        if (!IsSpatializationSupportedUsage((it->second).second)) {
            if (!outputDeviceChange) {
                sessionIDToSpatializationEnabledMap.insert(std::make_pair(it->first, false));
            }
            spatializationStateChangeCb->OnSpatializationStateChange(spatializationNotSupported);
        } else {
            if (!outputDeviceChange) {
                sessionIDToSpatializationEnabledMap.insert(std::make_pair(it->first, spatializationEnabledReal_));
            }
            spatializationStateChangeCb->OnSpatializationStateChange(spatializationState);
        }
    }

    if (!outputDeviceChange) {
        AUDIO_INFO_LOG("notify offload entered");
        std::thread notifyOffloadThread = std::thread(std::bind(
            &AudioPolicyService::UpdateA2dpOffloadFlagBySpatialService,
            &AudioPolicyService::GetAudioPolicyService(),
            currentDeviceAddress_,
            sessionIDToSpatializationEnabledMap));
        notifyOffloadThread.detach();
    }
}

void AudioSpatializationService::InitSpatializationState()
{
    std::lock_guard<std::mutex> lock(spatializationServiceMutex_);
    CHECK_AND_RETURN_LOG(!isLoadedfromDb_, "the spatialization values have already been loaded");
    int32_t pack = 0;
    int32_t sceneType = 0;

    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.GetIntValue(SPATIALIZATION_STATE_SETTINGKEY, pack);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("Failed to read spatialization_state from setting db! Err: %{public}d", ret);
        WriteSpatializationStateToDb(WRITE_SPATIALIZATION_STATE);
    } else {
        UnpackSpatializationState(pack, spatializationStateFlag_);
        UpdateSpatializationStateReal(false);
    }

    ret = settingProvider.GetIntValue(SPATIALIZATION_SCENE_SETTINGKEY, sceneType);
    if (ret != SUCCESS || sceneType < SPATIALIZATION_SCENE_TYPE_DEFAULT || sceneType > SPATIALIZATION_SCENE_TYPE_MAX) {
        AUDIO_WARNING_LOG("Failed to read spatialization_scene from setting db! Err: %{public}d", ret);
        WriteSpatializationStateToDb(WRITE_SPATIALIZATION_SCENE);
    } else {
        spatializationSceneType_ = static_cast<AudioSpatializationSceneType>(sceneType);
        UpdateSpatializationSceneType();
    }
    isLoadedfromDb_ = true;
}

void AudioSpatializationService::WriteSpatializationStateToDb(WriteToDbOperation operation)
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    switch (operation) {
        case WRITE_SPATIALIZATION_STATE: {
            ErrCode ret = settingProvider.PutIntValue(
                SPATIALIZATION_STATE_SETTINGKEY, PackSpatializationState(spatializationStateFlag_));
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to write spatialization_state to setting db: %{public}d", ret);
            break;
        }
        case WRITE_SPATIALIZATION_SCENE: {
            ErrCode ret = settingProvider.PutIntValue(
                SPATIALIZATION_SCENE_SETTINGKEY, static_cast<uint32_t>(spatializationSceneType_));
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to write spatialization_scene to setting db: %{public}d", ret);
            break;
        }
        default:
            break;
    }
}

bool AudioSpatializationService::IsHeadTrackingDataRequestedForCurrentDevice()
{
    std::lock_guard<std::mutex> lock(rendererInfoChangingMutex_);
    bool isStreamRunning = false;
    for (const auto &rendererInfo : spatializationRendererInfoList_) {
        if (rendererInfo.rendererState == RENDERER_RUNNING && rendererInfo.deviceMacAddress == currentDeviceAddress_ &&
            IsSpatializationSupportedUsage(rendererInfo.streamUsage)) {
            isStreamRunning = true;
            break;
        }
    }
    return (isStreamRunning && headTrackingEnabledReal_);
}

void AudioSpatializationService::UpdateHeadTrackingDeviceState(bool outputDeviceChange, std::string preDeviceAddress)
{
    std::unordered_map<std::string, bool> headTrackingDeviceChangeInfo;
    if (outputDeviceChange && !preDeviceAddress.empty() && isHeadTrackingDataRequested_) {
        headTrackingDeviceChangeInfo.insert(std::make_pair(preDeviceAddress, false));
    }

    bool isRequested = IsHeadTrackingDataRequestedForCurrentDevice();
    if (!currentDeviceAddress_.empty() &&
        ((!outputDeviceChange && (isHeadTrackingDataRequested_ != isRequested)) ||
        (outputDeviceChange && isRequested))) {
        headTrackingDeviceChangeInfo.insert(std::make_pair(currentDeviceAddress_, isRequested));
    }

    isHeadTrackingDataRequested_ = isRequested;

    HandleHeadTrackingDeviceChange(headTrackingDeviceChangeInfo);
}

void AudioSpatializationService::HandleHeadTrackingDeviceChange(const std::unordered_map<std::string, bool> &changeInfo)
{
    AUDIO_DEBUG_LOG("callback is triggered, change info size is %{public}zu", changeInfo.size());

    if (changeInfo.size() == 0) {
        return;
    }

    if (audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendHeadTrackingDeviceChangeEvent(changeInfo);
    }
}
} // namespace AudioStandard
} // namespace OHOS
