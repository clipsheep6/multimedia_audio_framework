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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_audio_enum.h"

namespace OHOS {
namespace AudioStandard {
static napi_value Init(napi_env env, napi_value exports)
{
    // NapiAudioManager::Init(env, exports);
    // NapiAudioCapturer::Init(env, exports);
    // NapiAudioRenderer::Init(env, exports);
#ifdef FEATURE_DTMF_TONE
    // TonePlayerNapi::Init(env, exports);
#endif
    // NapiAudioParameters::Init(env, exports);
    // NapiAudioStreamMgr::Init(env, exports);
    // NapiAudioRoutingManager::Init(env, exports);
    // NapiAudioVolumeGroupManager::Init(env, exports);
    // NapiAudioVolumeManager::Init(env, exports);
    // NapiAudioInterruptManager::Init(env, exports);
    NapiAudioEnum::Init(env, exports);
    return exports;
}

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "multimedia.audio",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
} // namespace AudioStandard
} // namespace OHOS