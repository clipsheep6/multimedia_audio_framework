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
#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <memory>
#include <vector>
#include <set>

#include "audio_effect_chain_adapter.h"
#include "audio_effect_chain_manager.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_info.h"

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
        AudioEffectChain::AudioEffectChain(std::string scene) {
            streamType = scene;
        }

        AudioEffectChain::~AudioEffectChain() {}

        void AudioEffectChain::SetEffectChain(std::vector<EffectHandleT *> effectHandles) {
            standByEffectHandles.clear();
            for (EffectHandleT *handleT: effectHandles) {
                standByEffectHandles.emplace_back(handleT);
            }
        }

        void AudioEffectChain::ApplyEffectChain(void *bufIn, void *bufOut) {
//            AUDIO_ERR_LOG("<log error> could not find library %{public}s to load effect %{public}s",
//                          effect.libraryName.c_str(), effect.name.c_str());

            AUDIO_INFO_LOG("apply effect chain beginning");
            for (EffectHandleT *handle: standByEffectHandles) {
                AUDIO_INFO_LOG("run effect: %{public}p", handle);
//                handle(bufIn, bufOut));
            }
        }

        LibEntryT *findlibOfEffect(std::string effect, std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList) {
            for (const std::unique_ptr <LibEntryT> &lib : effectLibraryList) {
                for (std::unique_ptr <EffectDescriptorT> &e: lib->effects) {
                    if (e->name == effect) {
                        return lib.get();
                    }
                }
            }
            return nullptr;
        }

        void AudioEffectChainManager::InitAudioEffectChain(std::vector<EffectChain> effectChains,
                                                           std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList) {

            std::set<std::string> effectSet;
            for (EffectChain efc: effectChains){
                for(std::string effect: efc.apply){
                    effectSet.insert(effect);
                }
            }

            // make EffectToLibraryEntryMap
            for (std::string effect: effectSet) {
                auto *libEntry = findlibOfEffect(effect, effectLibraryList);
                if (!libEntry) {
//                    std::cout << "libEntry is nil while find effect:" << effect.name << std::endl;
                }

                EffectToLibraryEntryMap[effect] = libEntry;
            }

            // make EffectChainToEffectsMap
            for (EffectChain efc: effectChains) {
                std::string key = efc.name;
                std::vector <std::string> effects;
                for (std::string effectName: efc.apply) {
                    effects.emplace_back(effectName);
                }
                EffectChainToEffectsMap[key] = effects;
            }
        }

        int32_t AudioEffectChainManager::SetAudioEffectChain(std::string streamType, std::string effectChain) {
            AudioEffectChain *audioEffectChain = new AudioEffectChain(streamType);
            StreamTypeToEffectChainMap[streamType] = audioEffectChain;

            std::vector <std::string> effectNames = EffectChainToEffectsMap[effectChain];

            std::vector < EffectHandleT * > effectHandles;
//            std::cout << "create effectchain:" << effectChain << std::endl;
            for (std::string effect: effectNames) {
                EffectHandleT handle;
                EffectToLibraryEntryMap[effect]->desc->CreateEffect(&effect, 0, 0, &handle);
                effectHandles.emplace_back(&handle);
            }

            audioEffectChain->SetEffectChain(effectHandles);
            return 0;
        }

        int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string streamType, void *bufIn, void *bufOut) {
            auto *audioEffectChain = StreamTypeToEffectChainMap[streamType];
            audioEffectChain->ApplyEffectChain(bufIn, bufOut);
            return 0;
        }

        AudioEffectChainManager::AudioEffectChainManager() {}

        AudioEffectChainManager::~AudioEffectChainManager() {}

        AudioEffectChainManager *AudioEffectChainManager::GetInstance() {
            static AudioEffectChainManager audioEffectChainManager;
            return &audioEffectChainManager;
        }

        int32_t AudioEffectChainManager::ProcessEffectChain(char *streamType, void *bufIn, void *bufOut) {
            return SUCCESS;
        }

    }
}