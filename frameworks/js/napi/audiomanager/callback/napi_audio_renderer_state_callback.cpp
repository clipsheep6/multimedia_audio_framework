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
#ifndef LOG_TAG
#define LOG_TAG "NapiAudioRendererStateCallback"
#endif

#include "napi_audio_renderer_state_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
NapiAudioRendererStateCallback::NapiAudioRendererStateCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioRendererStateCallback: instance create");
}

NapiAudioRendererStateCallback::~NapiAudioRendererStateCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioRendererStateCallback: instance destroy");
}

void NapiAudioRendererStateCallback::RemoveCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    rendererStateCallback_.reset();
}

void NapiAudioRendererStateCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioRendererStateCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioRendererStateCallback: creating callback failed");

    rendererStateCallback_ = cb;
}

void NapiAudioRendererStateCallback::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_PRERELEASE_LOGI("OnRendererStateChange entered");

    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_LOG(rendererStateCallback_ != nullptr, "rendererStateCallback_ is nullptr!");

    std::unique_ptr<AudioRendererStateJsCallback> cb = std::make_unique<AudioRendererStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    for (const auto &changeInfo : audioRendererChangeInfos) {
        rendererChangeInfos.push_back(std::make_unique<AudioRendererChangeInfo>(*changeInfo));
    }

    cb->callback = rendererStateCallback_;
    cb->changeInfos = move(rendererChangeInfos);

    return OnJsCallbackRendererState(cb);
}

void NapiAudioRendererStateCallback::OnJsCallbackRendererState(std::unique_ptr<AudioRendererStateJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }

    AudioRendererStateJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<AudioRendererStateJsCallback> context(
            static_cast<AudioRendererStateJsCallback*>(event),
            [](AudioRendererStateJsCallback* ptr) {
                delete ptr;
        });
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
            NapiParamUtils::SetRendererChangeInfos(env, event->changeInfos, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "fail to convert to jsobj");

            const size_t argCount = ARGS_ONE;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call Interrupt callback");
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: Failed to SendEvent");
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS
