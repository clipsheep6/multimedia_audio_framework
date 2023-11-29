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

#include "audio_policy_server.h"

#include <csignal>
#include <memory>
#include <unordered_set>
#include <vector>
#include <condition_variable>

#ifdef FEATURE_MULTIMODALINPUT_INPUT
#include "input_manager.h"
#include "key_event.h"
#include "key_option.h"
#endif
#include "power_mgr_client.h"

#include "privacy_kit.h"
#include "accesstoken_kit.h"
#include "permission_state_change_info.h"
#include "token_setproc.h"
#include "tokenid_kit.h"

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_policy_manager_listener_proxy.h"
#include "i_standard_audio_policy_manager_listener.h"
#include "microphone_descriptor.h"
#include "parameter.h"
#include "parameters.h"

using OHOS::Security::AccessToken::PrivacyKit;
using OHOS::Security::AccessToken::TokenIdKit;
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr float DUCK_FACTOR = 0.2f; // 20%
constexpr int32_t PARAMS_VOLUME_NUM = 5;
constexpr int32_t PARAMS_INTERRUPT_NUM = 4;
constexpr int32_t PARAMS_RENDER_STATE_NUM = 2;
constexpr int32_t EVENT_DES_SIZE = 60;
constexpr int32_t ADAPTER_STATE_CONTENT_DES_SIZE = 60;
constexpr uid_t UID_ROOT = 0;
constexpr uid_t UID_MSDP_SA = 6699;
constexpr uid_t UID_INTELLIGENT_VOICE_SA = 1042;
constexpr uid_t UID_CAST_ENGINE_SA = 5526;
constexpr uid_t UID_CAAS_SA = 5527;
constexpr uid_t UID_DISTRIBUTED_AUDIO_SA = 3055;
constexpr uid_t UID_MEDIA_SA = 1013;
constexpr uid_t UID_AUDIO = 1041;
constexpr uid_t UID_FOUNDATION_SA = 5523;
constexpr uid_t UID_BLUETOOTH_SA = 1002;

REGISTER_SYSTEM_ABILITY_BY_ID(AudioPolicyServer, AUDIO_POLICY_SERVICE_ID, true)

const map<InterruptHint, AudioFocuState> AudioPolicyServer::HINTSTATEMAP = AudioPolicyServer::CreateStateMap();

map<InterruptHint, AudioFocuState> AudioPolicyServer::CreateStateMap()
{
    map<InterruptHint, AudioFocuState> stateMap;
    stateMap[INTERRUPT_HINT_PAUSE] = PAUSE;
    stateMap[INTERRUPT_HINT_DUCK] = DUCK;
    stateMap[INTERRUPT_HINT_NONE] = ACTIVE;
    stateMap[INTERRUPT_HINT_RESUME] = ACTIVE;
    stateMap[INTERRUPT_HINT_UNDUCK] = ACTIVE;

    return stateMap;
}

const std::list<uid_t> AudioPolicyServer::RECORD_ALLOW_BACKGROUND_LIST = {
    UID_ROOT,
    UID_MSDP_SA,
    UID_INTELLIGENT_VOICE_SA,
    UID_CAAS_SA,
    UID_DISTRIBUTED_AUDIO_SA,
    UID_AUDIO,
    UID_FOUNDATION_SA
};

const std::list<uid_t> AudioPolicyServer::RECORD_PASS_APPINFO_LIST = {
    UID_MEDIA_SA,
    UID_CAST_ENGINE_SA
};

AudioPolicyServer::AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate),
      audioPolicyService_(AudioPolicyService::GetAudioPolicyService()),
      audioSpatializationService_(AudioSpatializationService::GetAudioSpatializationService())
{
    if (audioPolicyService_.SetAudioSessionCallback(this)) {
        AUDIO_DEBUG_LOG("AudioPolicyServer: SetAudioSessionCallback failed");
    }

    volumeStep_ = system::GetIntParameter("const.multimedia.audio.volumestep", 1);
    AUDIO_INFO_LOG("Get volumeStep parameter success %{public}d", volumeStep_);

    clientOnFocus_ = 0;
    focussedAudioInterruptInfo_ = nullptr;
    powerStateCallbackRegister_ = false;
}

void AudioPolicyServer::OnDump()
{
    return;
}

void AudioPolicyServer::OnStart()
{
    AUDIO_INFO_LOG("AudioPolicyServer OnStart");
    audioPolicyService_.Init();
    AddSystemAbilityListener(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    AddSystemAbilityListener(MULTIMODAL_INPUT_SERVICE_ID);
#endif
    AddSystemAbilityListener(AUDIO_DISTRIBUTED_SERVICE_ID);
    AddSystemAbilityListener(BLUETOOTH_HOST_SYS_ABILITY_ID);
    AddSystemAbilityListener(ACCESSIBILITY_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(POWER_MANAGER_SERVICE_ID);

    bool res = Publish(this);
    if (!res) {
        AUDIO_INFO_LOG("AudioPolicyServer start err");
    }

    Security::AccessToken::PermStateChangeScope scopeInfo;
    scopeInfo.permList = {"ohos.permission.MICROPHONE"};
    auto callbackPtr = std::make_shared<PerStateChangeCbCustomizeCallback>(scopeInfo, this);
    callbackPtr->ready_ = false;
    int32_t iRes = Security::AccessToken::AccessTokenKit::RegisterPermStateChangeCallback(callbackPtr);
    if (iRes < 0) {
        AUDIO_ERR_LOG("fail to call RegisterPermStateChangeCallback.");
    }

#ifdef FEATURE_MULTIMODALINPUT_INPUT
    SubscribeVolumeKeyEvents();
#endif
}

void AudioPolicyServer::OnStop()
{
    audioPolicyService_.Deinit();
    UnRegisterPowerStateListener();
    return;
}

void AudioPolicyServer::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    AUDIO_DEBUG_LOG("OnAddSystemAbility systemAbilityId:%{public}d", systemAbilityId);
    switch (systemAbilityId) {
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        case MULTIMODAL_INPUT_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility input service start");
            SubscribeVolumeKeyEvents();
            break;
#endif
        case DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility kv data service start");
            InitKVStore();
            RegisterDataObserver();
            break;
        case AUDIO_DISTRIBUTED_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility audio service start");
            if (!isFirstAudioServiceStart_) {
                ConnectServiceAdapter();
                sessionProcessor_.Start();
                RegisterParamCallback();
                LoadEffectLibrary();
                isFirstAudioServiceStart_ = true;
            } else {
                AUDIO_ERR_LOG("OnAddSystemAbility audio service is not first start");
            }
            break;
        case BLUETOOTH_HOST_SYS_ABILITY_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility bluetooth service start");
            RegisterBluetoothListener();
            break;
        case ACCESSIBILITY_MANAGER_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility accessibility service start");
            SubscribeAccessibilityConfigObserver();
            InitKVStore();
            RegisterDataObserver();
            break;
        case POWER_MANAGER_SERVICE_ID:
            AUDIO_INFO_LOG("OnAddSystemAbility power manager service start");
            SubscribePowerStateChangeEvents();
            RegisterPowerStateListener();
            break;
        default:
            AUDIO_ERR_LOG("OnAddSystemAbility unhandled sysabilityId:%{public}d", systemAbilityId);
            break;
    }
}

void AudioPolicyServer::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    AUDIO_DEBUG_LOG("AudioPolicyServer::OnRemoveSystemAbility systemAbilityId:%{public}d removed", systemAbilityId);
}

