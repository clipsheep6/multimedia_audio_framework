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

int32_t EffectChainManagerProcess(struct EffectChainAdapter *adapter, char *sceneType) {
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null EffectChainAdapter");
    AudioEffectChainManager *audioEffectChainManager = static_cast<AudioEffectChainManager *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString(sceneType);
    if (!audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, adapter->bufIn, adapter->bufOut)) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t EffectChainManagerGetFrameLen(struct EffectChainAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "null EffectChainAdapter");
    AudioEffectChainManager *audioEffectChainManager = static_cast<AudioEffectChainManager *>(adapter->wapper);
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    return audioEffectChainManager->GetFrameLen();
}

int32_t EffectChainManagerReturnValue(struct EffectChainAdapter *adapter, int32_t i)
{
    AUDIO_INFO_LOG("xjl: AdapterReturnValue start, value=%{public}d", i);
    AudioEffectChainManager *audioEffectChainManager = static_cast<AudioEffectChainManager *>(adapter->wapper);
    int j = audioEffectChainManager->ReturnValue(i);
    AUDIO_INFO_LOG("xjl: AdapterReturnValue end, value=%{public}d", i);
    return j;
}

namespace OHOS {
    namespace AudioStandard {
        AudioEffectChain::AudioEffectChain(std::string scene) {
            sceneType = scene;
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

        int32_t AudioEffectChainManager::ReturnValue(int32_t i)
        {
            AUDIO_INFO_LOG("xyq: come into AudioEffectChainManager::ReturnValue, value=%{public}d", i);
            return i;
        }

        int32_t AudioEffectChainManager::SetFrameLen(int32_t frameLength)
        {
            frameLen = frameLength;
            return SUCCESS;
        }

        int32_t AudioEffectChainManager::GetFrameLen()
        {
            return frameLen;
        }

        void AudioEffectChainManager::InitAudioEffectChain(std::vector<EffectChain> effectChains,
                                                           std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList) {

            AUDIO_INFO_LOG("xjl: init audio effect chain in AudioEffectChainManager step1");
            std::set<std::string> effectSet;
            for (EffectChain efc: effectChains){
                for(std::string effect: efc.apply){
                    effectSet.insert(effect);
                }
            }

            AUDIO_INFO_LOG("xjl: init audio effect chain in AudioEffectChainManager step2");
            // make EffectToLibraryEntryMap
            for (std::string effect: effectSet) {
                auto *libEntry = findlibOfEffect(effect, effectLibraryList);
                if (!libEntry) {
//                    std::cout << "libEntry is nil while find effect:" << effect.name << std::endl;
                }

                EffectToLibraryEntryMap[effect] = libEntry;
            }

            AUDIO_INFO_LOG("xjl: init audio effect chain in AudioEffectChainManager step3");
            // make EffectChainToEffectsMap
            for (EffectChain efc: effectChains) {
                std::string key = efc.name;
                std::vector <std::string> effects;
                for (std::string effectName: efc.apply) {
                    effects.emplace_back(effectName);
                }
                EffectChainToEffectsMap[key] = effects;
            }
            AUDIO_INFO_LOG("xjl: init audio effect chain in AudioEffectChainManager step4");

            SetAudioEffectChain("SCENE_MUSIC", "default");
            SetAudioEffectChain("SCENE_MOVIE", "default");
            SetAudioEffectChain("SCENE_GAME", "default");
            SetAudioEffectChain("SCENE_SPEECH", "default");
            SetAudioEffectChain("SCENE_RING", "default");
            SetAudioEffectChain("SCENE_OTHERS", "default");

            AUDIO_INFO_LOG("xjl: EffectToLibraryEntryMap size %{public}d", EffectToLibraryEntryMap.size());
            AUDIO_INFO_LOG("xjl: EffectChainToEffectsMap size %{public}d", EffectChainToEffectsMap.size());
            AUDIO_INFO_LOG("xjl: SceneTypeToEffectChainMap size %{public}d", SceneTypeToEffectChainMap.size());
        }

        int32_t AudioEffectChainManager::SetAudioEffectChain(std::string sceneType, std::string effectChain) {
            AudioEffectChain *audioEffectChain = new AudioEffectChain(sceneType);
            SceneTypeToEffectChainMap[sceneType] = audioEffectChain;

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

        int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string sceneType, void *bufIn, void *bufOut) {
            float *bufferIn = (float *)bufIn;
            float *bufferOut = (float *)bufOut;
            if (sceneType == "SCENE_MUSIC") {
                for (int i = 0; i < frameLen * 2; i++) {
                    bufferOut[i] = bufferIn[i] * 3;
                }
            }
            else if (sceneType == "SCENE_MOVIE") {
                for (int i = 0; i < frameLen * 2; i++) {
                    bufferOut[i] = bufferIn[i] / 3;
                }
            }
            AUDIO_INFO_LOG("xjl: ApplyAudioEffectChain running %{public}s", sceneType.c_str());
            // auto *audioEffectChain = SceneTypeToEffectChainMap[sceneType];
            // audioEffectChain->ApplyEffectChain(bufIn, bufOut);
            return 0;
        }

        AudioEffectChainManager::AudioEffectChainManager() {}

        AudioEffectChainManager::~AudioEffectChainManager() {}

        AudioEffectChainManager *AudioEffectChainManager::GetInstance() {
            static AudioEffectChainManager audioEffectChainManager;
            return &audioEffectChainManager;
        }
    }
}