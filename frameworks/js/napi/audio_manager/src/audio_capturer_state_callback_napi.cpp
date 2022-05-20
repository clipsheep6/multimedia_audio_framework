/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_capturer_state_callback_napi.h"

#include <uv.h>

#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioCapturerStateCallbackNapi::AudioCapturerStateCallbackNapi(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("AudioCapturerStateCallbackNapi: instance create");
}

AudioCapturerStateCallbackNapi::~AudioCapturerStateCallbackNapi()
{
    AUDIO_DEBUG_LOG("AudioCapturerStateCallbackNapi: instance destroy");
}

void AudioCapturerStateCallbackNapi::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
                         "AudioCapturerStateCallbackNapi: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "AudioCapturerStateCallbackNapi: creating callback failed");

    capturerStateCallback_ = cb;
}

void AudioCapturerStateCallbackNapi::OnCapturerStateChange(const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_INFO_LOG("AudioCapturerStateCallbackNapi: OnCapturerStateChange is called");

    //CHECK_AND_RETURN_LOG(capturerChangeInfo != nullptr, "Cannot find the reference of capturer change callback");

    std::lock_guard<std::mutex> lock(mutex_);

    std::unique_ptr<AudioCapturerStateJsCallback> cb = std::make_unique<AudioCapturerStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    cb->callback = capturerStateCallback_;
    //cb->changeInfo = capturerChangeInfo;

    return OnJsCallbackCapturerState(cb);    
}

static void SetValueInt32(const napi_env& env, const std::string& fieldStr, const int intValue, napi_value& obj)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, obj, fieldStr.c_str(), value);
}

static void NativeCapturerChangeInfoToJsObj(const napi_env &env, napi_value &jsArrayChangeInfoObj,
    const vector<unique_ptr<AudioCapturerChangeInfo>> &changeInfos)
{
    napi_value jsChangeInfoObj = nullptr;
    napi_value jsCapInfoObj = nullptr;

    size_t size = changeInfos.size();
    int32_t position = 0;

    napi_create_array_with_length(env, size, &jsArrayChangeInfoObj);
    for (unique_ptr<AudioCapturerChangeInfo> changeInfo : changeInfos) {
        napi_create_object(env, &jsChangeInfoObj);
        SetValueInt32(env, "sessionId", changeInfo->sessionId, jsChangeInfoObj);
        SetValueInt32(env, "capturerState", static_cast<int32_t>(changeInfo->capturerState), jsChangeInfoObj);
        SetValueInt32(env, "clientUID", changeInfo->clientUID, jsChangeInfoObj);

        napi_create_object(env, &jsCapInfoObj);
        SetValueInt32(env, "sourceType", static_cast<int32_t>(changeInfo.capturerInfo.sourceType), jsCapInfoObj);
        SetValueInt32(env, "capturerFlags", changeInfo.capturerInfo.capturerFlags, jsCapInfoObj);
        napi_set_named_property(env, jsChangeInfoObj, "capturerInfo", jsCapInfoObj);

        napi_set_element(env, jsArrayChangeInfoObj, position, jsChangeInfoObj);
        position++;
    }
}

void AudioCapturerStateCallbackNapi::OnJsCallbackCapturerState(std::unique_ptr<AudioCapturerStateJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        AUDIO_ERR_LOG("AudioCapturerStateCallbackNapi: OnJsCallbackCapturerState: No memory");
        return;
    }

    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        // Js Thread
        AudioCapturerStateJsCallback *event = reinterpret_cast<AudioCapturerStateJsCallback *>(work->data);
        napi_env env = event->callback->env_;
        napi_ref callback = event->callback->cb_;

        do {
            napi_value jsCallback = nullptr;
            napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");

            // Call back function
            napi_value args[1] = { nullptr };
            NativeCapturerChangeInfoToJsObj(env, args[0], event->changeInfos);

            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
                " fail to convert to jsobj");

            const size_t argCount = 1;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call renderstate callback");
        } while (0);
        delete event;
        delete work;
    });
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
