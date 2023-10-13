/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "audio_policy_client_stub.h"
#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyClientStub::AudioPolicyClientStub()
    : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create("AudioPolicyClientStubRunner"))
{}

AudioPolicyClientStub::~AudioPolicyClientStub()
{}

int AudioPolicyClientStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        AUDIO_ERR_LOG("AudioPolicyClientStub: ReadInterfaceToken failed");
        return -1;
    }
    switch (code) {
        case UPDATE_POLICY_CALLBACK_CLIENT: {
            uint32_t updateCode = static_cast<uint32_t>(data.ReadInt32());
            if (updateCode > static_cast<uint32_t>(AudioPolicyClientCode::AUDIO_POLICY_CLIENT_CODE_MAX)) {
                return -1;
            }
            (this->*handlers[updateCode])(data, reply);
            break;
        }
        default: {
            reply.WriteInt32(ERR_INVALID_OPERATION);
            break;
        }
    }
    return SUCCESS;
}

void AudioPolicyClientStub::HandleVolumeKeyEvent(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<VolumeEvent> object = std::make_unique<VolumeEvent>();
    object->volumeType = static_cast<AudioStreamType>(data.ReadInt32());
    object->volume = data.ReadInt32();
    object->updateUi = data.ReadBool();
    object->volumeGroupId = data.ReadInt32();
    object->networkId = data.ReadString();
    SendEvent(AppExecFwk::InnerEvent::Get(static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT), object));
    reply.WriteInt32(SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS