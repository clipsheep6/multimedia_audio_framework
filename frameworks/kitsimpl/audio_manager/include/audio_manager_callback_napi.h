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

#ifndef AUDIO_MANAGER_CALLBACK_NAPI_H_
#define AUDIO_MANAGER_CALLBACK_NAPI_H_

#include "audio_manager_napi.h"
#include "audio_system_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AudioStandard {
const std::string ERROR_CALLBACK_NAME = "error";
const std::string DEVICE_CHANGE_CALLBACK_NAME = "deviceChange";
const std::string INTERRUPT_CALLBACK_NAME = "interrupt";

struct AudioManagerJsCallback {
    napi_env env = nullptr;
    napi_ref callback = nullptr;
    std::string callbackName = "unknown";
    std::string errorType = "unknown";
    std::string errorCode = "unknown";
    int32_t position = -1;
};

class AudioManagerCallbackNapi : public AudioManagerCallback {
public:
    AudioManagerCallbackNapi(napi_env env, AudioManagerNapi &audioManagerNapi);
    virtual ~AudioManagerCallbackNapi();

protected:
    void OnError(AudioManagerErrorType errorType, int32_t errorCode) override;
    void OnInfo(AudioManagerInfoType type, int32_t extra) override;

private:
    void OnDeviceChangeCb(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    void OnInterruptCb(InterruptType interruptType);
    void OnJsCallback(AudioManagerJsCallback *jsCb);
    void OnJsCallbackError(AudioManagerJsCallback *jsCb);
    void OnJsCallbackPosition(AudioManagerJsCallback *jsCb);

private:
    napi_env env_ = nullptr;
    AudioManagerNapi &managerNapi_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_MANAGER_CALLBACK_NAPI_H_