#ifdef FEATURE_MULTIMODALINPUT_INPUT
bool AudioPolicyServer::MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType,
    const AudioStreamType &streamInFocus)
{
    bool volLevelCheck = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ?
        volLevel >= GetMaxVolumeLevel(streamInFocus) : volLevel <= GetMinVolumeLevel(streamInFocus);
    if (volLevelCheck) {
        for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
            sptr<IAudioPolicyClient> volumeChangeCb = it->second;
            if (volumeChangeCb == nullptr) {
                AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
                continue;
            }

            if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) {
                AUDIO_INFO_LOG("---------volume greater than max, trigger cb clientPid : %{public}d", it->first);
            } else {
                AUDIO_INFO_LOG("---------volume lower than min, trigger cb clientPid : %{public}d", it->first);
            }
            VolumeEvent volumeEvent;
            volumeEvent.volumeType = (streamInFocus == STREAM_ALL) ? STREAM_MUSIC : streamInFocus;
            volumeEvent.volume = volLevel;
            volumeEvent.updateUi = true;
            volumeEvent.volumeGroupId = 0;
            volumeEvent.networkId = LOCAL_NETWORK_ID;
            volumeChangeCb->OnVolumeKeyEvent(volumeEvent);
        }
        return true;
    }

    return false;
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
void AudioPolicyServer::RegisterVolumeKeyEvents(const int32_t keyType)
{
    if ((keyType != OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) && (keyType != OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN)) {
        AUDIO_ERR_LOG("VolumeKeyEvents: invalid key type : %{public}d", keyType);
        return;
    }
    AUDIO_INFO_LOG("RegisterVolumeKeyEvents: volume key: %{public}s.",
        (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");

    MMI::InputManager *im = MMI::InputManager::GetInstance();
    CHECK_AND_RETURN_LOG(im != nullptr, "Failed to obtain INPUT manager");

    std::set<int32_t> preKeys;
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_LOG(keyOption != nullptr, "Invalid key option");
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(keyType);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(VOLUME_KEY_DURATION);
    int32_t keySubId = im->SubscribeKeyEvent(keyOption, [=](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
        AUDIO_INFO_LOG("Receive volume key event: %{public}s.",
            (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
        std::lock_guard<std::mutex> lock(volumeKeyEventMutex_);
        AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
        if (audioPolicyService_.GetLocalDevicesType().compare("2in1") == 0) {
            streamInFocus = AudioStreamType::STREAM_ALL;
        } else {
            streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
        }
        if (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP && GetStreamMuteInternal(streamInFocus)) {
            AUDIO_INFO_LOG("VolumeKeyEvents: volumeKey: Up. volumeType %{public}d is mute. Unmute.", streamInFocus);
            SetStreamMuteInternal(streamInFocus, false, true);
            return;
        }
        int32_t volumeLevelInInt = GetSystemVolumeLevelInternal(streamInFocus, false);
        if (MaxOrMinVolumeOption(volumeLevelInInt, keyType, streamInFocus)) {
            return;
        }

        volumeLevelInInt = (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ?
            ++volumeLevelInInt : --volumeLevelInInt;
        SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt, true);
    });
    if (keySubId < 0) {
        AUDIO_ERR_LOG("SubscribeKeyEvent: subscribing for volume key: %{public}s option failed",
            (keyType == OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP) ? "up" : "down");
    }
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
void AudioPolicyServer::RegisterVolumeKeyMuteEvents()
{
    AUDIO_INFO_LOG("RegisterVolumeKeyMuteEvents: volume key: mute");
    MMI::InputManager *im = MMI::InputManager::GetInstance();
    CHECK_AND_RETURN_LOG(im != nullptr, "Failed to obtain INPUT manager");

    std::shared_ptr<OHOS::MMI::KeyOption> keyOptionMute = std::make_shared<OHOS::MMI::KeyOption>();
    CHECK_AND_RETURN_LOG(keyOptionMute != nullptr, "keyOptionMute: Invalid key option");
    std::set<int32_t> preKeys;
    keyOptionMute->SetPreKeys(preKeys);
    keyOptionMute->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_MUTE);
    keyOptionMute->SetFinalKeyDown(true);
    keyOptionMute->SetFinalKeyDownDuration(VOLUME_MUTE_KEY_DURATION);
    int32_t muteKeySubId = im->SubscribeKeyEvent(keyOptionMute,
        [this](std::shared_ptr<MMI::KeyEvent> keyEventCallBack) {
            AUDIO_INFO_LOG("Receive volume key event: mute");
            std::lock_guard<std::mutex> lock(volumeKeyEventMutex_);
            bool isMuted = GetStreamMute(AudioStreamType::STREAM_ALL);
            SetStreamMuteInternal(AudioStreamType::STREAM_ALL, !isMuted, true);
        });
    if (muteKeySubId < 0) {
        AUDIO_ERR_LOG("SubscribeKeyEvent: subscribing for mute failed ");
    }
}
#endif

#ifdef FEATURE_MULTIMODALINPUT_INPUT
void AudioPolicyServer::SubscribeVolumeKeyEvents()
{
    if (hasSubscribedVolumeKeyEvents_.load()) {
        AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: volume key events has been sunscirbed!");
        return;
    }

    AUDIO_INFO_LOG("SubscribeVolumeKeyEvents: first time.");
    hasSubscribedVolumeKeyEvents_.store(true);
    RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_UP);
    RegisterVolumeKeyEvents(OHOS::MMI::KeyEvent::KEYCODE_VOLUME_DOWN);
    RegisterVolumeKeyMuteEvents();
}
#endif

AudioVolumeType AudioPolicyServer::GetVolumeTypeFromStreamType(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_MESSAGE:
            return STREAM_VOICE_CALL;
        case STREAM_RING:
        case STREAM_SYSTEM:
        case STREAM_NOTIFICATION:
        case STREAM_SYSTEM_ENFORCED:
        case STREAM_DTMF:
            return STREAM_RING;
        case STREAM_MUSIC:
        case STREAM_MEDIA:
        case STREAM_MOVIE:
        case STREAM_GAME:
        case STREAM_SPEECH:
        case STREAM_NAVIGATION:
            return STREAM_MUSIC;
        case STREAM_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_ALARM:
            return STREAM_ALARM;
        case STREAM_ACCESSIBILITY:
            return STREAM_ACCESSIBILITY;
        case STREAM_ULTRASONIC:
            return STREAM_ULTRASONIC;
        case STREAM_ALL:
            return STREAM_ALL;
        default:
            return STREAM_MUSIC;
    }
}

bool AudioPolicyServer::IsVolumeTypeValid(AudioStreamType streamType)
{
    bool result = false;
    switch (streamType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_NOTIFICATION:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
        case STREAM_ULTRASONIC:
        case STREAM_ALL:
            result = true;
            break;
        default:
            result = false;
            AUDIO_ERR_LOG("IsVolumeTypeValid: streamType[%{public}d] is not supported", streamType);
            break;
    }
    return result;
}

bool AudioPolicyServer::IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel)
{
    bool result = true;
    if (volumeLevel < audioPolicyService_.GetMinVolumeLevel(streamType) ||
        volumeLevel > audioPolicyService_.GetMaxVolumeLevel(streamType)) {
        AUDIO_ERR_LOG("IsVolumeLevelValid: volumeLevel[%{public}d] is out of valid range for streamType[%{public}d]",
            volumeLevel, streamType);
        result = false;
    }
    return result;
}

void AudioPolicyServer::SubscribePowerStateChangeEvents()
{
    sptr<PowerMgr::IPowerStateCallback> powerStateCallback_;

    if (powerStateCallback_ == nullptr) {
        powerStateCallback_ = new (std::nothrow) AudioPolicyServerPowerStateCallback(this);
    }
    
    if (powerStateCallback_ == nullptr) {
        AUDIO_ERR_LOG("subscribe create power state callback Create Error");
        return;
    }
    
    bool RegisterSuccess = PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback(powerStateCallback_);
    if (!RegisterSuccess) {
        AUDIO_ERR_LOG("register power state callback failed");
    } else {
        AUDIO_INFO_LOG("register power state callback success");
        powerStateCallbackRegister_ = true;
    }
}

void AudioPolicyServer::CheckSubscribePowerStateChange()
{
    if (!powerStateCallbackRegister_) {
        SubscribePowerStateChangeEvents();
    }

    if (!powerStateCallbackRegister_) {
        AUDIO_ERR_LOG("PowerState CallBack Register Failed");
    } else {
        AUDIO_DEBUG_LOG("PowerState CallBack Register Success");
    }
}

void AudioPolicyServer::HandlePowerStateChanged(PowerMgr::PowerState state)
{
    audioPolicyService_.HandlePowerStateChanged(state);
}

int32_t AudioPolicyServer::SetOffloadStream(uint32_t sessionId)
{
    CheckSubscribePowerStateChange();
    return audioPolicyService_.SetOffloadStream(sessionId);
}

int32_t AudioPolicyServer::ReleaseOffloadStream(uint32_t sessionId)
{
    return audioPolicyService_.ReleaseOffloadStream(sessionId);
}

void AudioPolicyServer::InterruptOffload(uint32_t activeSessionId, AudioStreamType incomingStreamType,
    uint32_t incomingSessionId)
{
    ReleaseOffloadStream(activeSessionId);
    if ((incomingStreamType == AudioStreamType::STREAM_MUSIC) ||
        (incomingStreamType == AudioStreamType::STREAM_SPEECH)) {
        SetOffloadStream(incomingSessionId);
    } else {
        AUDIO_DEBUG_LOG("session:%{public}d not get offload stream type is %{public}d", incomingSessionId,
            incomingStreamType);
    }
}

AudioPolicyServer::AudioPolicyServerPowerStateCallback::AudioPolicyServerPowerStateCallback(
    AudioPolicyServer* policyServer) : PowerMgr::PowerStateCallbackStub(), policyServer_(policyServer)
{}

void AudioPolicyServer::AudioPolicyServerPowerStateCallback::OnPowerStateChanged(PowerMgr::PowerState state)
{
    policyServer_->HandlePowerStateChanged(state);
}

void AudioPolicyServer::InitKVStore()
{
    audioPolicyService_.InitKVStore();
}

void AudioPolicyServer::ConnectServiceAdapter()
{
    if (!audioPolicyService_.ConnectServiceAdapter()) {
        AUDIO_ERR_LOG("ConnectServiceAdapter Error in connecting to audio service adapter");
        return;
    }
}

void AudioPolicyServer::LoadEffectLibrary()
{
    audioPolicyService_.LoadEffectLibrary();
}

int32_t AudioPolicyServer::GetMaxVolumeLevel(AudioVolumeType volumeType)
{
    return audioPolicyService_.GetMaxVolumeLevel(volumeType);
}

int32_t AudioPolicyServer::GetMinVolumeLevel(AudioVolumeType volumeType)
{
    return audioPolicyService_.GetMinVolumeLevel(volumeType);
}

int32_t AudioPolicyServer::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel, API_VERSION api_v)
{
    if (api_v == API_9 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetSystemVolumeLevel: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    if (!IsVolumeTypeValid(streamType)) {
        return ERR_NOT_SUPPORTED;
    }
    if (!IsVolumeLevelValid(streamType, volumeLevel)) {
        return ERR_NOT_SUPPORTED;
    }

    return SetSystemVolumeLevelInternal(streamType, volumeLevel, false);
}

int32_t AudioPolicyServer::GetSystemVolumeLevel(AudioStreamType streamType)
{
    return GetSystemVolumeLevelInternal(streamType, false);
}

int32_t AudioPolicyServer::GetSystemVolumeLevelInternal(AudioStreamType streamType, bool isFromVolumeKey)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
        AUDIO_DEBUG_LOG("GetVolume of STREAM_ALL for streamType = %{public}d ", streamType);
    }
    return audioPolicyService_.GetSystemVolumeLevel(streamType, isFromVolumeKey);
}

int32_t AudioPolicyServer::SetLowPowerVolume(int32_t streamId, float volume)
{
    return audioPolicyService_.SetLowPowerVolume(streamId, volume);
}

float AudioPolicyServer::GetLowPowerVolume(int32_t streamId)
{
    return audioPolicyService_.GetLowPowerVolume(streamId);
}

float AudioPolicyServer::GetSingleStreamVolume(int32_t streamId)
{
    return audioPolicyService_.GetSingleStreamVolume(streamId);
}

bool AudioPolicyServer::IsVolumeUnadjustable()
{
    return audioPolicyService_.IsVolumeUnadjustable();
}

int32_t AudioPolicyServer::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    AudioStreamType streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
    if (streamInFocus == AudioStreamType::STREAM_DEFAULT) {
        streamInFocus = AudioStreamType::STREAM_MUSIC;
    }

    int32_t volumeLevelInInt = GetSystemVolumeLevel(streamInFocus);
    int32_t ret = ERROR;
    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt + volumeStep_, false);
        AUDIO_INFO_LOG("AdjustVolumeByStep Up, VolumeLevel is %{public}d", GetSystemVolumeLevel(streamInFocus));
    }

    if (adjustType == VolumeAdjustType::VOLUME_DOWN) {
        ret = SetSystemVolumeLevelInternal(streamInFocus, volumeLevelInInt - volumeStep_, false);
        AUDIO_INFO_LOG("AdjustVolumeByStep Down, VolumeLevel is %{public}d", GetSystemVolumeLevel(streamInFocus));
    }
    return ret;
}

int32_t AudioPolicyServer::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    int32_t volumeLevelInInt = GetSystemVolumeLevel(volumeType);
    int32_t ret = ERROR;

    if (adjustType == VolumeAdjustType::VOLUME_UP) {
        ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt + volumeStep_, false);
        AUDIO_INFO_LOG("AdjustSystemVolumeByStep Up, VolumeLevel:%{public}d", GetSystemVolumeLevel(volumeType));
    }

    if (adjustType == VolumeAdjustType::VOLUME_DOWN) {
        ret = SetSystemVolumeLevelInternal(volumeType, volumeLevelInInt - volumeStep_, false);
        AUDIO_INFO_LOG("AdjustSystemVolumeByStep Down, VolumeLevel:%{public}d", GetSystemVolumeLevel(volumeType));
    }
    return ret;
}

float AudioPolicyServer::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    if (!IsVolumeTypeValid(volumeType)) {
        return static_cast<float>(ERR_INVALID_PARAM);
    }
    if (!IsVolumeLevelValid(volumeType, volumeLevel)) {
        return static_cast<float>(ERR_INVALID_PARAM);
    }

    return audioPolicyService_.GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

int32_t AudioPolicyServer::SetStreamMute(AudioStreamType streamType, bool mute, API_VERSION api_v)
{
    if (api_v == API_9 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetStreamMute: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return SetStreamMuteInternal(streamType, mute, false);
}

int32_t AudioPolicyServer::SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetStreamMuteInternal streamType: %{public}d, mute: %{public}d, updateUi: %{public}d",
        streamType, mute, isUpdateUi);

    if (streamType == STREAM_ALL) {
        for (auto audioStreamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetMute of STREAM_ALL for StreamType = %{public}d ", audioStreamType);
            int32_t setResult = SetSingleStreamMute(audioStreamType, mute, isUpdateUi);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }

    return SetSingleStreamMute(streamType, mute, isUpdateUi);
}

int32_t AudioPolicyServer::SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi)
{
    if (streamType == AudioStreamType::STREAM_RING && !isUpdateUi) {
        if (!VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
            AUDIO_ERR_LOG("SetStreamMute permission denied for stream type : %{public}d", streamType);
            return ERR_PERMISSION_DENIED;
        }
    }

    int result = audioPolicyService_.SetStreamMute(streamType, mute);
    for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
        sptr<IAudioPolicyClient> volumeChangeCb = it->second;
        if (volumeChangeCb == nullptr) {
            AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
            continue;
        }
        AUDIO_INFO_LOG("-----SetStreamMute trigger volumeChangeCb clientPid: %{public}d, type: %{public}d",
            it->first, streamType);
        VolumeEvent volumeEvent;
        volumeEvent.volumeType = streamType;
        volumeEvent.volume = GetSystemVolumeLevel(streamType);
        volumeEvent.updateUi = isUpdateUi;
        volumeEvent.volumeGroupId = 0;
        volumeEvent.networkId = LOCAL_NETWORK_ID;
        volumeChangeCb->OnVolumeKeyEvent(volumeEvent);
    }

    return result;
}

float AudioPolicyServer::GetSystemVolumeDb(AudioStreamType streamType)
{
    return audioPolicyService_.GetSystemVolumeDb(streamType);
}

int32_t AudioPolicyServer::SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel,
    bool isUpdateUi)
{
    AUDIO_INFO_LOG("SetSystemVolumeLevelInternal streamType: %{public}d, volumeLevel: %{public}d, updateUi: %{public}d",
        streamType, volumeLevel, isUpdateUi);
    if (IsVolumeUnadjustable()) {
        AUDIO_ERR_LOG("Unadjustable device, not allow set volume");
        return ERR_OPERATION_FAILED;
    }
    if (streamType == STREAM_ALL) {
        for (auto audioSteamType : GET_STREAM_ALL_VOLUME_TYPES) {
            AUDIO_INFO_LOG("SetVolume of STREAM_ALL, SteamType = %{public}d ", audioSteamType);
            int32_t setResult = SetSingleStreamVolume(audioSteamType, volumeLevel, isUpdateUi);
            if (setResult != SUCCESS) {
                return setResult;
            }
        }
        return SUCCESS;
    }
    return SetSingleStreamVolume(streamType, volumeLevel, isUpdateUi);
}

