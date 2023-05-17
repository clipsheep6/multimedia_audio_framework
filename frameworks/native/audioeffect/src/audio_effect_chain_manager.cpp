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

int32_t EffectChainManagerProcess(void *bufferAttrVoid, char *sceneType)
{
    BufferAttr *bufferAttr = (BufferAttr *)bufferAttrVoid;
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    std::string sceneTypeString(sceneType);
    if (!audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, bufferAttr->bufIn, bufferAttr->bufOut)) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t EffectChainManagerGetFrameLen()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    return audioEffectChainManager->GetFrameLen();
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
    AUDIO_INFO_LOG("apply effect chain beginning");
    for (EffectHandleT *handle: standByEffectHandles) {
        AUDIO_INFO_LOG("run effect: %{public}p", handle);
//                handle(bufIn, bufOut));
    }
}

int32_t FindEffectLib(std::string effect, std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList, LibEntryT *libEntry) {
    for (const std::unique_ptr <LibEntryT> &lib : effectLibraryList) {
        for (std::unique_ptr <EffectDescriptorT> &e: lib->effects) {
            if (strcmp(e->name, effect.c_str())) {
                libEntry = lib.get();
                return SUCCESS;
            }
        }
    }
    return ERROR;
}

AudioEffectChainManager::AudioEffectChainManager() {}

AudioEffectChainManager::~AudioEffectChainManager() {}

AudioEffectChainManager *AudioEffectChainManager::GetInstance() {
    static AudioEffectChainManager audioEffectChainManager;
    return &audioEffectChainManager;
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

// Boot initialize
void AudioEffectChainManager::InitAudioEffectChain(std::vector<EffectChain> effectChains,
                                                    std::vector <std::unique_ptr<LibEntryT>> &effectLibraryList) {

    std::set<std::string> effectSet;
    for (EffectChain efc: effectChains){
        for(std::string effect: efc.apply){
            effectSet.insert(effect);
        }
    }

    // Construct EffectToLibraryEntryMap that stores libEntry for each effect name
    LibEntryT libEntry;
    int32_t errorCode;
    for (std::string effect: effectSet) {
        errorCode = FindEffectLib(effect, effectLibraryList, &libEntry);
        if (errorCode == ERROR) {
            AUDIO_INFO_LOG("Couldn't find libEntry of effect %{public}s", effect.c_str());
            continue;
        }
        EffectToLibraryEntryMap[effect] = &libEntry;
    }
    
    // Construct EffectChainToEffectsMap that stores all effect names of each effect chain
    for (EffectChain efc: effectChains) {
        std::string key = efc.name;
        std::vector <std::string> effects;
        for (std::string effectName: efc.apply) {
            effects.emplace_back(effectName);
        }
        EffectChainToEffectsMap[key] = effects;
    }

    // Construct SceneTypeToEffectChainMap that stores lib handles of current effect chain of each scene type
    SetAudioEffectChain("SCENE_MUSIC", "EFFECT_DEFAULT");
    SetAudioEffectChain("SCENE_MOVIE", "EFFECT_DEFAULT");
    SetAudioEffectChain("SCENE_GAME", "EFFECT_DEFAULT");
    SetAudioEffectChain("SCENE_SPEECH", "EFFECT_DEFAULT");
    SetAudioEffectChain("SCENE_RING", "EFFECT_DEFAULT");
    SetAudioEffectChain("SCENE_OTHERS", "EFFECT_DEFAULT");
    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}d", EffectToLibraryEntryMap.size());
    AUDIO_INFO_LOG("EffectChainToEffectsMap size %{public}d", EffectChainToEffectsMap.size());
    AUDIO_INFO_LOG("SceneTypeToEffectChainMap size %{public}d", SceneTypeToEffectChainMap.size());
}

int32_t AudioEffectChainManager::SetAudioEffectChain(std::string sceneType, std::string effectChain) {
    if (!EffectChainToEffectsMap.count(effectChain)) {
        effectChain = "EFFECT_NONE";
        AUDIO_INFO_LOG("EffectChain's name [%{public}s] does not exist, auto set to EFFECT_NONE", effectChain.c_str());
    }

    AudioEffectChain *audioEffectChain;
    if (SceneTypeToEffectChainMap.count(sceneType)) {
        audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    } else {
        audioEffectChain = new AudioEffectChain(sceneType);
        SceneTypeToEffectChainMap[sceneType] = audioEffectChain;
    }

    std::vector<EffectHandleT *> effectHandles;
    for (std::string effect: EffectChainToEffectsMap[effectChain]) {
        EffectHandleT handle;
        EffectToLibraryEntryMap[effect]->desc->CreateEffect(&effect, 0, 0, &handle);
        effectHandles.emplace_back(&handle);
    }

    audioEffectChain->SetEffectChain(effectHandles);
    return SUCCESS;
}

// Every play 
int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string sceneType, void *bufIn, void *bufOut) {
    // if (!SceneTypeToEffectChainMap.count(sceneType)) {
    //     AUDIO_INFO_LOG("Scene type [%{public}s] does not exist", sceneType.c_str());
    //     return ERROR;
    // }

    float *bufferIn = (float *)bufIn;
    float *bufferOut = (float *)bufOut;
    if (sceneType == "SCENE_MUSIC") {
        for (int i = 0; i < frameLen * 2; i++) {
            bufferOut[i] = bufferIn[i] * 3;
        }
    }
    else {
        for (int i = 0; i < frameLen * 2; i++) {
            bufferOut[i] = bufferIn[i] / 3;
        }
    }
    // AUDIO_INFO_LOG("xjl: ApplyAudioEffectChain running %{public}s", sceneType.c_str());
    // auto *audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    // audioEffectChain->ApplyEffectChain(bufIn, bufOut);
    return SUCCESS;
}

}
}