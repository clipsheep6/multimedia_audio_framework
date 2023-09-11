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

#include "audio_policy_client_stub_impl.h"
#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioPolicyClientStubImpl::SetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    volumeKeyEventCallbackList_.push_back(callback);
    return SUCCESS;
}

int32_t AudioPolicyClientStubImpl::UnSetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    volumeKeyEventCallbackList_.clear();
    return SUCCESS;
}

void AudioPolicyClientStubImpl::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    for (auto it = volumeKeyEventCallbackList_.begin(; it != volumeKeyEventCallbackList_.end(); ++it)) {
        (*it)->OnVolumeKeyEvent(volumeEvent);
    }
}

void AudioPolicyClientStubImpl::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    if (eventId == static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT)) {
        VolumeEvent volumeEvent = *(event->GetUniqueObject<VolumeEvent>());
        OnVolumeKeyEvent(volumeEvent);
    }
}
} // namespace AudioStandard
} // namespace OHOS