int32_t AudioPolicyServer::SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi)
{
    if ((streamType == AudioStreamType::STREAM_RING) && !isUpdateUi) {
        int32_t curRingVolumeLevel = GetSystemVolumeLevel(STREAM_RING);
        if ((curRingVolumeLevel > 0 && volumeLevel == 0) || (curRingVolumeLevel == 0 && volumeLevel > 0)) {
            if (!VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
                AUDIO_ERR_LOG("Access policy permission denied for volume type : %{public}d", streamType);
                return ERR_PERMISSION_DENIED;
            }
        }
    }

    int ret = audioPolicyService_.SetSystemVolumeLevel(streamType, volumeLevel, isUpdateUi);
    for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
        sptr<IAudioPolicyClient> volumeChangeCb = it->second;
        if (volumeChangeCb == nullptr) {
            AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
            continue;
        }

        AUDIO_INFO_LOG("--------SetSystemVolumeLevelInternal trigger volumeChangeCb clientPid : %{public}d", it->first);
        VolumeEvent volumeEvent;
        volumeEvent.volumeType = streamType;
        volumeEvent.volume = GetSystemVolumeLevel(streamType);
        volumeEvent.updateUi = isUpdateUi;
        volumeEvent.volumeGroupId = 0;
        volumeEvent.networkId = LOCAL_NETWORK_ID;
        volumeChangeCb->OnVolumeKeyEvent(volumeEvent);
    }

    return ret;
}

bool AudioPolicyServer::GetStreamMute(AudioStreamType streamType)
{
    if (streamType == AudioStreamType::STREAM_RING) {
        if (!VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
            AUDIO_ERR_LOG("GetStreamMute permission denied for stream type : %{public}d", streamType);
            return false;
        }
    }

    return GetStreamMuteInternal(streamType);
}

bool AudioPolicyServer::GetStreamMuteInternal(AudioStreamType streamType)
{
    if (streamType == STREAM_ALL) {
        streamType = STREAM_MUSIC;
        AUDIO_INFO_LOG("GetStreamMute of STREAM_ALL for streamType = %{public}d ", streamType);
    }
    return audioPolicyService_.GetStreamMute(streamType);
}

int32_t AudioPolicyServer::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SelectOutputDevice: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    return audioPolicyService_.SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
}

std::string AudioPolicyServer::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType)
{
    return audioPolicyService_.GetSelectedDeviceInfo(uid, pid, streamType);
}

int32_t AudioPolicyServer::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SelectInputDevice: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret = audioPolicyService_.SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
    return ret;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetDevices(DeviceFlag deviceFlag)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (deviceFlag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("GetDevices: No system permission");
                std::vector<sptr<AudioDeviceDescriptor>> info = {};
                return info;
            }
            break;
        default:
            break;
    }

    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs = audioPolicyService_.GetDevices(deviceFlag);

    if (!hasSystemPermission) {
        for (sptr<AudioDeviceDescriptor> desc : deviceDescs) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

int32_t AudioPolicyServer::SetWakeUpAudioCapturer(InternalAudioCapturerOptions options)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("SetWakeUpAudioCapturer: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    bool hasManageIntellgentPermission = VerifyPermission(MANAGE_INTELLIGENT_VOICE_PERMISSION);
    if (!hasManageIntellgentPermission) {
        AUDIO_ERR_LOG("SetWakeUpAudioCapturer: No permission");
        return ERR_PERMISSION_DENIED;
    }
    return audioPolicyService_.SetWakeUpAudioCapturer(options);
}

int32_t AudioPolicyServer::VerifyVoiceCallPermission()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("VerifyVoiceCallPermission: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    bool hasRecordVoiceCallPermission = VerifyPermission(RECORD_VOICE_CALL_PERMISSION);
    if (!hasRecordVoiceCallPermission) {
        AUDIO_ERR_LOG("VerifyVoiceCallPermission: No permission");
        return ERR_PERMISSION_DENIED;
    }
    return SUCCESS;
}

int32_t AudioPolicyServer::CloseWakeUpAudioCapturer()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        AUDIO_ERR_LOG("CloseWakeUpAudioCapturer: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    bool hasManageIntellgentPermission = VerifyPermission(MANAGE_INTELLIGENT_VOICE_PERMISSION);
    if (!hasManageIntellgentPermission) {
        AUDIO_ERR_LOG("CloseWakeUpAudioCapturer: No permission");
        return ERR_PERMISSION_DENIED;
    }
    auto res = audioPolicyService_.CloseWakeUpAudioCapturer();
    return res;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetPreferredOutputDeviceDescriptors(rendererInfo);
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyServer::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo)
{
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescs =
        audioPolicyService_.GetPreferredInputDeviceDescriptors(captureInfo);
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDescs);
    }

    return deviceDescs;
}

bool AudioPolicyServer::IsStreamActive(AudioStreamType streamType)
{
    return audioPolicyService_.IsStreamActive(streamType);
}

int32_t AudioPolicyServer::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    return audioPolicyService_.SetDeviceActive(deviceType, active);
}

bool AudioPolicyServer::IsDeviceActive(InternalDeviceType deviceType)
{
    return audioPolicyService_.IsDeviceActive(deviceType);
}

InternalDeviceType AudioPolicyServer::GetActiveOutputDevice()
{
    return audioPolicyService_.GetActiveOutputDevice();
}

InternalDeviceType AudioPolicyServer::GetActiveInputDevice()
{
    return audioPolicyService_.GetActiveInputDevice();
}

int32_t AudioPolicyServer::SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v)
{
    if (api_v == API_9 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetRingerMode: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    bool isPermissionRequired = false;

    if (ringMode == AudioRingerMode::RINGER_MODE_SILENT) {
        isPermissionRequired = true;
    } else {
        AudioRingerMode currentRingerMode = GetRingerMode();
        if (currentRingerMode == AudioRingerMode::RINGER_MODE_SILENT) {
            isPermissionRequired = true;
        }
    }

    if (isPermissionRequired) {
        if (!VerifyPermission(ACCESS_NOTIFICATION_POLICY_PERMISSION)) {
            AUDIO_ERR_LOG("Access policy permission denied for ringerMode : %{public}d", ringMode);
            return ERR_PERMISSION_DENIED;
        }
    }

    int32_t ret = audioPolicyService_.SetRingerMode(ringMode);
    
    std::lock_guard<std::mutex> lock(ringerModeMutex_);
    if (ret == SUCCESS) {
        for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
            sptr<IAudioPolicyClient> ringerModeListenerCb = it->second;
            if (ringerModeListenerCb == nullptr) {
                AUDIO_ERR_LOG("ringerModeListenerCb nullptr for client %{public}d", it->first);
                continue;
            }

            AUDIO_DEBUG_LOG("ringerModeListenerCb client %{public}d", it->first);
            ringerModeListenerCb->OnRingerModeUpdated(ringMode);
        }
    }

    return ret;
}

#ifdef FEATURE_DTMF_TONE
std::shared_ptr<ToneInfo> AudioPolicyServer::GetToneConfig(int32_t ltonetype)
{
    return audioPolicyService_.GetToneConfig(ltonetype);
}

std::vector<int32_t> AudioPolicyServer::GetSupportedTones()
{
    return audioPolicyService_.GetSupportedTones();
}
#endif

int32_t AudioPolicyServer::SetMicrophoneMuteCommon(bool isMute, API_VERSION api_v)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> lock(micStateChangeMutex_);
    bool isMicrophoneMute = IsMicrophoneMute(api_v);
    int32_t ret = audioPolicyService_.SetMicrophoneMute(isMute);
    if (ret == SUCCESS && isMicrophoneMute != isMute) {
        for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
            sptr<IAudioPolicyClient> micStateChangeListenerCb = it->second;
            if (micStateChangeListenerCb == nullptr) {
                AUDIO_ERR_LOG("callback is nullptr for client %{public}d", it->first);
                continue;
            }
            MicStateChangeEvent micStateChangeEvent;
            micStateChangeEvent.mute = isMute;
            micStateChangeListenerCb->OnMicStateUpdated(micStateChangeEvent);
        }
    }
    return ret;
}

int32_t AudioPolicyServer::SetMicrophoneMute(bool isMute)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (!VerifyPermission(MICROPHONE_PERMISSION)) {
        AUDIO_ERR_LOG("SetMicrophoneMute: MICROPHONE permission denied");
        return ERR_PERMISSION_DENIED;
    }
    return SetMicrophoneMuteCommon(isMute, API_7);
}

int32_t AudioPolicyServer::SetMicrophoneMuteAudioConfig(bool isMute)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (!VerifyPermission(MANAGE_AUDIO_CONFIG)) {
        AUDIO_ERR_LOG("SetMicrophoneMuteAudioConfig: MANAGE_AUDIO_CONFIG permission denied");
        return ERR_PERMISSION_DENIED;
    }
    return SetMicrophoneMuteCommon(isMute, API_9);
}

bool AudioPolicyServer::IsMicrophoneMute(API_VERSION api_v)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    if (api_v == API_7 && !VerifyPermission(MICROPHONE_PERMISSION)) {
        AUDIO_ERR_LOG("IsMicrophoneMute: MICROPHONE permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return audioPolicyService_.IsMicrophoneMute();
}

AudioRingerMode AudioPolicyServer::GetRingerMode()
{
    return audioPolicyService_.GetRingerMode();
}

int32_t AudioPolicyServer::SetAudioScene(AudioScene audioScene)
{
    if (audioScene <= AUDIO_SCENE_INVALID || audioScene >= AUDIO_SCENE_MAX) {
        AUDIO_ERR_LOG("SetAudioScene: param is invalid");
        return ERR_INVALID_PARAM;
    }

    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetAudioScene: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    return audioPolicyService_.SetAudioScene(audioScene);
}

AudioScene AudioPolicyServer::GetAudioScene()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    return audioPolicyService_.GetAudioScene(hasSystemPermission);
}

int32_t AudioPolicyServer::SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(interruptMutex_);

    auto callerUid = IPCSkeleton::GetCallingUid();
    if (!audioPolicyService_.IsSessionIdValid(callerUid, sessionID)) {
        AUDIO_ERR_LOG("SetAudioInterruptCallback for sessionID %{public}d, id is invalid", sessionID);
        return ERR_INVALID_PARAM;
    }

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "SetAudioInterruptCallback object is nullptr");

    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "SetAudioInterruptCallback obj cast failed");

    std::shared_ptr<AudioInterruptCallback> callback = std::make_shared<AudioPolicyManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "SetAudioInterruptCallback create cb failed");

    interruptCbsMap_[sessionID] = callback;
    AUDIO_DEBUG_LOG("SetAudioInterruptCallback for sessionID %{public}d done", sessionID);

    return SUCCESS;
}

int32_t AudioPolicyServer::UnsetAudioInterruptCallback(const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(interruptMutex_);

    if (interruptCbsMap_.erase(sessionID) == 0) {
        AUDIO_ERR_LOG("UnsetAudioInterruptCallback session %{public}d not present", sessionID);
        return ERR_INVALID_OPERATION;
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::SetAudioManagerInterruptCallback(const int32_t /* clientId */,
                                                            const sptr<IRemoteObject> &object)
{
    std::lock_guard<std::mutex> lock(amInterruptMutex_);

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetAudioManagerInterruptCallback object is nullptr");

    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetAudioManagerInterruptCallback obj cast failed");

    std::shared_ptr<AudioInterruptCallback> callback = std::make_shared<AudioPolicyManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetAudioManagerInterruptCallback create cb failed");

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    amInterruptCbsMap_[clientPid] = callback;
    AUDIO_INFO_LOG("SetAudioManagerInterruptCallback for client id %{public}d done", clientPid);

    return SUCCESS;
}

int32_t AudioPolicyServer::UnsetAudioManagerInterruptCallback(const int32_t /* clientId */)
{
    std::lock_guard<std::mutex> lock(amInterruptMutex_);
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    if (amInterruptCbsMap_.erase(clientPid) == 0) {
        AUDIO_ERR_LOG("UnsetAudioManagerInterruptCallback client %{public}d not present", clientPid);
        return ERR_INVALID_OPERATION;
    }

    return SUCCESS;
}

int32_t AudioPolicyServer::RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("RequestAudioFocus in");
    std::lock_guard<std::recursive_mutex> lock(focussedAudioInterruptInfoMutex_);
    if (clientOnFocus_ == clientId) {
        AUDIO_INFO_LOG("client already has focus");
        NotifyFocusGranted(clientId, audioInterrupt);
        return SUCCESS;
    }

    if (focussedAudioInterruptInfo_ != nullptr) {
        AUDIO_INFO_LOG("Existing stream: %{public}d, incoming stream: %{public}d",
            (focussedAudioInterruptInfo_->audioFocusType).streamType, audioInterrupt.audioFocusType.streamType);
        NotifyFocusAbandoned(clientOnFocus_, *focussedAudioInterruptInfo_);
        AbandonAudioFocus(clientOnFocus_, *focussedAudioInterruptInfo_);
    }

    NotifyFocusGranted(clientId, audioInterrupt);

    return SUCCESS;
}

int32_t AudioPolicyServer::AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("AudioPolicyServer: AbandonAudioFocus in");

    std::lock_guard<std::recursive_mutex> lock(focussedAudioInterruptInfoMutex_);
    if (clientId == clientOnFocus_) {
        AUDIO_DEBUG_LOG("AudioPolicyServer: remove app focus");
        focussedAudioInterruptInfo_.reset();
        focussedAudioInterruptInfo_ = nullptr;
        clientOnFocus_ = 0;
    }

    return SUCCESS;
}

