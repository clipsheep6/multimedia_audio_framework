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
#ifndef NAPI_AUDIO_INTERRUPT_MANAGER_H
#define NAPI_AUDIO_INTERRUPT_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_INTERRUPT_MANAGER_NAPI_CLASS_NAME = "AudioInterruptManager";

class NapiAudioInterruptManager {
public:
    NapiAudioInterruptManager();
    ~NapiAudioInterruptManager();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value CreateInterruptManagerWrapper(napi_env env);

private:
    struct AudioInterruptManagerAsyncContext : public ContextBase {
        napi_env env;
        napi_async_work work;
        napi_deferred deferred;
        napi_ref callbackRef = nullptr;
        int32_t deviceFlag;
        bool bArgTransFlag = true;
        int32_t status = SUCCESS;
        int32_t groupId;
        std::string networkId;
    };

    static napi_value Construct(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalizeHint);
    AudioSystemManager *audioSystemMngr_;

    napi_env env_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_INTERRUPT_MANAGER_H */
