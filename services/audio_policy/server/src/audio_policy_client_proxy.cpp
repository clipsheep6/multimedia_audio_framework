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

#include "audio_policy_client_proxy.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioPolicyClientProxy::AudioPolicyClientProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAudioPolicyClient>(impl)
{}

AudioPolicyClientProxy::~AudioPolicyClientProxy()
{}

VolumeKeyEventCallbackListener::VolumeKeyEventCallbackListener(const sptr<IAudioPolicyClient> &listener)
: listener_(listener) {}

VolumeKeyEventCallbackListener::~VolumeKeyEventCallbackListener() {}

int32_t AudioPolicyClientProxy::RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const uint32_t code, API_VERSION api_v)
{
    if (code > static_cast<uint32_t>(AudioPolicyClientCode::AUDIO_POLICY_CLIENT_CODE_MAX)) {
        return -1;
    }
    return (this->*handlers[code])(object, api_v);
}

void AudioPolicyClientProxy::UnregisterPolicyCallbackClient(const uint32_t code)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT):
            volumeKeyEventCallbackList_.clear();
            break;
        default:
            break;
    }
}

int32_t AudioPolicyClientProxy::SetVolumeKeyEventCallback(const sptr<IRemoteObject> &object, API_VERSION api_v)
{
    if (api_v == API_8 && !PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("SetVolumeKeyEventCallback: No system permission");
        return ERR_PERMISSION_DENIED;
    }
    sptr<IAudioPolicyClient> listener = iface_cast<IAudioPolicyClient>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "SetVolumeKeyEventCallback listener obj cast failed");
    std::shared_ptr<VolumeKeyEventCallback> callback = std::make_shared<VolumeKeyEventCallbackListener>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "SetVolumeKeyEventCallback failed to create cb obj");
    volumeKeyEventCallbackList_.push_back(callback);
    return SUCCESS;
}

void AudioPolicyClientProxy::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    if (volumeKeyEventCallbackList_.empty()) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AUDIO_ERR_LOG("AudioPolicyClientProxy: WriteInterfaceToken failed");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT));
    data.WriteInt32(static_cast<int32_t>(volumeEvent.volumeType));
    data.WriteInt32(volumeEvent.volume);
    data.WriteBool(volumeEvent.updateUi);
    data.WriteInt32(volumeEvent.volumeGroupId);
    data.WriteString(volumeEvent.networkId);
    int error = Remote()->SendRequest(static_cast<uint32_t>(UPDATE_POLICY_CALLBACK_CLIENT), data, reply, option);
    if (error != 0) {
        AUDIO_DEBUG_LOG("Error while sending volume key event %{public}d", error);
    }
    reply.ReadInt32();
}
} // namespace AudioStandard
} // namespace OHOS