void AudioPolicyServer::NotifyFocusGranted(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("Notify focus granted in: %{public}d", clientId);
    if (amInterruptCbsMap_.find(clientId) == amInterruptCbsMap_.end()) {
        AUDIO_ERR_LOG("Notify focus granted in: %{public}d failed, callback does not exist", clientId);
        return;
    }
    std::shared_ptr<AudioInterruptCallback> interruptCb = amInterruptCbsMap_[clientId];
    if (interruptCb == nullptr) {
        AUDIO_ERR_LOG("Notify focus granted in: %{public}d failed, callback is nullptr", clientId);
        return;
    } else {
        InterruptEventInternal interruptEvent = {};
        interruptEvent.eventType = INTERRUPT_TYPE_END;
        interruptEvent.forceType = INTERRUPT_SHARE;
        interruptEvent.hintType = INTERRUPT_HINT_NONE;
        interruptEvent.duckVolume = 0;

        AUDIO_DEBUG_LOG("callback focus granted");
        interruptCb->OnInterrupt(interruptEvent);

        unique_ptr<AudioInterrupt> tempAudioInterruptInfo = make_unique<AudioInterrupt>();
        tempAudioInterruptInfo->streamUsage = audioInterrupt.streamUsage;
        tempAudioInterruptInfo->contentType = audioInterrupt.contentType;
        (tempAudioInterruptInfo->audioFocusType).streamType = audioInterrupt.audioFocusType.streamType;
        tempAudioInterruptInfo->pauseWhenDucked = audioInterrupt.pauseWhenDucked;
        focussedAudioInterruptInfo_ = move(tempAudioInterruptInfo);
        clientOnFocus_ = clientId;
    }
}

int32_t AudioPolicyServer::NotifyFocusAbandoned(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("Notify focus abandoned in: %{public}d", clientId);
    std::shared_ptr<AudioInterruptCallback> interruptCb = nullptr;
    interruptCb = amInterruptCbsMap_[clientId];
    if (!interruptCb) {
        AUDIO_ERR_LOG("Notify failed, callback not present");
        return ERR_INVALID_PARAM;
    }

    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = INTERRUPT_TYPE_BEGIN;
    interruptEvent.forceType = INTERRUPT_SHARE;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    interruptEvent.duckVolume = 0;
    AUDIO_DEBUG_LOG("callback focus abandoned");
    interruptCb->OnInterrupt(interruptEvent);

    return SUCCESS;
}

bool AudioPolicyServer::IsSameAppInShareMode(const AudioInterrupt incomingInterrupt,
    const AudioInterrupt activateInterrupt)
{
    if (incomingInterrupt.mode != SHARE_MODE || activateInterrupt.mode != SHARE_MODE) {
        return false;
    }
    if (incomingInterrupt.pid == DEFAULT_APP_PID || activateInterrupt.pid == DEFAULT_APP_PID) {
        return false;
    }
    return incomingInterrupt.pid == activateInterrupt.pid;
}

void AudioPolicyServer::ProcessCurrentInterrupt(const AudioInterrupt &incomingInterrupt)
{
    auto focusMap = audioPolicyService_.GetAudioFocusMap();
    AudioFocusType incomingFocusType = incomingInterrupt.audioFocusType;
    for (auto iterActive = audioFocusInfoList_.begin(); iterActive != audioFocusInfoList_.end();) {
        if (IsSameAppInShareMode(incomingInterrupt, iterActive->first)) {
            ++iterActive;
            continue;
        }
        bool iterActiveErased = false;
        AudioFocusType activeFocusType = (iterActive->first).audioFocusType;
        std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
            std::make_pair(activeFocusType, incomingFocusType);
        AudioFocusEntry focusEntry = focusMap[audioFocusTypePair];
        if (iterActive->second == PAUSE || focusEntry.actionOn != CURRENT) {
            ++iterActive;
            continue;
        }
        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, focusEntry.forceType, focusEntry.hintType, 1.0f};
        uint32_t activeSessionID = (iterActive->first).sessionID;
        std::shared_ptr<AudioInterruptCallback> policyListenerCb = interruptCbsMap_[activeSessionID];

        switch (focusEntry.hintType) {
            case INTERRUPT_HINT_STOP:
                iterActive = audioFocusInfoList_.erase(iterActive);
                iterActiveErased = true;
                OnAudioFocusInfoChange();
                break;
            case INTERRUPT_HINT_PAUSE:
                iterActive->second = PAUSE;
                break;
            case INTERRUPT_HINT_DUCK:
                iterActive->second = DUCK;
                interruptEvent.duckVolume = DUCK_FACTOR * GetSystemVolumeDb(activeFocusType.streamType);
                break;
            default:
                break;
        }
        if (policyListenerCb != nullptr && interruptEvent.hintType != INTERRUPT_HINT_NONE) {
            AUDIO_INFO_LOG("OnInterrupt for active sessionID:%{public}d, hintType:%{public}d. By sessionID:%{public}d",
                activeSessionID, interruptEvent.hintType, incomingInterrupt.sessionID);
            policyListenerCb->OnInterrupt(interruptEvent);
            if (!iterActiveErased) {
                OnAudioFocusInfoChange();
            }
        }
        if (!iterActiveErased) {
            ++iterActive;
        }
        InterruptOffload(activeSessionID, incomingInterrupt.audioFocusType.streamType, incomingInterrupt.sessionID);
    }
}

int32_t AudioPolicyServer::ProcessFocusEntry(const AudioInterrupt &incomingInterrupt)
{
    auto focusMap = audioPolicyService_.GetAudioFocusMap();
    AudioFocuState incomingState = ACTIVE;
    std::shared_ptr<AudioInterruptCallback> policyListenerCb = interruptCbsMap_[incomingInterrupt.sessionID];
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_NONE, 1.0f};
    for (auto iterActive = audioFocusInfoList_.begin(); iterActive != audioFocusInfoList_.end(); ++iterActive) {
        if (IsSameAppInShareMode(incomingInterrupt, iterActive->first)) {
            continue;
        }
        std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
            std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType);

        CHECK_AND_RETURN_RET_LOG(focusMap.find(audioFocusTypePair) != focusMap.end(), ERR_INVALID_PARAM,
            "ProcessFocusEntry: audio focus type pair is invalid");
        AudioFocusEntry focusEntry = focusMap[audioFocusTypePair];
        if (iterActive->second == PAUSE || focusEntry.actionOn == CURRENT) {
            continue;
        }
        if (focusEntry.isReject) {
            AUDIO_INFO_LOG("ProcessFocusEntry: the incoming stream is rejected by sessionId:%{public}d, pid:%{public}d",
                (iterActive->first).sessionID, (iterActive->first).pid);
            incomingState = STOP;
            break;
        }
        AudioFocuState newState = ACTIVE;
        auto pos = HINTSTATEMAP.find(focusEntry.hintType);
        newState = (pos == HINTSTATEMAP.end()) ? ACTIVE : pos->second;
        incomingState = (newState > incomingState) ? newState : incomingState;
    }
    if (incomingState == STOP) {
        interruptEvent.hintType = INTERRUPT_HINT_STOP;
    } else if (incomingState == PAUSE) {
        interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    } else if (incomingState == DUCK) {
        interruptEvent.hintType = INTERRUPT_HINT_DUCK;
        interruptEvent.duckVolume = DUCK_FACTOR * GetSystemVolumeDb(incomingInterrupt.audioFocusType.streamType);
    } else {
        ProcessCurrentInterrupt(incomingInterrupt);
    }
    if (incomingState != STOP) {
        audioFocusInfoList_.emplace_back(std::make_pair(incomingInterrupt, incomingState));
        OnAudioFocusInfoChange();
    }
    if (policyListenerCb != nullptr && interruptEvent.hintType != INTERRUPT_HINT_NONE) {
        AUDIO_INFO_LOG("OnInterrupt for incoming sessionID: %{public}d, hintType: %{public}d",
            incomingInterrupt.sessionID, interruptEvent.hintType);
        policyListenerCb->OnInterrupt(interruptEvent);
    }
    return incomingState >= PAUSE ? ERR_FOCUS_DENIED : SUCCESS;
}

AudioScene AudioPolicyServer::GetHighestPriorityAudioSceneFromAudioFocusInfoList() const
{
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    int audioScenePriority = GetAudioScenePriority(audioScene);
    for (const auto&[interrupt, focuState] : audioFocusInfoList_) {
        AudioScene itAudioScene = GetAudioSceneFromAudioInterrupt(interrupt);
        int itAudioScenePriority = GetAudioScenePriority(itAudioScene);
        if (itAudioScenePriority > audioScenePriority) {
            audioScene = itAudioScene;
            audioScenePriority = itAudioScenePriority;
        }
    }
    return audioScene;
}

int32_t AudioPolicyServer::ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(interruptMutex_);

    AudioStreamType streamType = audioInterrupt.audioFocusType.streamType;
    AUDIO_INFO_LOG("ActivateAudioInterrupt::sessionID: %{public}u, streamType: %{public}d, streamUsage: %{public}d, "\
        "sourceType: %{public}d, pid: %{public}d", audioInterrupt.sessionID, streamType, audioInterrupt.streamUsage,
        (audioInterrupt.audioFocusType).sourceType, audioInterrupt.pid);

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_INFO_LOG("ActivateAudioInterrupt::parallelPlayFlag is true.");
        return SUCCESS;
    }

    if ((streamType == AudioStreamType::STREAM_MUSIC) || (streamType == AudioStreamType::STREAM_SPEECH)) {
        SetOffloadStream(audioInterrupt.sessionID);
    } else {
        AUDIO_DEBUG_LOG("session:%{public}d not get offload stream type is %{public}d", audioInterrupt.sessionID,
            streamType);
    }
    
    if (!audioPolicyService_.IsAudioInterruptEnabled()) {
        AUDIO_WARNING_LOG("AudioInterrupt is not enabled. No need to ActivateAudioInterrupt");
        audioFocusInfoList_.emplace_back(std::make_pair(audioInterrupt, ACTIVE));
        AudioScene targetAudioScene = GetHighestPriorityAudioSceneFromAudioFocusInfoList();
        UpdateAudioScene(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT);
        return SUCCESS;
    }

    uint32_t incomingSessionID = audioInterrupt.sessionID;
    if (!audioFocusInfoList_.empty()) {
        // If the session is present in audioFocusInfoList_, remove and treat it as a new request
        AUDIO_DEBUG_LOG("audioFocusInfoList_ is not empty, check whether the session is present");
        audioFocusInfoList_.remove_if([&incomingSessionID](std::pair<AudioInterrupt, AudioFocuState> &audioFocus) {
            return (audioFocus.first).sessionID == incomingSessionID;
        });
    }
    if (audioFocusInfoList_.empty()) {
        // If audioFocusInfoList_ is empty, directly activate interrupt
        AUDIO_INFO_LOG("audioFocusInfoList_ is empty, add the session into it directly");
        audioFocusInfoList_.emplace_back(std::make_pair(audioInterrupt, ACTIVE));
        OnAudioFocusInfoChange();
        AudioScene targetAudioScene = GetHighestPriorityAudioSceneFromAudioFocusInfoList();
        UpdateAudioScene(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT);
        return SUCCESS;
    }

    // Process ProcessFocusEntryTable for current audioFocusInfoList_
    int32_t ret = ProcessFocusEntry(audioInterrupt);
    if (ret) {
        AUDIO_ERR_LOG("ActivateAudioInterrupt request rejected");
        return ERR_FOCUS_DENIED;
    }
    AudioScene targetAudioScene = GetHighestPriorityAudioSceneFromAudioFocusInfoList();
    UpdateAudioScene(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT);
    return SUCCESS;
}

