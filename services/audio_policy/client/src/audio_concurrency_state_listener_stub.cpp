/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "AudioConcurrencyStateListenerStub"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_concurrency_state_listener_stub.h"

namespace OHOS {
namespace AudioStandard {

AudioConcurrencyStateListenerStub::AudioConcurrencyStateListenerStub()
{
    AUDIO_INFO_LOG("lxj concurrencyCbStub ctor");
}

AudioConcurrencyStateListenerStub::~AudioConcurrencyStateListenerStub()
{
    AUDIO_INFO_LOG("lxj concurrencyCbStub dtor");
}


int AudioConcurrencyStateListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        AUDIO_INVALID_PARAM, "lxj ReadInterfaceToken failed");
    switch (code) {
        case ON_CONCEDE_STREAM: {
            OnConcedeStream();
            return AUDIO_OK;
        }
        default: {
            AUDIO_ERR_LOG("lxj default case, need check AudioListenerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

void AudioConcurrencyStateListenerStub::OnConcedeStream()
{
    std::shared_ptr<AudioConcurrencyCallback> cb = callback_.lock();
    if (cb != nullptr) {
        cb->OnConcedeStream();
    } else {
        AUDIO_WARNING_LOG("lxj AudioConcurrencyStateListenerStub: callback_ is nullptr");
    }
}

void AudioConcurrencyStateListenerStub::SetConcurrencyCallback(const std::weak_ptr<AudioConcurrencyCallback> &callback)
{
    AUDIO_INFO_LOG("lxj add cb to stub");
    callback_ = callback;
}

} // namespace AudioStandard
} // namespace OHOS