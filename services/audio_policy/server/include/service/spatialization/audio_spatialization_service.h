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

#ifndef ST_AUDIO_SPATIALIZATION_SERVICE_H
#define ST_AUDIO_SPATIALIZATION_SERVICE_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <shared_mutex>
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"
#include "ipc_skeleton.h"

#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "parser_factory.h"
#include "audio_effect_manager.h"
#include "audio_volume_config.h"
#include "policy_provider_stub.h"

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationService : public IPortObserver, public IDeviceStatusObserver,
    public IAudioAccessibilityConfigObserver, public IPolicyProvider {
public:
    static AudioSpatializationService& GetAudioSpatializationService()
    {
        static AudioSpatializationService AudioSpatializationService;
        return AudioSpatializationService;
    }

    bool Init(void);
    void Deinit(void);
    void InitKVStore();
    bool ConnectServiceAdapter();

    const sptr<IStandardAudioService> GetAudioServerProxy();
    bool IsSpatializationEnabled();
    int32_t SetSpatializationEnabled(const bool enable);
    bool IsHeadTrackingEnabled();
    int32_t SetHeadTrackingEnabled(const bool enable);
private:
    AudioSpatializationService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())

    ~AudioSpatializationService();
    bool spatializationEnabledFlag_ = true;
    bool headTrackingEnabledFlag_ = false;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_SPATIALIZATION_SERVICE_H
