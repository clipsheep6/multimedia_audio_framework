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

#include <uv.h>

#include "audio_manager_callback_napi.h"
#include "audio_errors.h"
#include "media_log.h"

namespace {
    // constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioManagerCallbackNapi"};
}

namespace OHOS {
namespace AudioStandard {
AudioManagerCallbackNapi::AudioManagerCallbackNapi(napi_env env, AudioManagerNapi &managerNapi)
    : env_(env),
      managerNapi_(managerNapi)
{
}

AudioManagerCallbackNapi::~AudioManagerCallbackNapi()
{
}

void AudioManagerCallbackNapi::OnError(AudioManagerErrorType errorType, int32_t errorCode)
{
    MEDIA_DEBUG_LOG("OnError is called, type: %{public}d, error code: %{public}d", errorType, errorCode);
    CHECK_AND_RETURN_LOG(env_ != nullptr, "env is nullptr");
    CHECK_AND_RETURN_LOG(managerNapi_.errorCallback_ != nullptr, "errorCallback_ is nullptr");
    AudioManagerJsCallback *cb = new(std::nothrow) AudioManagerJsCallback {
        .env = env_,
        .callback = managerNapi_.errorCallback_,
        .callbackName = ERROR_CALLBACK_NAME,
        .errorType = AudioManagerErrorTypeToString(errorType),
        .errorCode = ASErrorToString(static_cast<AudioServiceErrCode>(errorCode)),
    };
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    return OnJsCallbackError(cb);
}

void OnDeviceChangeCb(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor)
{
    MEDIA_DEBUG_LOG("OnDeviceChangeCb is called");
}

void AudioManagerCallbackNapi::OnInterruptCb(InterruptType interruptType) {
    MEDIA_DEBUG_LOG("OnDeviceInterruptCb is called");
}

void AudioManagerCallbackNapi::OnInfo(AudioManagerInfoType type, int32_t extra)
{
    MEDIA_INFO_LOG("OnInfo is called, AudioManagerInfoType: %{public}d", type);
    switch (type) {
        case INFO_TYPE_INTERRUPT:
            OnInterruptCb(static_cast<InterruptType>(extra));
            break;
        case INFO_TYPE_DEVICE_CHANGE:
            // OnDeviceChangeCb(extra);
            break;
        default:
            break;
    }
    MEDIA_DEBUG_LOG("send OnInfo callback success");
}

void AudioManagerCallbackNapi::OnJsCallbackError(AudioManagerJsCallback *jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        MEDIA_ERR_LOG("fail to new uv_work_t");
        delete jsCb;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb);

    uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        // Js Thread
        AudioManagerJsCallback *event = reinterpret_cast<AudioManagerJsCallback *>(work->data);
        std::string request = event->callbackName;
        MEDIA_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work start", request.c_str());
        do {
            napi_value jsCallback = nullptr;
            status = napi_get_reference_value(event->env, event->callback, &jsCallback);
            CHECK_AND_BREAK_LOG(status == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
                request.c_str());

            napi_value errorTypeVal = nullptr;
            status = napi_create_string_utf8(event->env, event->errorType.c_str(), NAPI_AUTO_LENGTH, &errorTypeVal);
            CHECK_AND_BREAK_LOG(status == napi_ok, "%{public}s fail to get error type value", request.c_str());

            napi_value errorCodeVal = nullptr;
            status = napi_create_string_utf8(event->env, event->errorCode.c_str(), NAPI_AUTO_LENGTH, &errorCodeVal);
            CHECK_AND_BREAK_LOG(status == napi_ok, "%{public}s fail to get error code value", request.c_str());

            napi_value args[1] = { nullptr };
            status = napi_create_error(event->env, errorTypeVal, errorCodeVal, &args[0]);
            CHECK_AND_BREAK_LOG(status == napi_ok, "%{public}s fail to create error callback", request.c_str());

            // Call back function
            const size_t argCount = 1;
            napi_value result = nullptr;
            status = napi_call_function(event->env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(status == napi_ok, "%{public}s fail to napi call function", request.c_str());

            uint32_t thisRefCount = 0;
            napi_reference_unref(event->env, event->callback, &thisRefCount);
        } while (0);
        delete event;
        delete work;
    });
}
}  // namespace AudioStandard
}  // namespace OHOS