void AudioPolicyServer::UpdateAudioScene(const AudioScene audioScene, AudioInterruptChangeType changeType)
{
    AudioScene currentAudioScene = GetAudioScene();
    AUDIO_INFO_LOG("UpdateAudioScene: currentAudioScene=%{public}d, audioScene=%{public}d, changeType=%{public}d",
        currentAudioScene, audioScene, changeType);

    switch (changeType) {
        case ACTIVATE_AUDIO_INTERRUPT:
            break;
        case DEACTIVATE_AUDIO_INTERRUPT:
            if (GetAudioScenePriority(audioScene) >= GetAudioScenePriority(currentAudioScene)) {
                return;
            }
            break;
        default:
            AUDIO_ERR_LOG("Unexpected changeType=%{public}d", changeType);
            return;
    }

    audioPolicyService_.SetAudioScene(audioScene);
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioPolicyServer::SimulateFocusEntry()
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList;
    auto focusMap = audioPolicyService_.GetAudioFocusMap();
    for (auto iterActive = audioFocusInfoList_.begin(); iterActive != audioFocusInfoList_.end(); ++iterActive) {
        AudioInterrupt incoming = iterActive->first;
        AudioFocuState incomingState = ACTIVE;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpAudioFocuInfoList = newAudioFocuInfoList;
        for (auto iter = newAudioFocuInfoList.begin(); iter != newAudioFocuInfoList.end(); ++iter) {
            AudioInterrupt inprocessing = iter->first;
            if (iter->second == PAUSE || IsSameAppInShareMode(incoming, inprocessing)) {
                continue;
            }
            AudioFocusType activeFocusType = inprocessing.audioFocusType;
            AudioFocusType incomingFocusType = incoming.audioFocusType;
            std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
                std::make_pair(activeFocusType, incomingFocusType);
            if (focusMap.find(audioFocusTypePair) == focusMap.end()) {
                AUDIO_WARNING_LOG("AudioPolicyServer: SimulateFocusEntry Audio Focus type is invalid");
                incomingState = iterActive->second;
                break;
            }
            AudioFocusEntry focusEntry = focusMap[audioFocusTypePair];
            auto pos = HINTSTATEMAP.find(focusEntry.hintType);
            if (pos == HINTSTATEMAP.end()) {
                continue;
            }
            if (focusEntry.actionOn == CURRENT) {
                iter->second = pos->second;
            } else {
                AudioFocuState newState = pos->second;
                incomingState = newState > incomingState ? newState : incomingState;
            }
        }

        if (incomingState == PAUSE) {
            newAudioFocuInfoList = tmpAudioFocuInfoList;
        }
        newAudioFocuInfoList.emplace_back(std::make_pair(incoming, incomingState));
    }

    return newAudioFocuInfoList;
}

void AudioPolicyServer::NotifyStateChangedEvent(AudioFocuState oldState, AudioFocuState newState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive)
{
    AudioInterrupt audioInterrupt = iterActive->first;
    uint32_t sessionID = audioInterrupt.sessionID;
    std::shared_ptr<AudioInterruptCallback> policyListenerCb = interruptCbsMap_[sessionID];
    if (policyListenerCb == nullptr) {
        AUDIO_WARNING_LOG("AudioPolicyServer: sessionID policyListenerCb is null");
        return;
    }
    InterruptEventInternal forceActive {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_RESUME, 1.0f};
    InterruptEventInternal forceUnduck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNDUCK, 1.0f};
    InterruptEventInternal forceDuck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, 1.0f};
    InterruptEventInternal forcePause {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 1.0f};
    float volumeDb = GetSystemVolumeDb((audioInterrupt.audioFocusType).streamType);
    forceDuck.duckVolume = DUCK_FACTOR * volumeDb;
    switch (newState) {
        case ACTIVE:
            if (oldState == PAUSE) {
                policyListenerCb->OnInterrupt(forceActive);
            }
            if (oldState == DUCK) {
                policyListenerCb->OnInterrupt(forceUnduck);
            }
            break;
        case DUCK:
            if (oldState == PAUSE) {
                policyListenerCb->OnInterrupt(forceActive);
            }
            policyListenerCb->OnInterrupt(forceDuck);
            break;
        case PAUSE:
            if (oldState == DUCK) {
                policyListenerCb->OnInterrupt(forceUnduck);
            }
            policyListenerCb->OnInterrupt(forcePause);
            break;
        default:
            break;
    }
    iterActive->second = newState;
}

void AudioPolicyServer::ResumeAudioFocusList()
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList = SimulateFocusEntry();
    for (auto iterActive = audioFocusInfoList_.begin(), iterNew = newAudioFocuInfoList.begin(); iterActive !=
        audioFocusInfoList_.end() && iterNew != newAudioFocuInfoList.end(); ++iterActive, ++iterNew) {
        AudioFocuState oldState = iterActive->second;
        AudioFocuState newState = iterNew->second;
        if (oldState != newState) {
            AUDIO_INFO_LOG("ResumeAudioFocusList: State change: sessionID %{public}d, oldstate %{public}d, "\
                "newState %{public}d", (iterActive->first).sessionID, oldState, newState);
            NotifyStateChangedEvent(oldState, newState, iterActive);
        }
    }
}

int32_t AudioPolicyServer::DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(interruptMutex_);

    AudioScene highestPriorityAudioScene = AUDIO_SCENE_DEFAULT;

    ReleaseOffloadStream(audioInterrupt.sessionID);
    if (!audioPolicyService_.IsAudioInterruptEnabled()) {
        AUDIO_WARNING_LOG("AudioInterrupt is not enabled. No need to DeactivateAudioInterrupt");
        uint32_t exitSessionID = audioInterrupt.sessionID;
        audioFocusInfoList_.remove_if([&](std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo) {
            if ((audioFocusInfo.first).sessionID != exitSessionID) {
                AudioScene targetAudioScene = GetAudioSceneFromAudioInterrupt(audioFocusInfo.first);
                if (GetAudioScenePriority(targetAudioScene) > GetAudioScenePriority(highestPriorityAudioScene)) {
                    highestPriorityAudioScene = targetAudioScene;
                }
                return false;
            }
            OnAudioFocusInfoChange();
            return true;
        });
        UpdateAudioScene(highestPriorityAudioScene, DEACTIVATE_AUDIO_INTERRUPT);
        return SUCCESS;
    }

    AUDIO_INFO_LOG("DeactivateAudioInterrupt::sessionID: %{public}u, streamType: %{public}d, streamUsage: %{public}d, "\
        "sourceType: %{public}d, pid: %{public}d", audioInterrupt.sessionID, (audioInterrupt.audioFocusType).streamType,
        audioInterrupt.streamUsage, (audioInterrupt.audioFocusType).sourceType, audioInterrupt.pid);

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_INFO_LOG("DeactivateAudioInterrupt::parallelPlayFlag is true.");
        return SUCCESS;
    }

    bool isInterruptActive = false;

    for (auto it = audioFocusInfoList_.begin(); it != audioFocusInfoList_.end();) {
        if ((it->first).sessionID == audioInterrupt.sessionID) {
            it = audioFocusInfoList_.erase(it);
            isInterruptActive = true;
            OnAudioFocusInfoChange();
        } else {
            AudioScene targetAudioScene = GetAudioSceneFromAudioInterrupt(it->first);
            if (GetAudioScenePriority(targetAudioScene) > GetAudioScenePriority(highestPriorityAudioScene)) {
                highestPriorityAudioScene = targetAudioScene;
            }
            ++it;
        }
    }

    // If it was not in the audioFocusInfoList_, no need to take any action on other sessions, just return.
    if (!isInterruptActive) {
        AUDIO_DEBUG_LOG("DeactivateAudioInterrupt: the stream (sessionID %{public}d) is not active now, return success",
            audioInterrupt.sessionID);
        return SUCCESS;
    }

    UpdateAudioScene(highestPriorityAudioScene, DEACTIVATE_AUDIO_INTERRUPT);
    
    OffloadStopPlaying(audioInterrupt);

    // resume if other session was forced paused or ducked
    ResumeAudioFocusList();

    return SUCCESS;
}

void AudioPolicyServer::OnSessionRemoved(const uint64_t sessionID)
{
    sessionProcessor_.Post({SessionEvent::Type::REMOVE, sessionID});
}

void AudioPolicyServer::ProcessSessionRemoved(const uint64_t sessionID)
{
    audioPolicyService_.OnCapturerSessionRemoved(sessionID);
    uint32_t removedSessionID = sessionID;

    auto isSessionPresent = [&removedSessionID] (const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo) {
        return audioFocusInfo.first.sessionID == removedSessionID;
    };

    std::unique_lock<std::mutex> lock(interruptMutex_);

    auto iterActive = std::find_if(audioFocusInfoList_.begin(), audioFocusInfoList_.end(), isSessionPresent);
    if (iterActive != audioFocusInfoList_.end()) {
        AudioInterrupt removedInterrupt = (*iterActive).first;
        lock.unlock();
        AUDIO_INFO_LOG("Removed SessionID: %{public}u is present in audioFocusInfoList_", removedSessionID);

        (void)DeactivateAudioInterrupt(removedInterrupt);
        (void)UnsetAudioInterruptCallback(removedSessionID);
        return;
    }

    // Though it is not present in the owners list, check and clear its entry from callback map
    lock.unlock();
    (void)UnsetAudioInterruptCallback(removedSessionID);
}

void AudioPolicyServer::OnCapturerSessionAdded(const uint64_t sessionID, SessionInfo sessionInfo)
{
    sessionProcessor_.Post({SessionEvent::Type::ADD, sessionID, sessionInfo});
}

void AudioPolicyServer::ProcessSessionAdded(SessionEvent sessionEvent)
{
    audioPolicyService_.OnCapturerSessionAdded(sessionEvent.sessionID, sessionEvent.sessionInfo_);
}

void AudioPolicyServer::ProcessorCloseWakeupSource(const uint64_t sessionID)
{
    audioPolicyService_.CloseWakeUpAudioCapturer();
}

void AudioPolicyServer::OnPlaybackCapturerStop()
{
    audioPolicyService_.UnloadLoopback();
}

void AudioPolicyServer::OnWakeupCapturerStop(uint32_t sessionID)
{
    sessionProcessor_.Post({SessionEvent::Type::CLOSE_WAKEUP_SOURCE, sessionID});
}

void AudioPolicyServer::OnDstatusUpdated(bool isConnected)
{
    static std::mutex mtx;
    static int count = 0;
    std::lock_guard<std::mutex> {mtx};
    if (isConnected) {
        if (count == 0) {
            sessionProcessor_.Post({SessionEvent::Type::ADD, DSTATUS_SESSION_ID,
                {SOURCE_TYPE_MIC, DSTATUS_DEFAULT_RATE}});
        }
        count++;
    } else {
        count--;
        if (count == 0) {
            sessionProcessor_.Post({SessionEvent::Type::REMOVE, DSTATUS_SESSION_ID});
        }
    }
}

AudioStreamType AudioPolicyServer::GetStreamInFocus()
{
    AudioStreamType streamInFocus = STREAM_DEFAULT;
    for (auto iter = audioFocusInfoList_.begin(); iter != audioFocusInfoList_.end(); ++iter) {
        if (iter->second != ACTIVE || (iter->first).audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
            // if the steam is not active or the active stream is an audio capturer stream, skip it.
            continue;
        }
        AudioInterrupt audioInterrupt = iter->first;
        streamInFocus = audioInterrupt.audioFocusType.streamType;
        if (streamInFocus != STREAM_ULTRASONIC) {
            break;
        }
    }

    return streamInFocus;
}

int32_t AudioPolicyServer::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt)
{
    uint32_t invalidSessionID = static_cast<uint32_t>(-1);
    audioInterrupt = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, invalidSessionID};

    for (auto iter = audioFocusInfoList_.begin(); iter != audioFocusInfoList_.end(); ++iter) {
        if (iter->second == ACTIVE) {
            audioInterrupt = iter->first;
        }
    }

    return SUCCESS;
}

void AudioPolicyServer::OnAudioFocusInfoChange()
{
    std::lock_guard<std::mutex> lock(focusInfoChangeMutex_);
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
        sptr<IAudioPolicyClient> foucusInfoCb = it->second;
        if (foucusInfoCb == nullptr) {
            AUDIO_ERR_LOG("foucusInfoCb: nullptr for client : %{public}d", it->first);
            continue;
        }
        foucusInfoCb->OnAudioFocusInfoChange(audioFocusInfoList_);
    }
}

int32_t AudioPolicyServer::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);

    focusInfoList = audioFocusInfoList_;
    return SUCCESS;
}

bool AudioPolicyServer::CheckRootCalling(uid_t callingUid, int32_t appUid)
{
    if (callingUid == UID_ROOT) {
        return true;
    }

    // check original caller if it pass
    if (std::count(RECORD_PASS_APPINFO_LIST.begin(), RECORD_PASS_APPINFO_LIST.end(), callingUid) > 0) {
        if (appUid == UID_ROOT) {
            return true;
        }
    }

    return false;
}

