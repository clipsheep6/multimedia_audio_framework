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

#ifndef ST_AUDIO_HISTEN_H
#define ST_AUDIO_HISTEN_H

#include <stdint.h>
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

struct HistenEffectInterface {
    int32_t (*HistenInit) (int32_t &para_gain);
    int32_t (*HistenSet) (int32_t &input_gain, int32_t &para_gain);
    int32_t (*HistenApply) (int32_t &input_signal, int32_t &para_gain);
};


// struct AudioEffectLibrary {
//     // int32_t (*createEffect) (const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);
//     // int32_t (*releaseEffect) (AudioEffectHandle handle);
//     // bool (*checkEffect) (const AudioEffectDescriptor descriptor);
//     uint32_t version;
//     const char *name;
//     const char *implementor;
//     bool (*checkEffect) (const AudioEffectDescriptor descriptor);
//     int32_t (*createEffect) (const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);
//     int32_t (*releaseEffect) (AudioEffectHandle handle);
// };

} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SERVER_H
