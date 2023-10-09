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

#ifndef AUDIO_POLICY_CLIENT_STUB_H
#define AUDIO_POLICY_CLIENT_STUB_H

#include "event_handler.h"
#include "event_runner.h"
#include "audio_policy_client.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyClientStub : public IRemoteStub<IAudioPolicyClient>, public AppExecFwk::EventHandler {
public:
    AudioPolicyClientStub();
    ~AudioPolicyClientStub();
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
private:
    void HandleVolumeKeyEvent(MessageParcel &data, MessageParcel &reply);

    using HandlerFunc = void (AudioPolicyClientStub::*)(MessageParcel &data, MessageParcel &reply);
    static inline HandlerFunc handlers[]  = {
        &AudioPolicyClientStub::HandleVolumeKeyEvent,
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_CLIENT_STUB_H