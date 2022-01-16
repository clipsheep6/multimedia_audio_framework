/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "audio_ringermode_update_listener_proxy.h"
#include "audio_system_manager.h"
#include "media_log.h"

namespace OHOS {
namespace AudioStandard {
AudioRingerModeUpdateListenerProxy::AudioRingerModeUpdateListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardRingerModeUpdateListener>(impl)
{
    MEDIA_DEBUG_LOG("Instances create");
}

AudioRingerModeUpdateListenerProxy::~AudioRingerModeUpdateListenerProxy()
{
    MEDIA_DEBUG_LOG("~AudioRingerModeUpdateListenerProxy: Instance destroy");
}

void AudioRingerModeUpdateListenerProxy::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(static_cast<int32_t>(ringerMode));
    int error = Remote()->SendRequest(ON_RINGERMODE_UPDATE, data, reply, option);
    if (error != ERR_NONE) {
        MEDIA_ERR_LOG("OnRingerModeUpdated failed, error: %{public}d", error);
    }
}

AudioRingerModeListenerCallback::AudioRingerModeListenerCallback(
    const sptr<IStandardRingerModeUpdateListener> &listener) : listener_(listener)
{
        MEDIA_DEBUG_LOG("AudioRingerModeListenerCallback: Instance create");
}

AudioRingerModeListenerCallback::~AudioRingerModeListenerCallback()
{
    MEDIA_DEBUG_LOG("AudioRingerModeListenerCallback: Instance destory");
}

void AudioRingerModeListenerCallback::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    if (listener_ != nullptr) {
        listener_->OnRingerModeUpdated(ringerMode);
    }
}
} // namespace AudioStandard
} // namespace OHOS
