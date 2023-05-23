/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "audio_histen.h"

namespace OHOS {
namespace AudioStandard {

struct HistenContext {
    const struct AudioEffectInterface *aei;
};

int32_t HistenProcess (AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer)
{
    return 1;
}

int32_t HistenCommand (AudioEffectHandle self, uint32_t cmdCode,
        AudioEffectTransInfo *cmdInfo, AudioEffectTransInfo *replyInfo)
{
    return 1;
}


const struct AudioEffectInterface g_HistenInterface = {
    HistenProcess,
    HistenCommand,
};


extern "C" bool EffectCreate(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
    auto *pContext = new HistenContext;
    pContext->aei = &g_HistenInterface;
    *handle = (AudioEffectHandle)pContext;
    // *pHandle = g_HistenInterface;
    // int32_t ret;
    // int32_t para_gain = 0;
    // ret = HistenInit(para_gain);
    // if (ret != 0) {
    //     // AUDIO_ERR_LOG("Init fail");
    //     return -1;
    // } else {
    //     // AUDIO_INFO_LOG("Init success");
    // }
    // int32_t input_gain = 1;
    // ret = HistenSet(input_gain, para_gain);
    // if (ret != 0) {
    //     // AUDIO_ERR_LOG("Set fail");
    //     return -1;
    // } else {
    //     // AUDIO_INFO_LOG("Set success");
    // }
    return 0;
}
extern "C" int32_t EffectRelease(AudioEffectHandle handle)
{
    // int32_t para_gain = 0;
    return true;
}
  
extern "C" bool EffectCheck(const AudioEffectDescriptor descriptor)
{
    // AUDIO_INFO_LOG("Get descriptor successfully");
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
extern "C" {
__attribute__((visibility("default"))) OHOS::AudioStandard::AudioEffectLibrary AELI = {
    .version = 0,
    .name = "histen",
    .implementor = "Huawei Technologies Co., Ltd",
    .checkEffect = OHOS::AudioStandard::EffectCheck,
    .createEffect = OHOS::AudioStandard::EffectCreate,
    .releaseEffect = OHOS::AudioStandard::EffectRelease
};
}
