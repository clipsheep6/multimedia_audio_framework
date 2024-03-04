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
#undef LOG_TAG
#define LOG_TAG "NapiAudioCapturerStateCallback"

#include "napi_audio_capturer_state_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
NapiAudioCapturerStateCallback::NapiAudioCapturerStateCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioCapturerStateCallback: instance create");
}

NapiAudioCapturerStateCallback::~NapiAudioCapturerStateCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioCapturerStateCallback: instance destroy");
}

void NapiAudioCapturerStateCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioCapturerStateCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioCapturerStateCallback: creating callback failed");

    capturerStateCallback_ = cb;
}

void NapiAudioCapturerStateCallback::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_INFO_LOG("OnCapturerStateChange is called");

    std::lock_guard<std::mutex> lock(mutex_);
    std::unique_ptr<AudioCapturerStateJsCallback> cb = std::make_unique<AudioCapturerStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    for (const auto &changeInfo : audioCapturerChangeInfos) {
        capturerChangeInfos.push_back(std::make_unique<AudioCapturerChangeInfo>(*changeInfo));
    }

    cb->callback = capturerStateCallback_;
    cb->changeInfos = move(capturerChangeInfos);

    return OnJsCallbackCapturerState(cb);
}

void NapiAudioCapturerStateCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioCapturerStateJsCallback> context(
        static_cast<AudioCapturerStateJsCallback*>(work->data),
        [work](AudioCapturerStateJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioCapturerStateJsCallback *event = reinterpret_cast<AudioCapturerStateJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetCapturerChangeInfos(env, event->changeInfos, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "fail to convert to jsobj");

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call Interrupt callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioCapturerStateCallback::OnJsCallbackCapturerState(std::unique_ptr<AudioCapturerStateJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackCapturerState: No memory");

    work->data = reinterpret_cast<void *>(jsCb.get());
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackInterruptDone,
        uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS