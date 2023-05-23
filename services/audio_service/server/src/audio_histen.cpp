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
int32_t HistenInit(int32_t &para_gain)
{
    para_gain = 0;
    return 0;
}
int32_t HistenSet(int32_t &input_gain, int32_t &para_gain)
{
    para_gain = input_gain;
    return 0;
}
int32_t HistenApply(int32_t &input_signal, int32_t &para_gain)
{
    input_signal *= para_gain;
    return 0;
}
// const struct HistenEffectInterface g_HistenInterface = {
//     HistenInit,
//     HistenSet,
//     HistenApply,
// };
extern "C" bool EffectCreate(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
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
