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

#ifndef ST_AUDIO_POLICY_CLIENT_PROXY_H
#define ST_AUDIO_POLICY_CLIENT_PROXY_H

#include "audio_policy_client.h"
#include "audio_system_manager.h"
#include "volume_key_event_callback_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientProxy : public IRemoteProxy<IAudioPolicyClient> {
public:
    AudioPolicyClientProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioPolicyClientProxy();
    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const uint32_t code, API_VERSION api_v);
    void UnregisterPolicyCallbackClient(const uint32_t code);
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;

private:
    static inline BrokerDelegator<AudioPolicyClientProxy> delegator_;
    int32_t SetVolumeKeyEventCallback(const sptr<IRemoteObject> &object, API_VERSION apt_v);
    std::vector<std::shared_ptr<VolumeKeyEventCallback>> volumeKeyEventCallbackList_;

    using HandlerFunc = int32_t(AudioPolicyClientProxy::*)(const sptr<IRemoteObject> &object, API_VERSION api_v);
    static inline HandlerFunc handlers[] = {
        &AudioPolicyClientProxy::SetVolumeKeyEventCallback,
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_PROXY_H