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
#include "audio_focus_parser.h"
#include "audio_manager_listener_stub.h"
// #include "datashare_helper.h"
// #include "datashare_predicates.h"
// #include "datashare_result_set.h"
// #include "data_share_observer_callback.h"
// #include "device_manager.h"
// #include "device_init_callback.h"
// #include "device_manager_impl.h"
// #include "uri.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const SPATIALIZATION_SERVICE_OK = 0;

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
    // AUDIO_INFO_LOG("AudioSpatializationService init");
    // serviceFlag_.reset();
    // audioPolicyManager_.Init();
    // audioEffectManager_.EffectManagerInit();

    // if (!configParser_.LoadConfiguration()) {
    //     AUDIO_ERR_LOG("Audio Config Load Configuration failed");
    //     return false;
    // }
    // if (!configParser_.Parse()) {
    //     AUDIO_ERR_LOG("Audio Config Parse failed");
    //     return false;
    // }

    // std::unique_ptr<AudioFocusParser> audioFocusParser = make_unique<AudioFocusParser>();
    // CHECK_AND_RETURN_RET_LOG(audioFocusParser != nullptr, false, "Failed to create AudioFocusParser");
    // std::string AUDIO_FOCUS_CONFIG_FILE = "system/etc/audio/audio_interrupt_policy_config.xml";

    // if (audioFocusParser->LoadConfig(focusMap_)) {
    //     AUDIO_ERR_LOG("Failed to load audio interrupt configuration!");
    //     return false;
    // }
    // AUDIO_INFO_LOG("Audio interrupt configuration has been loaded. FocusMap.size: %{public}zu", focusMap_.size());

    // if (deviceStatusListener_->RegisterDeviceStatusListener()) {
    //     AUDIO_ERR_LOG("[Policy Service] Register for device status events failed");
    //     return false;
    // }

    // RegisterRemoteDevStatusCallback();

    // // Get device type from const.product.devicetype when starting.
    // char devicesType[100] = {0}; // 100 for system parameter usage
    // (void)GetParameter("const.product.devicetype", " ", devicesType, sizeof(devicesType));
    // localDevicesType_ = devicesType;

    // if (policyVolumeMap_ == nullptr) {
    //     size_t mapSize = IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume);
    //     AUDIO_INFO_LOG("InitSharedVolume create shared volume map with size %{public}zu", mapSize);
    //     policyVolumeMap_ = AudioSharedMemory::CreateFormLocal(mapSize, "PolicyVolumeMap");
    //     CHECK_AND_RETURN_RET_LOG(policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr,
    //         false, "Get shared memory failed!");
    //     volumeVector_ = reinterpret_cast<Volume *>(policyVolumeMap_->GetBase());
    // }
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

void AudioSpatializationService::InitKVStore()
{
    // audioPolicyManager_.InitKVStore();
}

bool AudioSpatializationService::ConnectServiceAdapter()
{
    // if (!audioPolicyManager_.ConnectServiceAdapter()) {
    //     AUDIO_ERR_LOG("AudioSpatializationService::ConnectServiceAdapter  Error in connecting to audio service adapter");
    //     return false;
    // }

    // OnServiceConnected(AudioServiceIndex::AUDIO_SERVICE_INDEX);

    return true;
}

void AudioSpatializationService::Deinit(void)
{
//     AUDIO_ERR_LOG("Policy service died. closing active ports");
//     std::for_each(IOHandles_.begin(), IOHandles_.end(), [&](std::pair<std::string, AudioIOHandle> handle) {
//         audioPolicyManager_.CloseAudioPort(handle.second);
//     });

//     IOHandles_.clear();
// #ifdef ACCESSIBILITY_ENABLE
//     accessibilityConfigListener_->UnsubscribeObserver();
// #endif
//     deviceStatusListener_->UnRegisterDeviceStatusListener();

//     if (isBtListenerRegistered) {
//         UnregisterBluetoothListener();
//     }
//     volumeVector_ = nullptr;
//     policyVolumeMap_ = nullptr;

    return;
}

bool AudioSpatializationService::IsSpatializationEnabled()
{
    return spatializationEnabledFlag_;
}

int32_t AudioSpatializationService::SetSpatializationEnabled(const bool enable);
{
    if (spatializationEnabledFlag_ == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    spatializationEnabledFlag_ = enable;
    return SPATIALIZATION_SERVICE_OK;
}

bool AudioSpatializationService::IsHeadTrackingEnabled()
{
    return headTrackingEnabledFlag_;
}

int32_t AudioSpatializationService::SetHeadTrackingEnabled(const bool enable);
{
    if (headTrackingEnabledFlag_ == enable) {
        return SPATIALIZATION_SERVICE_OK;
    }
    headTrackingEnabledFlag_ = enable;
    return SPATIALIZATION_SERVICE_OK;
}
} // namespace AudioStandard
} // namespace OHOS
