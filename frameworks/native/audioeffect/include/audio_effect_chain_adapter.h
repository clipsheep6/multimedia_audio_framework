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

struct BufferAttrAdapter {
    float *bufIn;
    float *bufOut;
    int numChan;
    int frameLen;
};

struct AudioBufferAdapter {
    size_t frameLength;
    union {
        void*     raw;
        float*    f32;
        int32_t*  s32;
        int16_t*  s16;
        uint8_t*  u8;
    };
};

struct AudioBufferConfigAdapter {
    struct AudioBufferAdapter buffer;
    uint32_t samplingRate;
    uint32_t channels;
    uint8_t format;
};

struct AudioEffectConfigAdapter {
    struct AudioBufferConfigAdapter inputCfg;
    struct AudioBufferConfigAdapter outputCfg;
};

int32_t EffectChainManagerProcess(void *ioBufferConfig, char *sceneType);
int32_t EffectChainManagerGetFrameLen();

#ifdef __cplusplus
}
#endif
#endif // AUDIO_EFFECT_CHAIN_ADAPTER_H
