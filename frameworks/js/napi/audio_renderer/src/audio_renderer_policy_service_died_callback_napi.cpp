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

#include "audio_renderer_policy_service_died_callback_napi.h"
#include "audio_renderer_device_change_callback_napi.h"

#include <uv.h>

#include "audio_errors.h"
#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioRendererPolicyServiceDiedCallbackNapi::AudioRendererPolicyServiceDiedCallbackNapi(AudioRendererNapi *rendererNapi)
    : rendererNapi_(rendererNapi)
{
    AUDIO_INFO_LOG("AudioRendererPolicyServiceDiedCallbackNapi: instance create");
}

AudioRendererPolicyServiceDiedCallbackNapi::~AudioRendererPolicyServiceDiedCallbackNapi()
{
    AUDIO_INFO_LOG("AudioRendererPolicyServiceDiedCallbackNapi: instance destroy");
}

void AudioRendererPolicyServiceDiedCallbackNapi::OnAudioPolicyServiceDied()
{
    rendererNapi_->DestroyCallbacks();
    AUDIO_INFO_LOG("AudioRendererNapi::UnegisterRendererDeviceChangeCallback is successful");
}
}  // namespace AudioStandard
}  // namespace OHOS