bool AudioPolicyServer::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    uid_t callingUid = IPCSkeleton::GetCallingUid();
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t callingFullTokenId = IPCSkeleton::GetCallingFullTokenID();

    if (CheckRootCalling(callingUid, appUid)) {
        AUDIO_INFO_LOG("root user recording");
        return true;
    }

    Security::AccessToken::AccessTokenID targetTokenId = GetTargetTokenId(callingUid, callingTokenId, appTokenId);
    if (!VerifyPermission(MICROPHONE_PERMISSION, targetTokenId, true)) {
        return false;
    }

    uint64_t targetFullTokenId = GetTargetFullTokenId(callingUid, callingFullTokenId, appFullTokenId);
    if (!CheckAppBackgroundPermission(callingUid, targetFullTokenId, targetTokenId)) {
        return false;
    }

    if (sourceType == SOURCE_TYPE_VOICE_CALL) {
        if (VerifyVoiceCallPermission() != SUCCESS) {
            return false;
        }
    }

    return true;
}

bool AudioPolicyServer::VerifyPermission(const std::string &permissionName, uint32_t tokenId, bool isRecording)
{
    AUDIO_DEBUG_LOG("Verify permission [%{public}s]", permissionName.c_str());

    if (!isRecording) {
        // root user case for auto test
        uid_t callingUid = IPCSkeleton::GetCallingUid();
        if (callingUid == UID_ROOT) {
            return true;
        }

        tokenId = IPCSkeleton::GetCallingTokenID();
    }

    int res = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    if (res != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        AUDIO_ERR_LOG("Permission denied [%{public}s]", permissionName.c_str());
        return false;
    }

    return true;
}

bool AudioPolicyServer::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    uid_t callingUid = IPCSkeleton::GetCallingUid();
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint64_t callingFullTokenId = IPCSkeleton::GetCallingFullTokenID();
    Security::AccessToken::AccessTokenID targetTokenId = GetTargetTokenId(callingUid, callingTokenId, appTokenId);
    uint64_t targetFullTokenId = GetTargetFullTokenId(callingUid, callingFullTokenId, appFullTokenId);

    // start recording need to check app state
    if (state == AUDIO_PERMISSION_START && !CheckRootCalling(callingUid, appUid)) {
        if (!CheckAppBackgroundPermission(callingUid, targetFullTokenId, targetTokenId)) {
            return false;
        }
    }

    NotifyPrivacy(targetTokenId, state);
    return true;
}

void AudioPolicyServer::NotifyPrivacy(uint32_t targetTokenId, AudioPermissionState state)
{
    if (state == AUDIO_PERMISSION_START) {
        int res = PrivacyKit::StartUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        if (res != 0) {
            AUDIO_WARNING_LOG("notice start using perm error");
        }
    } else {
        int res = PrivacyKit::StopUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
        if (res != 0) {
            AUDIO_WARNING_LOG("notice stop using perm error");
        }
    }
}

bool AudioPolicyServer::CheckAppBackgroundPermission(uid_t callingUid, uint64_t targetFullTokenId,
    uint32_t targetTokenId)
{
    if (TokenIdKit::IsSystemAppByFullTokenID(targetFullTokenId)) {
        AUDIO_INFO_LOG("system app recording");
        return true;
    }
    if (std::count(RECORD_ALLOW_BACKGROUND_LIST.begin(), RECORD_ALLOW_BACKGROUND_LIST.end(), callingUid) > 0) {
        AUDIO_INFO_LOG("internal sa user directly recording");
        return true;
    }
    return PrivacyKit::IsAllowedUsingPermission(targetTokenId, MICROPHONE_PERMISSION);
}

Security::AccessToken::AccessTokenID AudioPolicyServer::GetTargetTokenId(uid_t callingUid, uint32_t callingTokenId,
    uint32_t appTokenId)
{
    return (std::count(RECORD_PASS_APPINFO_LIST.begin(), RECORD_PASS_APPINFO_LIST.end(), callingUid) > 0) ?
        appTokenId : callingTokenId;
}

uint64_t AudioPolicyServer::GetTargetFullTokenId(uid_t callingUid, uint64_t callingFullTokenId,
    uint64_t appFullTokenId)
{
    return (std::count(RECORD_PASS_APPINFO_LIST.begin(), RECORD_PASS_APPINFO_LIST.end(), callingUid) > 0) ?
        appFullTokenId : callingFullTokenId;
}

int32_t AudioPolicyServer::ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType)
{
    // Only root users should have access to this api
    if (ROOT_UID != IPCSkeleton::GetCallingUid()) {
        AUDIO_INFO_LOG("Unautorized user. Cannot modify channel");
        return ERR_PERMISSION_DENIED;
    }

    return audioPolicyService_.ReconfigureAudioChannel(count, deviceType);
}

void AudioPolicyServer::GetPolicyData(PolicyData &policyData)
{
    policyData.ringerMode = GetRingerMode();
    policyData.callStatus = GetAudioScene();

    // Get stream volumes
    for (int stream = AudioStreamType::STREAM_VOICE_CALL; stream <= AudioStreamType::STREAM_TYPE_MAX; stream++) {
        AudioStreamType streamType = (AudioStreamType)stream;

        if (AudioServiceDump::IsStreamSupported(streamType)) {
            int32_t volume = GetSystemVolumeLevel(streamType);
            policyData.streamVolumes.insert({ streamType, volume });
        }
    }

    // Get Audio Focus Information
    policyData.audioFocusInfoList = audioFocusInfoList_;
    GetDeviceInfo(policyData);
    GetGroupInfo(policyData);
    GetStreamVolumeInfoMap(policyData.streamVolumeInfos);
    policyData.availableMicrophones = GetAvailableMicrophones();
    // Get Audio Effect Manager Information
    audioPolicyService_.GetEffectManagerInfo(policyData.oriEffectConfig, policyData.availableEffects);
}

void AudioPolicyServer::GetStreamVolumeInfoMap(StreamVolumeInfoMap& streamVolumeInfos)
{
    audioPolicyService_.GetStreamVolumeInfoMap(streamVolumeInfos);
}

void AudioPolicyServer::GetDeviceInfo(PolicyData& policyData)
{
    DeviceFlag deviceFlag = DeviceFlag::INPUT_DEVICES_FLAG;
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors = GetDevices(deviceFlag);

    for (auto it = audioDeviceDescriptors.begin(); it != audioDeviceDescriptors.end(); it++) {
        AudioDeviceDescriptor audioDeviceDescriptor = **it;
        DevicesInfo deviceInfo;
        deviceInfo.deviceType = audioDeviceDescriptor.deviceType_;
        deviceInfo.deviceRole = audioDeviceDescriptor.deviceRole_;
        deviceInfo.conneceType  = CONNECT_TYPE_LOCAL;
        policyData.inputDevices.push_back(deviceInfo);
    }

    deviceFlag = DeviceFlag::OUTPUT_DEVICES_FLAG;
    audioDeviceDescriptors = GetDevices(deviceFlag);

    for (auto it = audioDeviceDescriptors.begin(); it != audioDeviceDescriptors.end(); it++) {
        AudioDeviceDescriptor audioDeviceDescriptor = **it;
        DevicesInfo deviceInfo;
        deviceInfo.deviceType = audioDeviceDescriptor.deviceType_;
        deviceInfo.deviceRole = audioDeviceDescriptor.deviceRole_;
        deviceInfo.conneceType  = CONNECT_TYPE_LOCAL;
        policyData.outputDevices.push_back(deviceInfo);
    }

    deviceFlag = DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG;
    audioDeviceDescriptors = GetDevices(deviceFlag);

    for (auto it = audioDeviceDescriptors.begin(); it != audioDeviceDescriptors.end(); it++) {
        AudioDeviceDescriptor audioDeviceDescriptor = **it;
        DevicesInfo deviceInfo;
        deviceInfo.deviceType = audioDeviceDescriptor.deviceType_;
        deviceInfo.deviceRole = audioDeviceDescriptor.deviceRole_;
        deviceInfo.conneceType  = CONNECT_TYPE_DISTRIBUTED;
        policyData.inputDevices.push_back(deviceInfo);
    }

    deviceFlag = DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG;
    audioDeviceDescriptors = GetDevices(deviceFlag);

    for (auto it = audioDeviceDescriptors.begin(); it != audioDeviceDescriptors.end(); it++) {
        AudioDeviceDescriptor audioDeviceDescriptor = **it;
        DevicesInfo deviceInfo;
        deviceInfo.deviceType = audioDeviceDescriptor.deviceType_;
        deviceInfo.deviceRole = audioDeviceDescriptor.deviceRole_;
        deviceInfo.conneceType  = CONNECT_TYPE_DISTRIBUTED;
        policyData.outputDevices.push_back(deviceInfo);
    }

    policyData.priorityOutputDevice = GetActiveOutputDevice();
    policyData.priorityInputDevice = GetActiveInputDevice();
}

void AudioPolicyServer::GetGroupInfo(PolicyData& policyData)
{
    // Get group info
    std::vector<sptr<VolumeGroupInfo>> groupInfos = audioPolicyService_.GetVolumeGroupInfos();

    for (auto volumeGroupInfo : groupInfos) {
        GroupInfo info;
        info.groupId = volumeGroupInfo->volumeGroupId_;
        info.groupName = volumeGroupInfo->groupName_;
        info.type = volumeGroupInfo->connectType_;
        policyData.groupInfos.push_back(info);
    }
}

void AudioPolicyServer::ProcessInterrupt(const InterruptHint& hint)
{
    InterruptType type = INTERRUPT_TYPE_BEGIN;
    InterruptForceType forceType = INTERRUPT_SHARE;
    InterruptEventInternal interruptEvent {type, forceType, hint, 0.2f};
    for (auto it : interruptCbsMap_) {
        if (it.second != nullptr) {
            it.second->OnInterrupt(interruptEvent);
        }
    }
}

int32_t AudioPolicyServer::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    AUDIO_DEBUG_LOG("AudioPolicyServer: Dump Process Invoked");
    std::queue<std::u16string> argQue;
    for (decltype(args.size()) index = 0; index < args.size(); ++index) {
        if (args[index] == u"debug_interrupt_resume") {
            ProcessInterrupt(INTERRUPT_HINT_RESUME);
        }
        if (args[index] == u"debug_interrupt_pause") {
            ProcessInterrupt(INTERRUPT_HINT_PAUSE);
        }
        argQue.push(args[index]);
    }
    std::string dumpString;
    PolicyData policyData;
    AudioServiceDump dumpObj;

    if (dumpObj.Initialize() != AUDIO_DUMP_SUCCESS) {
        AUDIO_ERR_LOG("Audio Service Dump Not initialised\n");
        return AUDIO_DUMP_INIT_ERR;
    }

    GetPolicyData(policyData);
    dumpObj.AudioDataDump(policyData, dumpString, argQue);

    return write(fd, dumpString.c_str(), dumpString.size());
}

int32_t AudioPolicyServer::GetAudioLatencyFromXml()
{
    return audioPolicyService_.GetAudioLatencyFromXml();
}

uint32_t AudioPolicyServer::GetSinkLatencyFromXml()
{
    return audioPolicyService_.GetSinkLatencyFromXml();
}

int32_t AudioPolicyServer::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object)
{
    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    AUDIO_INFO_LOG("RegisterTracker: [caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        if (mode == AUDIO_MODE_PLAYBACK) {
            streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioRendererChangeInfo.clientUID);
        } else {
            streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioCapturerChangeInfo.clientUID);
        }
    }
    RegisterClientDeathRecipient(object, TRACKER_CLIENT);
    return audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object);
}

int32_t AudioPolicyServer::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    // update the clientUid
    auto callerUid = IPCSkeleton::GetCallingUid();
    streamChangeInfo.audioRendererChangeInfo.createrUID = callerUid;
    streamChangeInfo.audioCapturerChangeInfo.createrUID = callerUid;
    AUDIO_INFO_LOG("UpdateTracker: [caller uid: %{public}d]", callerUid);
    if (callerUid != MEDIA_SERVICE_UID) {
        if (mode == AUDIO_MODE_PLAYBACK) {
            streamChangeInfo.audioRendererChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioRendererChangeInfo.clientUID);
        } else {
            streamChangeInfo.audioCapturerChangeInfo.clientUID = callerUid;
            AUDIO_DEBUG_LOG("Non media service caller, use the uid retrieved. ClientUID:%{public}d]",
                streamChangeInfo.audioCapturerChangeInfo.clientUID);
        }
    }
    return audioPolicyService_.UpdateTracker(mode, streamChangeInfo);
}

int32_t AudioPolicyServer::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return audioPolicyService_.GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
}

int32_t AudioPolicyServer::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: BT use permission: %{public}d", hasBTPermission);
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos: System use permission: %{public}d", hasSystemPermission);

    return audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
}

