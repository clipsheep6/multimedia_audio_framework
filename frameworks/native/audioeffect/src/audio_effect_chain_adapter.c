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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "audio_effect_chain_adapter.h"
#include "audio_log.h"

#ifdef __cplusplus
extern "C" {
#endif

const int32_t SUCCESS = 0;
const int32_t ERROR = -1;

int32_t LoadEffectChainAdapter(struct EffectChainAdapter *adapter)
{
    if (adapter == NULL) {
        AUDIO_ERR_LOG("%{public}s: Invalid parameter", __func__);
        return ERROR;
    }

    // struct EffectChainAdapter *adapter = (struct EffectChainAdapter *)calloc(1, sizeof(*ecAdapter));
    // if (adapter == NULL) {
    //     AUDIO_ERR_LOG("%{public}s: alloc sink adapter failed", __func__);
    //     return ERROR;
    // }

    if (FillinEffectChainWapper(adapter) != SUCCESS) {
        AUDIO_ERR_LOG("%{public}s: Device not supported", __func__);
        free(adapter);
        return ERROR;
    }

    // adapter->EffectChainProcess = &AudioEffectChainProcess;
    // adapter->EffectChainReturnValue = &AdapterReturnValue;

    // AUDIO_INFO_LOG("xyq: LoadEffectChainAdapter");
    // int idx = adapter->EffectChainReturnValue(adapter, 2);
    // AUDIO_INFO_LOG("xyq: EffectChainReturnValue, value=%{public}d", idx);

    return SUCCESS;
}

int32_t UnLoadEffectChainAdapter(struct EffectChainAdapter *ecAdapter)
{
    if (ecAdapter == NULL) {
        AUDIO_ERR_LOG("%{public}s: Invalid parameter", __func__);
        return ERROR;
    }

    free(ecAdapter);

    return SUCCESS;
}

#ifdef __cplusplus
}
#endif