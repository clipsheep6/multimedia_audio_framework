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

#include <utility>
#include "audio_policy_client_stub.h"
#include "audio_errors.h"
#include "audio_log.h"

using namespace std;
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
        case UPDATE_CALLBACK_CLIENT: {
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

void AudioPolicyClientStub::HandleAudioFocusInfoChange(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<std::list<std::pair<AudioInterrupt, AudioFocuState>>>object =
        std::make_unique<std::list<std::pair<AudioInterrupt, AudioFocuState>>>();
    std::pair<AudioInterrupt, AudioFocuState> focusInfo = {};
    int32_t size = data.ReadInt32();
    for (uint32_t i = 0; i < size; i++) {
        focusInfo.first.Unmarshalling(data);
        focusInfo.second = static_cast<AudioFocuState>(data.ReadInt32());
        object->emplace_back(focusInfo);
    }
    SendEvent(AppExecFwk::InnerEvent::Get(static_cast<uint32_t>(AudioPolicyClientCode::ON_FOCUS_INFO_CHANGED), object));
    reply.WriteInt32(SUCCESS);
}

void AudioPolicyClientStub::HandleDeviceChange(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<DeviceChangeAction> object = std::make_unique<DeviceChangeAction>();
    object->type = static_cast<DeviceChangeType>(data.ReadUint32());
    object->flag = static_cast<DeviceFlag>(data.ReadUint32());
    size_t size = data.ReadInt32();
    for (uint32_t i = 0; i < size; i++) {
        object->deviceDescriptors.emplace_back(AudioDeviceDescriptor::Unmarshalling(data));
    }
    SendEvent(AppExecFwk::InnerEvent::Get(static_cast<uint32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE), object));
    reply.WriteInt32(SUCCESS);
}

void AudioPolicyClientStub::HandleAudioInterrupt(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<InterruptEventInternal> object = std::make_unique<InterruptEventInternal>();
    object->eventType = static_cast<InterruptType>(data.ReadInt32());
    object->forceType = static_cast<InterruptForceType>(data.ReadInt32());
    object->hintType = static_cast<InterruptHint>(data.ReadInt32());
    object->duckVolume = data.ReadFloat();
    SendEvent(AppExecFwk::InnerEvent::Get(static_cast<uint32_t>(AudioPolicyClientCode::ON_INTERRUPT), object));
    reply.WriteInt32(SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