void AudioPolicyServer::RegisterClientDeathRecipient(const sptr<IRemoteObject> &object, DeathRecipientId id)
{
    AUDIO_INFO_LOG("Register clients death recipient!!");
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    CHECK_AND_RETURN_LOG(object != nullptr, "Client proxy obj NULL!!");

    pid_t uid = 0;
    if (id == TRACKER_CLIENT) {
        // Deliberately casting UID to pid_t
        uid = static_cast<pid_t>(IPCSkeleton::GetCallingUid());
    } else {
        uid = IPCSkeleton::GetCallingPid();
    }
    if (id == TRACKER_CLIENT && std::find(clientDiedListenerState_.begin(), clientDiedListenerState_.end(), uid)
        != clientDiedListenerState_.end()) {
        AUDIO_INFO_LOG("Tracker has been registered for %{public}d!", uid);
        return;
    }
    sptr<AudioServerDeathRecipient> deathRecipient_ = new(std::nothrow) AudioServerDeathRecipient(uid);
    if (deathRecipient_ != nullptr) {
        if (id == TRACKER_CLIENT) {
            deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyServer::RegisteredTrackerClientDied,
                this, std::placeholders::_1));
        } else {
            AUDIO_INFO_LOG("RegisteredStreamListenerClientDied register!!");
            deathRecipient_->SetNotifyCb(std::bind(&AudioPolicyServer::RegisteredStreamListenerClientDied,
                this, std::placeholders::_1));
        }
        bool result = object->AddDeathRecipient(deathRecipient_);
        if (result && id == TRACKER_CLIENT) {
            clientDiedListenerState_.push_back(uid);
        }
        if (!result) {
            AUDIO_ERR_LOG("failed to add deathRecipient");
        }
    }
}

void AudioPolicyServer::RegisteredTrackerClientDied(pid_t uid)
{
    AUDIO_INFO_LOG("RegisteredTrackerClient died: remove entry, uid %{public}d", uid);
    std::lock_guard<std::mutex> lock(clientDiedListenerStateMutex_);
    audioPolicyService_.RegisteredTrackerClientDied(uid);
    auto filter = [&uid](int val) {
        return uid == val;
    };
    clientDiedListenerState_.erase(std::remove_if(clientDiedListenerState_.begin(), clientDiedListenerState_.end(),
        filter), clientDiedListenerState_.end());
}

void AudioPolicyServer::RegisteredStreamListenerClientDied(pid_t pid)
{
    AUDIO_INFO_LOG("RegisteredStreamListenerClient died: remove entry, uid %{public}d", pid);
    if (audioPolicyClientProxyCBMap_.erase(pid) == 0) {
        AUDIO_ERR_LOG("RegisteredStreamListenerClientDied client %{public}d not exist.", pid);
    }
    audioPolicyService_.SetAudioPolicyClientProxy(audioPolicyClientProxyCBMap_);
}

int32_t AudioPolicyServer::UpdateStreamState(const int32_t clientUid,
    StreamSetState streamSetState, AudioStreamType audioStreamType)
{
    constexpr int32_t avSessionUid = 6700; // "uid" : "av_session"
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != avSessionUid) {
        // This function can only be used by av_session
        AUDIO_ERR_LOG("UpdateStreamState callerUid is error: not av_session");
        return ERROR;
    }

    AUDIO_INFO_LOG("UpdateStreamState::uid:%{public}d streamSetState:%{public}d audioStreamType:%{public}d",
        clientUid, streamSetState, audioStreamType);
    StreamSetState setState = StreamSetState::STREAM_PAUSE;
    if (streamSetState == StreamSetState::STREAM_RESUME) {
        setState  = StreamSetState::STREAM_RESUME;
    } else if (streamSetState != StreamSetState::STREAM_PAUSE) {
        AUDIO_ERR_LOG("UpdateStreamState streamSetState value is error");
        return ERROR;
    }
    StreamSetStateEventInternal setStateEvent = {};
    setStateEvent.streamSetState = setState;
    setStateEvent.audioStreamType = audioStreamType;

    return audioPolicyService_.UpdateStreamState(clientUid, setStateEvent);
}

int32_t AudioPolicyServer::GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return ERR_PERMISSION_DENIED;
    }

    infos = audioPolicyService_.GetVolumeGroupInfos();
    auto filter = [&networkId](const sptr<VolumeGroupInfo>& info) {
        return networkId != info->networkId_;
    };
    infos.erase(std::remove_if(infos.begin(), infos.end(), filter), infos.end());

    return SUCCESS;
}

int32_t AudioPolicyServer::GetNetworkIdByGroupId(int32_t groupId, std::string &networkId)
{
    auto volumeGroupInfos = audioPolicyService_.GetVolumeGroupInfos();

    auto filter = [&groupId](const sptr<VolumeGroupInfo>& info) {
        return groupId != info->volumeGroupId_;
    };
    volumeGroupInfos.erase(std::remove_if(volumeGroupInfos.begin(), volumeGroupInfos.end(), filter),
        volumeGroupInfos.end());
    if (volumeGroupInfos.size() > 0) {
        networkId = volumeGroupInfos[0]->networkId_;
        AUDIO_INFO_LOG("GetNetworkIdByGroupId: get networkId %{public}s.", networkId.c_str());
    } else {
        AUDIO_ERR_LOG("GetNetworkIdByGroupId: has no valid group");
        return ERROR;
    }

    return SUCCESS;
}

AudioPolicyServer::RemoteParameterCallback::RemoteParameterCallback(sptr<AudioPolicyServer> server)
{
    server_ = server;
}

void AudioPolicyServer::RemoteParameterCallback::OnAudioParameterChange(const std::string networkId,
    const AudioParamKey key, const std::string& condition, const std::string& value)
{
    AUDIO_INFO_LOG("AudioPolicyServer::OnAudioParameterChange key:%{public}d, condition:%{public}s, value:%{public}s",
        key, condition.c_str(), value.c_str());
    if (server_ == nullptr) {
        AUDIO_ERR_LOG("server_ is nullptr");
        return;
    }
    switch (key) {
        case VOLUME:
            VolumeOnChange(networkId, condition);
            break;
        case INTERRUPT:
            InterruptOnChange(networkId, condition);
            break;
        case PARAM_KEY_STATE:
            StateOnChange(networkId, condition, value);
            break;
        default:
            AUDIO_DEBUG_LOG("[AudioPolicyServer]: No processing");
            break;
    }
}

void AudioPolicyServer::RemoteParameterCallback::VolumeOnChange(const std::string networkId,
    const std::string& condition)
{
    VolumeEvent volumeEvent;
    volumeEvent.networkId = networkId;
    char eventDes[EVENT_DES_SIZE];
    if (sscanf_s(condition.c_str(), "%[^;];AUDIO_STREAM_TYPE=%d;VOLUME_LEVEL=%d;IS_UPDATEUI=%d;VOLUME_GROUP_ID=%d;",
        eventDes, EVENT_DES_SIZE, &(volumeEvent.volumeType), &(volumeEvent.volume), &(volumeEvent.updateUi),
        &(volumeEvent.volumeGroupId)) < PARAMS_VOLUME_NUM) {
        AUDIO_ERR_LOG("[VolumeOnChange]: Failed parse condition");
        return;
    }

    volumeEvent.updateUi = false;
    for (auto it = server_->audioPolicyClientProxyCBMap_.begin(); it != server_->audioPolicyClientProxyCBMap_.end(); ++it) {
        sptr<IAudioPolicyClient> volumeChangeCb = it->second;
        if (volumeChangeCb == nullptr) {
            AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
            continue;
        }

        AUDIO_INFO_LOG("--------trigger volumeChangeCb clientPid : %{public}d", it->first);
        volumeChangeCb->OnVolumeKeyEvent(volumeEvent);
    }
}

void AudioPolicyServer::RemoteParameterCallback::InterruptOnChange(const std::string networkId,
    const std::string& condition)
{
    char eventDes[EVENT_DES_SIZE];
    InterruptType type = INTERRUPT_TYPE_BEGIN;
    InterruptForceType forceType = INTERRUPT_SHARE;
    InterruptHint hint = INTERRUPT_HINT_NONE;

    if (sscanf_s(condition.c_str(), "%[^;];EVENT_TYPE=%d;FORCE_TYPE=%d;HINT_TYPE=%d;", eventDes,
        EVENT_DES_SIZE, &type, &forceType, &hint) < PARAMS_INTERRUPT_NUM) {
        AUDIO_ERR_LOG("[InterruptOnChange]: Failed parse condition");
        return;
    }

    InterruptEventInternal interruptEvent {type, forceType, hint, 0.2f};
    for (auto it : server_->interruptCbsMap_) {
        if (it.second != nullptr) {
            it.second->OnInterrupt(interruptEvent);
        }
    }
}

void AudioPolicyServer::RemoteParameterCallback::StateOnChange(const std::string networkId,
    const std::string& condition, const std::string& value)
{
    char eventDes[EVENT_DES_SIZE];
    char contentDes[ADAPTER_STATE_CONTENT_DES_SIZE];
    if (sscanf_s(condition.c_str(), "%[^;];%s", eventDes, EVENT_DES_SIZE, contentDes,
        ADAPTER_STATE_CONTENT_DES_SIZE) < PARAMS_RENDER_STATE_NUM) {
        AUDIO_ERR_LOG("StateOnChange: Failed parse condition");
        return;
    }
    CHECK_AND_RETURN_LOG(strcmp(eventDes, "ERR_EVENT") == 0,
        "StateOnChange: Event %{public}s is not supported.", eventDes);

    std::string devTypeKey = "DEVICE_TYPE=";
    std::string contentDesStr = std::string(contentDes);
    auto devTypeKeyPos =  contentDesStr.find(devTypeKey);
    CHECK_AND_RETURN_LOG(devTypeKeyPos != std::string::npos,
        "StateOnChange: Not find daudio device type info, contentDes %{public}s.", contentDesStr.c_str());
    size_t devTypeValPos = devTypeKeyPos + devTypeKey.length();
    CHECK_AND_RETURN_LOG(devTypeValPos < contentDesStr.length(),
        "StateOnChange: Not find daudio device type value, contentDes %{public}s.", contentDesStr.c_str());

    if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_SPK) {
        server_->audioPolicyService_.NotifyRemoteRenderState(networkId, contentDesStr, value);
    } else if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_MIC) {
        AUDIO_INFO_LOG("StateOnChange: ERR_EVENT of DAUDIO_DEV_TYPE_MIC.");
    } else {
        AUDIO_ERR_LOG("StateOnChange: Device type is not supported, contentDes %{public}s.", contentDesStr.c_str());
    }
}

void AudioPolicyServer::PerStateChangeCbCustomizeCallback::PermStateChangeCallback(
    Security::AccessToken::PermStateChangeInfo& result)
{
    ready_ = true;
    Security::AccessToken::HapTokenInfo hapTokenInfo;
    int32_t res = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(result.tokenID, hapTokenInfo);
    if (res < 0) {
        AUDIO_ERR_LOG("Call GetHapTokenInfo fail.");
    }
    bool bSetMute;
    if (result.permStateChangeType > 0) {
        bSetMute = false;
    } else {
        bSetMute = true;
    }

    int32_t appUid = getUidByBundleName(hapTokenInfo.bundleName, hapTokenInfo.userID);
    if (appUid < 0) {
        AUDIO_ERR_LOG("fail to get uid.");
    } else {
        server_->audioPolicyService_.SetSourceOutputStreamMute(appUid, bSetMute);
        AUDIO_DEBUG_LOG("get uid value:%{public}d", appUid);
    }
}

int32_t AudioPolicyServer::PerStateChangeCbCustomizeCallback::getUidByBundleName(std::string bundle_name, int user_id)
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        return ERR_INVALID_PARAM;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        return ERR_INVALID_PARAM;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        return ERR_INVALID_PARAM;
    }
    int32_t iUid = bundleMgrProxy->GetUidByBundleName(bundle_name, user_id);

    return iUid;
}

void AudioPolicyServer::RegisterParamCallback()
{
    AUDIO_INFO_LOG("RegisterParamCallback");
    remoteParameterCallback_ = std::make_shared<RemoteParameterCallback>(this);
    audioPolicyService_.SetParameterCallback(remoteParameterCallback_);
    // regiest policy provider in audio server
    audioPolicyService_.RegiestPolicy();
}

void AudioPolicyServer::RegisterBluetoothListener()
{
    AUDIO_INFO_LOG("RegisterBluetoothListener");
    audioPolicyService_.RegisterBluetoothListener();
}

void AudioPolicyServer::SubscribeAccessibilityConfigObserver()
{
    AUDIO_INFO_LOG("SubscribeAccessibilityConfigObserver");
    audioPolicyService_.SubscribeAccessibilityConfigObserver();
}

bool AudioPolicyServer::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_INFO_LOG("IsAudioRendererLowLatencySupported server call");
    return true;
}

