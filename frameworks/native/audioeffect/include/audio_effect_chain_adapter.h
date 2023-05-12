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

#ifndef AUDIO_EFFECT_CHAIN_ADAPTER_H
#define AUDIO_EFFECT_CHAIN_ADAPTER_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct EffectChainAdapter {
    void *wapper; // AudioEffectChainManager instance
    void *bufIn; // input buffer, output of the effect sink
    void *bufOut; // output buffer for the final processed output
};

int32_t LoadEffectChainAdapter(struct EffectChainAdapter *adapter);
int32_t UnLoadEffectChainAdapter(struct EffectChainAdapter *adapter);

// functions for cpp
int32_t FillinEffectChainWapper(struct EffectChainAdapter *adapter);
int32_t EffectChainManagerProcess(struct EffectChainAdapter *adapter, char *streamType);
int32_t EffectChainManagerGetFrameLen(struct EffectChainAdapter *adapter);
int32_t EffectChainManagerReturnValue(struct EffectChainAdapter *adapter, int32_t i);

#ifdef __cplusplus
}
#endif
#endif // AUDIO_EFFECT_CHAIN_ADAPTER_H
