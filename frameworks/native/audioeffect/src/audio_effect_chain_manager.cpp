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
#include "audio_effect.h"

using namespace OHOS::AudioStandard;

int32_t EffectChainManagerProcess(void *ioBufferConfigAdapter, char *sceneType)
{
    AudioEffectConfig *ioBufferConfig = static_cast<AudioEffectConfig *>(ioBufferConfigAdapter);
    AudioBuffer *bufIn = &(ioBufferConfig->inputCfg.buffer);
    AudioBuffer *bufOut = &(ioBufferConfig->outputCfg.buffer);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    std::string sceneTypeString(sceneType);
    if (!audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, bufIn, bufOut)) {
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

void AudioEffectChain::SetEffectChain(std::vector<AudioEffectHandle *> effectHandles) {
    standByEffectHandles.clear();
    for (AudioEffectHandle *handleT: effectHandles) {
        standByEffectHandles.emplace_back(handleT);
    }
}

void AudioEffectChain::ApplyEffectChain(AudioBuffer *bufIn, AudioBuffer *bufOut) {
    // for (AudioEffectHandle *handle: standByEffectHandles) {
    //     *handle->process(handle, bufIn, bufOut);
    // }
}

int32_t FindEffectLib(std::string effect, std::vector<std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList, 
    AudioEffectLibEntry *libEntry, std::string &libName) {
    for (const std::unique_ptr<AudioEffectLibEntry> &lib : effectLibraryList) {
        for (auto e: lib->effectName) {
            if (e == effect) {
                libName = lib->libraryName;
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
    std::vector <std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList) {

    std::set<std::string> effectSet;
    for (EffectChain efc: effectChains){
        for(std::string effect: efc.apply){
            effectSet.insert(effect);
        }
    }

    // Construct EffectToLibraryEntryMap that stores libEntry for each effect name
    AudioEffectLibEntry libEntry;
    std::string libName;
    int32_t errorCode;
    for (std::string effect: effectSet) {
        errorCode = FindEffectLib(effect, effectLibraryList, &libEntry, libName);
        if (errorCode == ERROR) {
            AUDIO_INFO_LOG("Couldn't find libEntry of effect %{public}s", effect.c_str());
            continue;
        }
        EffectToLibraryEntryMap[effect] = &libEntry;
        EffectToLibraryName[effect] = libName;
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
    std::string effectChain = "EFFECT_DEFAULT";
    auto sceneMode = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT);
    if (sceneMode != AUDIO_SUPPORTED_SCENE_MODES.end()) {
        effectChain = sceneMode->second;
    }
    for (auto sceneType = AUDIO_SUPPORTED_SCENE_TYPES.begin(); sceneType != AUDIO_SUPPORTED_SCENE_TYPES.end(); ++sceneType) {
        SetAudioEffectChain(sceneType->second, effectChain);
    }
    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}d", EffectToLibraryEntryMap.size());
    AUDIO_INFO_LOG("EffectChainToEffectsMap size %{public}d", EffectChainToEffectsMap.size());
    AUDIO_INFO_LOG("SceneTypeToEffectChainMap size %{public}d", SceneTypeToEffectChainMap.size());
}

int32_t AudioEffectChainManager::SetAudioEffectChain(std::string sceneType, std::string effectChain) {
    if (!EffectChainToEffectsMap.count(effectChain)) {
        AUDIO_ERR_LOG("EffectChain's name [%{public}s] does not exist, auto set to EFFECT_NONE", effectChain.c_str());
        effectChain = "EFFECT_NONE";
    }

    AudioEffectChain *audioEffectChain;
    if (SceneTypeToEffectChainMap.count(sceneType)) {
        audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    } else {
        audioEffectChain = new AudioEffectChain(sceneType);
        SceneTypeToEffectChainMap[sceneType] = audioEffectChain;
    }

    std::vector<AudioEffectHandle *> effectHandles;
    for (std::string effect: EffectChainToEffectsMap[effectChain]) {
        AudioEffectHandle handle;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = EffectToLibraryName[effect];
        descriptor.effectName = effect;
        EffectToLibraryEntryMap[effect]->audioEffectLibHandle->createEffect(descriptor, &handle);
        effectHandles.emplace_back(&handle);
    }

    audioEffectChain->SetEffectChain(effectHandles);
    return SUCCESS;
}

// Every play 
int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string sceneType, AudioBuffer *bufIn, AudioBuffer *bufOut) {
    // if (!SceneTypeToEffectChainMap.count(sceneType)) {
    //     AUDIO_INFO_LOG("Scene type [%{public}s] does not exist", sceneType.c_str());
    //     return ERROR;
    // }    
    
    // auto *audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    // audioEffectChain->ApplyEffectChain(bufIn, bufOut);

    float *bufferIn = (float *)bufIn->f32;
    float *bufferOut = (float *)bufOut->f32;
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
    
    return SUCCESS;
}

}
}