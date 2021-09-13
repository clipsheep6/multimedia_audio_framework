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

#include "audio_manager_listener_proxy.h"
#include "audio_system_manager.h"
#include "media_log.h"

namespace OHOS {
namespace AudioStandard {
AudioManagerListenerProxy::AudioManagerListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioManagerListener>(impl)
{
    MEDIA_DEBUG_LOG("Instances create");
}

AudioManagerListenerProxy::~AudioManagerListenerProxy()
{
    MEDIA_DEBUG_LOG("Instances destroy");
}

void AudioManagerListenerProxy::OnError(AudioManagerErrorType errorType, int32_t errorCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInt32(errorType);
    data.WriteInt32(errorCode);
    int error = Remote()->SendRequest(AudioManagerListenerMsg::ON_ERROR, data, reply, option);
    if (error != ERR_OK) {
        MEDIA_ERR_LOG("on error failed, error: %{public}d", error);
    }
}

void AudioManagerListenerProxy::OnInfo(AudioManagerInfoType type, int32_t extra)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInt32(type);
    data.WriteInt32(extra);
    int error = Remote()->SendRequest(AudioManagerListenerMsg::ON_INFO, data, reply, option);
    if (error != ERR_OK) {
        MEDIA_ERR_LOG("on info failed, error: %{public}d", error);
    }
}
AudioManagerListenerCallback::AudioManagerListenerCallback(const sptr<IStandardAudioManagerListener> &listener)
    : listener_(listener)
{
}

AudioManagerListenerCallback::~AudioManagerListenerCallback()
{
    if (listener_ != nullptr && listener_->AsObject() != nullptr) {
        listener_ = nullptr;
    }
    MEDIA_DEBUG_LOG("Instances destory");
}

void AudioManagerListenerCallback::OnError(AudioManagerErrorType errorType, int32_t errorCode)
{
    MEDIA_ERR_LOG("AudioManager callback onError, errorType: %{public}d, errorCode: %{public}d", errorType, errorCode);
    if (listener_ != nullptr) {
        listener_->OnError(errorType, errorCode);
    }
}

void AudioManagerListenerCallback::OnInfo(AudioManagerInfoType type, int32_t extra)
{
    if (listener_ != nullptr) {
        listener_->OnInfo(type, extra);
    }
}
} // namespace AudioStandard
} // namespace OHOS
