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

#include "audio_manager_listener_stub.h"
#include "audio_system_manager.h"
#include "media_log.h"

namespace OHOS {
namespace AudioStandard {

AudioManagerListenerStub::AudioManagerListenerStub()
{
    MEDIA_DEBUG_LOG("AudioManagerLiterner Instance create");
}

AudioManagerListenerStub::~AudioManagerListenerStub()
{
    MEDIA_DEBUG_LOG("AudioManagerLiterner Instance destroy");
}

int AudioManagerListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case ON_ERROR: {
            int32_t errorType = data.ReadInt32();
            int32_t errorCode = data.ReadInt32();
            OnError(static_cast<AudioManagerErrorType>(errorType), errorCode);
            return MEDIA_OK;
        }
        case ON_INFO: {
            int32_t type = data.ReadInt32();
            int32_t extra = data.ReadInt32();
            // MEDIA_DEBUG_LOG("0x%{public}06" PRIXPTR " listen stub on info type: %{public}d  extra %{public}d",
                       // FAKE_POINTER(this), type, extra);
            OnInfo(static_cast<AudioManagerInfoType>(type), extra);
            return MEDIA_OK;
        }
        default: {
            MEDIA_ERR_LOG("default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioManagerListenerStub::OnError(AudioManagerErrorType errorType, int32_t errorCode)
{
    std::shared_ptr<AudioManagerCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnError(errorType, errorCode);
    }
}

void AudioManagerListenerStub::OnInfo(AudioManagerInfoType type, int32_t extra)
{
    std::shared_ptr<AudioManagerCallback> cb = callback_.lock();
    CHECK_AND_RETURN(cb != nullptr);
    switch (type) {
        case INFO_TYPE_INTERRUPT:
            cb->OnInfo(INFO_TYPE_INTERRUPT, extra);
            break;
        case INFO_TYPE_DEVICE_CHANGE:
            // cb->OnInfo(INFO_TYPE_VOLUME_CHANGE, extra, infoBody);
            break;
        default:
            MEDIA_ERR_LOG("default case, need check AudioManagerListenerStub");
            break;
    }
}

void AudioManagerListenerStub::SetCallback(const std::weak_ptr<AudioManagerCallback> &callback)
{
    callback_ = callback;
}
} // namespace AudioStandard
} // namespace OHOS
