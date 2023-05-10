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
#include "audio_effect_chain_manager.h"
#include "audio_log.h"
#include "audio_errors.h"

using namespace OHOS::AudioStandard;

int32_t FillinEffectChainWapper(struct EffectChainAdapter *adapter) {    
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null EffectChainAdapter");
    AudioEffectChainManager *instance = AudioEffectChainManager::GetInstance();
    if (instance != nullptr) {
        adapter->wapper = static_cast<void *>(instance);
    } else {
        adapter->wapper = nullptr;
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainProcess(struct EffectChainAdapter *adapter, char *streamType) {
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null EffectChainAdapter");
    AudioEffectChainManager *audioEffectChainManager = static_cast<AudioEffectChainManager *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    if (!audioEffectChainManager->ProcessEffectChain(streamType, adapter->bufIn, adapter->bufOut)) {
        return ERROR;
    }
    return SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
AudioEffectChainManager *AudioEffectChainManager::GetInstance()
{
    static AudioEffectChainManager audioEffectChainManager;
    return &audioEffectChainManager;
}

int32_t AudioEffectChainManager::ProcessEffectChain(char *streamType, void* bufIn, void *bufOut)
{
    return SUCCESS;
}

}
}