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
        private:
            std::string sceneType;
            std::vector<EffectHandleT *> standByEffectHandles;

        public:
            AudioEffectChain(std::string scene);

            ~AudioEffectChain();

            // 设置standByEffectChain
            void SetEffectChain(std::vector<EffectHandleT *> effectHandles);

            // 依次执行stanbyEffectChain中的handle
            void ApplyEffectChain(void *bufIn, void *bufOut);
        };

        class AudioEffectChainManager {
        private:
            std::map<std::string, LibEntryT *> EffectToLibraryEntryMap;  // {"hvs": libEntryT}
            std::map <std::string, std::vector<std::string>> EffectChainToEffectsMap; // {"EFFECTCHAIN_SPK_MUSIC": [hvs, eq, histen]}
            std::map<std::string, AudioEffectChain *> SceneTypeToEffectChainMap; // {"STREAM_MUSIC": AudioEffectChain}  init时创建AudioEffectChain对象

        public:
            AudioEffectChainManager();
            ~AudioEffectChainManager();
            static AudioEffectChainManager *GetInstance();
            // 初始化 SceneTypeToEffectChainMap, mode和device与effectchain一一对应
            void InitAudioEffectChain(std::vector<EffectChain> effectChains, std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList);
            // 设置根据mdInfo创建effect handle
            int32_t SetAudioEffectChain(std::string sceneType, std::string effectChain);
            int32_t ApplyAudioEffectChain(std::string sceneType, void *bufIn, void *bufOut);
            int ReturnValue(int i);
        };


    }  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_CHAIN_MANAGER_H