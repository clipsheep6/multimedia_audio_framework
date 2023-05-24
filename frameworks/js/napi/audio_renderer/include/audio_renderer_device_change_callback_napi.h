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

#ifndef AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_NAPI_H_
#define AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_NAPI_H_

#include <uv.h>
#include <list>
#include "audio_common_napi.h"
#include "audio_renderer.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"


namespace OHOS {
namespace AudioStandard {
class AudioRendererDeviceChangeCallbackNapi : public AudioRendererDeviceChangeCallback {
public:
    explicit AudioRendererDeviceChangeCallbackNapi(napi_env env);
    virtual ~AudioRendererDeviceChangeCallbackNapi();
    void AddCallbackReference(napi_value callback);
    void RemoveCallbackReference(napi_env env, napi_value args);
    void OnStateChange(const DeviceInfo &deviceInfo) override;
    void RemoveAllCallbacks() override;
    int32_t GetCallbackListSize() const;

private:
    struct AudioRendererDeviceChangeJsCallback {
        napi_ref callback_;
        napi_env env_;
        DeviceInfo deviceInfo_;
    };

    void OnJsCallbackRendererDeviceInfo(napi_ref method, const DeviceInfo &deviceInfo);
    static void WorkCallbackCompleted(uv_work_t* work, int aStatus);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<napi_ref> callbacks_ {};
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_NAPI_H_
