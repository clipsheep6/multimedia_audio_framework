/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


#ifndef AUDIO_EFFECT_CHAIN_MANAGER_H
#define AUDIO_EFFECT_CHAIN_MANAGER_H

#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "audio_info.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

class AudioEffectChain {
public:
    AudioEffectChain(std::string scene);
    ~AudioEffectChain();
    void SetEffectChain(std::vector<EffectHandleT *> effectHandles);
    void ApplyEffectChain(void *bufIn, void *bufOut);
private:
    std::string sceneType;
    std::vector<EffectHandleT *> standByEffectHandles;
};

class AudioEffectChainManager {
public:
    AudioEffectChainManager();
    ~AudioEffectChainManager();
    static AudioEffectChainManager *GetInstance();
    void InitAudioEffectChain(std::vector<EffectChain> effectChains, std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList);    
    int32_t SetAudioEffectChain(std::string sceneType, std::string effectChain);
    int32_t ApplyAudioEffectChain(std::string sceneType, void *bufIn, void *bufOut);
    int32_t GetFrameLen();
    int32_t SetFrameLen(int32_t frameLen);
private:
    std::map<std::string, LibEntryT *> EffectToLibraryEntryMap;  // {"hvs": libEntryT}
    std::map <std::string, std::vector<std::string>> EffectChainToEffectsMap; // {"EFFECTCHAIN_SPK_MUSIC": [hvs, eq, histen]}
    std::map<std::string, AudioEffectChain *> SceneTypeToEffectChainMap; // {"STREAM_MUSIC": AudioEffectChain}  Create AudioEffectChain when init
    int32_t frameLen = 960;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_CHAIN_MANAGER_H