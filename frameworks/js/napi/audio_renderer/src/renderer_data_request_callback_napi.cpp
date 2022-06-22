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

#include "renderer_data_request_callback_napi.h"

#include <uv.h>

#include "audio_errors.h"
#include "audio_log.h"

namespace {
    const std::string RENDERER_DATA_REQUEST_CALLBACK_NAME = "dataRequest";
}

namespace OHOS {
namespace AudioStandard {
RendererDataRequestCallbackNapi::RendererDataRequestCallbackNapi(napi_env env)
    : env_(env)
{
    AUDIO_INFO_LOG("RendererDataRequestCallbackNapi: instance create");
}

RendererDataRequestCallbackNapi::~RendererDataRequestCallbackNapi()
{
    AUDIO_INFO_LOG("RendererDataRequestCallbackNapi: instance destroy");
}

void RendererDataRequestCallbackNapi::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
                         "RendererDataRequestCallbackNapi: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == RENDERER_DATA_REQUEST_CALLBACK_NAME) {
        rendererDataRequestCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("RendererDataRequestCallbackNapi: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void RendererDataRequestCallbackNapi::OnWriteData(size_t length)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("RendererDataRequestCallbackNapi: onDataRequest");
    CHECK_AND_RETURN_LOG(rendererDataRequestCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<RendererDataRequestJsCallback> cb = std::make_unique<RendererDataRequestJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = rendererDataRequestCallback_;
    cb->callbackName = RENDERER_DATA_REQUEST_CALLBACK_NAME;
    AudioRendererDataInfo audioRendererDataInfo = {};
    cb->audioRendererDataInfo = audioRendererDataInfo;
    return OnJsRendererDataRequestCallback(cb);
}

static void NativeAudioRendererDataInfoToJsObj(const napi_env& env, napi_value& jsObj,
    const AudioRendererDataInfo &audioRendererDataInfo)
{
    napi_create_object(env, &jsObj);
}

void RendererDataRequestCallbackNapi::OnJsRendererDataRequestCallback(
    std::unique_ptr<RendererDataRequestJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        AUDIO_ERR_LOG("RendererDataRequestCallbackNapi: OnJsRendererPeriodPositionCallback: No memory");
        return;
    }
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("RendererDataRequestCallbackNapi: OnJsRendererPeriodPositionCallback: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        // Js Thread
        RendererDataRequestJsCallback *event = reinterpret_cast<RendererDataRequestJsCallback *>(work->data);
        std::string request = event->callbackName;
        napi_env env = event->callback->env_;
        napi_ref callback = event->callback->cb_;
        AUDIO_DEBUG_LOG("RendererDataRequestCallbackNapi: JsCallBack %{public}s, uv_queue_work start",
            request.c_str());
        do {
            CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

            napi_value jsCallback = nullptr;
            napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
                request.c_str());

            napi_value args[1] = { nullptr };
            NativeAudioRendererDataInfoToJsObj(env, args[0], event->audioRendererDataInfo);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
                "%{public}s fail to create position callback", request.c_str());

            const size_t argCount = 1;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call position callback", request.c_str());
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