int32_t AudioPolicyServer::SetSystemSoundUri(const std::string &key, const std::string &uri)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    AUDIO_INFO_LOG("SetSystemSoundUri:: key: %{public}s, uri: %{public}s", key.c_str(), uri.c_str());
    return audioPolicyService_.SetSystemSoundUri(key, uri);
}

std::string AudioPolicyServer::GetSystemSoundUri(const std::string &key)
{
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("GetVolumeGroupInfos: No system permission");
        return "";
    }
    AUDIO_INFO_LOG("GetSystemSoundUri:: key: %{public}s", key.c_str());
    return audioPolicyService_.GetSystemSoundUri(key);
}

float AudioPolicyServer::GetMinStreamVolume()
{
    return audioPolicyService_.GetMinStreamVolume();
}

float AudioPolicyServer::GetMaxStreamVolume()
{
    return audioPolicyService_.GetMaxStreamVolume();
}

int32_t AudioPolicyServer::GetMaxRendererInstances()
{
    AUDIO_INFO_LOG("GetMaxRendererInstances");
    return audioPolicyService_.GetMaxRendererInstances();
}

void AudioPolicyServer::RegisterDataObserver()
{
    audioPolicyService_.RegisterDataObserver();
}

int32_t AudioPolicyServer::QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    int32_t ret = audioPolicyService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    return ret;
}

int32_t AudioPolicyServer::SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config,
    uint32_t appTokenId)
{
    for (auto &usg : config.filterOptions.usages) {
        if (usg != STREAM_USAGE_VOICE_COMMUNICATION) {
            continue;
        }

        if (!VerifyPermission(CAPTURER_VOICE_DOWNLINK_PERMISSION, appTokenId)) {
            AUDIO_ERR_LOG("downlink capturer permission check failed");
            return ERR_PERMISSION_DENIED;
        }
    }
    return audioPolicyService_.SetPlaybackCapturerFilterInfos(config);
}

int32_t AudioPolicyServer::GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc)
{
    return audioPolicyService_.GetHardwareOutputSamplingRate(desc);
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyServer::GetAudioCapturerMicrophoneDescriptors(int32_t sessionId)
{
    vector<sptr<MicrophoneDescriptor>> micDescs =
        audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(sessionId);
    return micDescs;
}

vector<sptr<MicrophoneDescriptor>> AudioPolicyServer::GetAvailableMicrophones()
{
    vector<sptr<MicrophoneDescriptor>> micDescs = audioPolicyService_.GetAvailableMicrophones();
    return micDescs;
}

int32_t AudioPolicyServer::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_BLUETOOTH_SA) {
        AUDIO_ERR_LOG("SetDeviceAbsVolumeSupported: Error caller uid: %{public}d", callerUid);
        return ERROR;
    }
    return audioPolicyService_.SetDeviceAbsVolumeSupported(macAddress, support);
}

bool AudioPolicyServer::IsAbsVolumeScene()
{
    return audioPolicyService_.IsAbsVolumeScene();
}

int32_t AudioPolicyServer::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume,
    const bool updateUi)
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != UID_BLUETOOTH_SA) {
        AUDIO_ERR_LOG("SetA2dpDeviceVolume: Error caller uid: %{public}d", callerUid);
        return ERROR;
    }

    AudioStreamType streamInFocus = AudioStreamType::STREAM_MUSIC; // use STREAM_MUSIC as default stream type
    if (audioPolicyService_.GetLocalDevicesType().compare("2in1") == 0) {
        streamInFocus = AudioStreamType::STREAM_ALL;
    } else {
        streamInFocus = GetVolumeTypeFromStreamType(GetStreamInFocus());
    }

    if (!IsVolumeLevelValid(streamInFocus, volume)) {
        return ERR_NOT_SUPPORTED;
    }
    int32_t ret = audioPolicyService_.SetA2dpDeviceVolume(macAddress, volume);
    if (ret == SUCCESS) {
        for (auto it = audioPolicyClientProxyCBMap_.begin(); it != audioPolicyClientProxyCBMap_.end(); ++it) {
            sptr<IAudioPolicyClient> volumeChangeCb = it->second;
            if (volumeChangeCb == nullptr) {
                AUDIO_ERR_LOG("volumeChangeCb: nullptr for client : %{public}d", it->first);
                continue;
            }

            AUDIO_INFO_LOG("---------SetA2dpDeviceVolume trigger volumeChangeCb clientPid : %{public}d", it->first);
            VolumeEvent volumeEvent;
            volumeEvent.volumeType = streamInFocus;
            volumeEvent.volume = volume;
            volumeEvent.updateUi = updateUi;
            volumeEvent.volumeGroupId = 0;
            volumeEvent.networkId = LOCAL_NETWORK_ID;
            volumeChangeCb->OnVolumeKeyEvent(volumeEvent);
        }
    }
    return ret;
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioPolicyServer::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> deviceDescs = {};
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
        case MEDIA_INPUT_DEVICES:
        case ALL_MEDIA_DEVICES:
        case CALL_OUTPUT_DEVICES:
        case CALL_INPUT_DEVICES:
        case ALL_CALL_DEVICES:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("GetAvailableDevices: No system permission");
                return deviceDescs;
            }
            break;
        default:
            break;
    }

    deviceDescs = audioPolicyService_.GetAvailableDevices(usage);

    if (!hasSystemPermission) {
        for (auto &desc : deviceDescs) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }

    std::vector<sptr<AudioDeviceDescriptor>> deviceDevices = {};
    for (auto &desc : deviceDescs) {
        deviceDevices.push_back(new(std::nothrow) AudioDeviceDescriptor(*desc));
    }

    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    if (!hasBTPermission) {
        audioPolicyService_.UpdateDescWhenNoBTPermission(deviceDevices);
        deviceDescs.clear();
        for (auto &dec : deviceDevices) {
            deviceDescs.push_back(make_unique<AudioDeviceDescriptor>(*dec));
        }
    }

    return deviceDescs;
}

int32_t AudioPolicyServer::SetAvailableDeviceChangeCallback(const int32_t /*clientId*/, const AudioDeviceUsage usage,
    const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "SetAvailableDeviceChangeCallback set listener object is nullptr");
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (usage) {
        case MEDIA_OUTPUT_DEVICES:
        case MEDIA_INPUT_DEVICES:
        case ALL_MEDIA_DEVICES:
        case CALL_OUTPUT_DEVICES:
        case CALL_INPUT_DEVICES:
        case ALL_CALL_DEVICES:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("SetAvailableDeviceChangeCallback: No system permission");
                return ERR_PERMISSION_DENIED;
            }
            break;
        default:
            break;
    }

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    return audioPolicyService_.SetAvailableDeviceChangeCallback(clientPid, usage, object, hasBTPermission);
}

int32_t AudioPolicyServer::UnsetAvailableDeviceChangeCallback(const int32_t /*clientId*/, AudioDeviceUsage usage)
{
    int32_t clientPid = IPCSkeleton::GetCallingPid();
    return audioPolicyService_.UnsetAvailableDeviceChangeCallback(clientPid, usage);
}

int32_t AudioPolicyServer::OffloadStopPlaying(const AudioInterrupt &audioInterrupt)
{
    return audioPolicyService_.OffloadStopPlaying(std::vector<int32_t>(1, audioInterrupt.sessionID));
}

void AudioPolicyServer::RegisterPowerStateListener()
{
    if (powerStateListener_ == nullptr) {
        powerStateListener_ = new (std::nothrow) PowerStateListener(this);
    }

    if (powerStateListener_ == nullptr) {
        AUDIO_ERR_LOG("create power state listener failed");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.RegisterSyncSleepCallback(powerStateListener_, SleepPriority::HIGH);
    if (!ret) {
        AUDIO_ERR_LOG("register sync sleep callback failed");
    } else {
        AUDIO_INFO_LOG("register sync sleep callback success");
    }
}

void AudioPolicyServer::UnRegisterPowerStateListener()
{
    if (powerStateListener_ == nullptr) {
        AUDIO_ERR_LOG("power state listener is null");
        return;
    }

    auto& powerMgrClient = OHOS::PowerMgr::PowerMgrClient::GetInstance();
    bool ret = powerMgrClient.UnRegisterSyncSleepCallback(powerStateListener_);
    if (!ret) {
        AUDIO_ERR_LOG("unregister sync sleep callback failed");
    } else {
        powerStateListener_ = nullptr;
        AUDIO_INFO_LOG("unregister sync sleep callback success");
    }
}

int32_t AudioPolicyServer::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener object is nullptr");

    sptr<IAudioPolicyClient> callback = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "RegisterPolicyCallbackClient listener obj cast failed");

    int32_t clientPid = IPCSkeleton::GetCallingPid();
    AUDIO_INFO_LOG("AudioPolicyServer::RegisterPolicyCallbackClient--------clientPid: %{public}d", clientPid);
    bool hasBTPermission = VerifyPermission(USE_BLUETOOTH_PERMISSION);
    bool hasSysPermission = PermissionUtil::VerifySystemPermission();
    callback->hasBTPermission_ = hasBTPermission;
    callback->hasSystemPermission_ = hasSysPermission;
    audioPolicyClientProxyCBMap_[clientPid] = callback;
    audioPolicyService_.SetAudioPolicyClientProxy(audioPolicyClientProxyCBMap_);
    return SUCCESS;
}

int32_t AudioPolicyServer::UnregisterPolicyCallbackClient()
{
    std::lock_guard<std::mutex> lock(volumeKeyEventMutex_);
    int32_t clientPid = IPCSkeleton::GetCallingPid();

    if (audioPolicyClientProxyCBMap_.erase(clientPid) == 0) {
        AUDIO_ERR_LOG("UnregisterPolicyCallbackClient client %{public}d not present", clientPid);
        return ERR_INVALID_OPERATION;
    }
    audioPolicyService_.SetAudioPolicyClientProxy(audioPolicyClientProxyCBMap_);
    return SUCCESS;
bool AudioPolicyServer::IsSpatializationEnabled()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationEnabled();
}

int32_t AudioPolicyServer::SetSpatializationEnabled(const bool enable)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetSpatializationEnabled(enable);
}

bool AudioPolicyServer::IsHeadTrackingEnabled()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingEnabled();
}

int32_t AudioPolicyServer::SetHeadTrackingEnabled(const bool enable)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.SetHeadTrackingEnabled(enable);
}

int32_t AudioPolicyServer::RegisterSpatializationEnabledEventListener(int32_t clientPid,
    const sptr<IRemoteObject> &object)
{
    clientPid = IPCSkeleton::GetCallingPid();
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    return audioSpatializationService_.RegisterSpatializationEnabledEventListener(
        clientPid, object, hasSystemPermission);
}

int32_t AudioPolicyServer::RegisterHeadTrackingEnabledEventListener(int32_t clientPid,
    const sptr<IRemoteObject> &object)
{
    clientPid = IPCSkeleton::GetCallingPid();
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    return audioSpatializationService_.RegisterHeadTrackingEnabledEventListener(clientPid, object, hasSystemPermission);
}

int32_t AudioPolicyServer::UnregisterSpatializationEnabledEventListener(int32_t clientPid)
{
    clientPid = IPCSkeleton::GetCallingPid();
    return audioSpatializationService_.UnregisterSpatializationEnabledEventListener(clientPid);
}

int32_t AudioPolicyServer::UnregisterHeadTrackingEnabledEventListener(int32_t clientPid)
{
    clientPid = IPCSkeleton::GetCallingPid();
    return audioSpatializationService_.UnregisterHeadTrackingEnabledEventListener(clientPid);
}

std::vector<bool> AudioPolicyServer::GetSpatializationState(const StreamUsage streamUsage)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        std::vector<bool> spatializationState;
        spatializationState.push_back(false);
        spatializationState.push_back(false);
        return spatializationState;
    }
    return audioSpatializationService_.GetSpatializationState(streamUsage);
}

bool AudioPolicyServer::IsSpatializationSupported()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationSupported();
}

bool AudioPolicyServer::IsSpatializationSupportedForDevice(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsSpatializationSupportedForDevice(address);
}

bool AudioPolicyServer::IsHeadTrackingSupported()
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingSupported();
}

bool AudioPolicyServer::IsHeadTrackingSupportedForDevice(const std::string address)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return false;
    }
    return audioSpatializationService_.IsHeadTrackingSupportedForDevice(address);
}

int32_t AudioPolicyServer::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.UpdateSpatialDeviceState(audioSpatialDeviceState);
}

int32_t AudioPolicyServer::RegisterSpatializationStateEventListener(const uint32_t sessionID,
    const StreamUsage streamUsage, const sptr<IRemoteObject> &object)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    if (!hasSystemPermission) {
        return ERR_PERMISSION_DENIED;
    }
    return audioSpatializationService_.RegisterSpatializationStateEventListener(sessionID, streamUsage, object);
}
} // namespace AudioStandard
} // namespace OHOS
