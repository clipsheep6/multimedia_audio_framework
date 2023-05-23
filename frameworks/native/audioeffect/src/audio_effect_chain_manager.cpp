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
#include "audio_effect.h"

using namespace OHOS::AudioStandard;

int32_t EffectChainManagerCreate(char *sceneType, BufferAttr *bufferAttr)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString(sceneType);
    if (audioEffectChainManager->CreateAudioEffectChain(sceneTypeString, bufferAttr) != SUCCESS) {
        AUDIO_INFO_LOG("cjw: CreateAudioEffectChain fail");
        audioEffectChainManager->Dump();
        return ERROR;
    }
    AUDIO_INFO_LOG("cjw: CreateAudioEffectChain SUCCESS");
    audioEffectChainManager->Dump();
    return SUCCESS;
}

int32_t EffectChainManagerProcess(char *sceneType, BufferAttr *bufferAttr)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneTypeString(sceneType);
    if (audioEffectChainManager->ApplyAudioEffectChain(sceneTypeString, bufferAttr) != SUCCESS) {
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
    audioBufIn.frameLength = 0;
    audioBufOut.frameLength = 0;
    ioBufferConfig.inputCfg.samplingRate = 48000;
    ioBufferConfig.inputCfg.channels = 2;
    ioBufferConfig.inputCfg.format = DATA_FORMAT_F32;
    ioBufferConfig.outputCfg.samplingRate = 48000;
    ioBufferConfig.outputCfg.channels = 2;
    ioBufferConfig.outputCfg.format = DATA_FORMAT_F32;
}

AudioEffectChain::~AudioEffectChain() {}

// void AudioEffectChain::SetEffectChain(std::vector<AudioEffectHandle *> &effectHandles) {
//     if (effectHandles.empty()) {
//         AUDIO_INFO_LOG("cjw: effectHandles size is zero during SetEffectChain");
//     }
//     standByEffectHandles.clear();
//     int32_t ret;
//     int32_t replyData = 0;
//     AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
//     AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
//     AudioEffectHandle handle;
//     for (AudioEffectHandle *effectHandle: effectHandles) {
//         handle = *effectHandle;
//         ret = (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
//         if (ret != 0) {
//             AUDIO_ERR_LOG("EFFECT_CMD_INIT fail");
//             // continue;
//         }
//         ret = (*handle)->command(handle, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
//         if (ret != 0) {
//             AUDIO_ERR_LOG("EFFECT_CMD_ENABLE fail");
//             // continue;
//         }
//         ret = (*handle)->command(handle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
//         if (ret != 0) {
//             AUDIO_ERR_LOG("EFFECT_CMD_SET_CONFIG fail");
//             // continue;
//         }
//         standByEffectHandles.emplace_back(effectHandle);
//     }

//     for (AudioEffectHandle *effectHandle: effectHandles) {
//         handle = *effectHandle;
//         AUDIO_INFO_LOG("cjw: SetEffectChain [%{public}s] effectHandle %{public}p", sceneType.c_str(), handle);
//     }
//     for (AudioEffectHandle *effectHandle: standByEffectHandles) {
//         handle = *effectHandle;
//         AUDIO_INFO_LOG("cjw: SetEffectChain [%{public}s] standByEffectHandle %{public}p", sceneType.c_str(), handle);
//     }
// }

void AudioEffectChain::AddEffectHandleBegin() {
    standByEffectHandles.clear();
}

void AudioEffectChain::AddEffectHandle(AudioEffectHandle handle) {
    
    // standByEffectHandles.clear();
    int32_t ret;
    int32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    // AudioEffectHandle handle = *effectHandle;
    // for (AudioEffectHandle *effectHandle: effectHandles) {
    ret = (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_INIT fail");
        // return;
    }
    ret = (*handle)->command(handle, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_ENABLE fail");
        // return;
    }
    ret = (*handle)->command(handle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_CONFIG fail");
        // return;
    }
    standByEffectHandles.emplace_back(handle);
    AUDIO_INFO_LOG("cjw: AddEffectHandle [%{public}s] effectHandle %{public}p", sceneType.c_str(), handle);   
}

void AudioEffectChain::AddEffectHandleEnd() {
    // standByEffectHandles.clear();
}

void AudioEffectChain::Dump()
{
    for (AudioEffectHandle handle: standByEffectHandles) {
        AUDIO_INFO_LOG("cjw: Dump [%{public}s] standByEffectHandle %{public}p", sceneType.c_str(), handle);
    }
}

void AudioEffectChain::ApplyEffectChain(float *bufIn, float *bufOut, uint32_t frameLen) {
    Dump();
    if (standByEffectHandles.empty()) {
        AUDIO_INFO_LOG("cjw: standByEffectHandles size is zero during ApplyEffectChain, copy bufIn to bufOut");
        for (uint32_t i = 0; i < frameLen * ioBufferConfig.outputCfg.channels; ++i) {
            bufOut[i] = bufIn[i];
        }
        return;
    }
    AUDIO_INFO_LOG("cjw: standByEffectHandles size is %{public}d during ApplyEffectChain", standByEffectHandles.size());

    audioBufIn.frameLength = frameLen;
    audioBufOut.frameLength = frameLen;
    int32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};    
    int count = 0;
    Dump();
    for (AudioEffectHandle handle: standByEffectHandles) {
        if (count % 2 == 0) {
            audioBufIn.raw = bufIn;
            audioBufOut.raw = bufOut;
        } else {
            audioBufOut.raw = bufIn;
            audioBufIn.raw = bufOut;
        }        
        AUDIO_INFO_LOG("cjw: suanfa in");
        AUDIO_INFO_LOG("cjw: ApplyEffectChain [%{public}s] standByEffectHandle %{public}p", sceneType.c_str(), handle);
        // (*handle)->process(handle, &audioBufIn, &audioBufOut);
        (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
        AUDIO_INFO_LOG("cjw: suanfa out");
    }
    bufOut = audioBufOut.f32;
}

void AudioEffectChain::SetIOBufferConfig(bool isInput, uint32_t samplingRate, uint32_t channels)
{
    if (isInput) {
        ioBufferConfig.inputCfg.samplingRate = samplingRate;
        ioBufferConfig.inputCfg.channels = channels;
    } else {
        ioBufferConfig.outputCfg.samplingRate = samplingRate;
        ioBufferConfig.outputCfg.channels = channels;
    }
}

// int32_t FindEffectLib(std::string effect, std::vector<std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList, 
//     AudioEffectLibEntry *libEntry, std::string &libName) {
//     for (const std::unique_ptr<AudioEffectLibEntry> &lib : effectLibraryList) {
//         for (auto e: lib->effectName) {
//             if (e == effect) {
//                 libName = lib->libraryName;
//                 libEntry = lib.get();
//                 return SUCCESS;
//             }
//         }
//     }
//     return ERROR;
// }

AudioEffectLibEntry *FindLibrary(const std::string name, std::vector<std::unique_ptr<AudioEffectLibEntry>> &libList, std::string &libName)
{
    for (const std::unique_ptr<AudioEffectLibEntry>& lib : libList) {
        if (lib->libraryName == name) {
            libName = lib->libraryName;
            return lib.get();
        }
    }
    return nullptr;
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
void AudioEffectChainManager::InitAudioEffectChainManager(std::vector<EffectChain> &effectChains,
    std::unordered_map<std::string, std::string> &map,
    std::vector<std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList) {

    for (const std::unique_ptr<AudioEffectLibEntry>& lib : effectLibraryList) {
        AUDIO_INFO_LOG("cjw: InitAEC libname %{public}s, address is %{public}p", lib->libraryName.c_str(), lib->audioEffectLibHandle);
    }

    std::set<std::string> effectSet;
    for (EffectChain efc: effectChains){
        for(std::string effect: efc.apply){
            effectSet.insert(effect);
        }
    }

    // Construct EffectToLibraryEntryMap that stores libEntry for each effect name
    // AudioEffectLibEntry *libEntry = nullptr;
    std::string libName;
    // int32_t errorCode;
    for (std::string effect: effectSet) {
        // errorCode = FindEffectLib(effect, effectLibraryList, libEntry, libName);
        AudioEffectLibEntry *libEntry = FindLibrary(effect, effectLibraryList, libName);
        // if (errorCode == ERROR) {
        if (!libEntry) {
            AUDIO_INFO_LOG("Couldn't find libEntry of effect %{public}s", effect.c_str());
            continue;
        }
        EffectToLibraryEntryMap[effect] = libEntry;        
        EffectToLibraryNameMap[effect] = libName;
        AUDIO_INFO_LOG("cjw: FindEffectLib libname %{public}s, address is %{public}p", libEntry->libraryName.c_str(), libEntry->audioEffectLibHandle);
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

    for (auto item = map.begin(); item != map.end(); ++item) {
        SceneTypeAndModeToEffectChainNameMap[item->first] = item->second;
    }

    for (auto item = SceneTypeAndModeToEffectChainNameMap.begin(); item != SceneTypeAndModeToEffectChainNameMap.end(); ++item) {
        AUDIO_INFO_LOG("cjw: SceneTypeAndModeToEffectChainNameMap.key: %{public}s, SceneTypeAndModeToEffectChainNameMap.val: %{public}s", 
            item->first.c_str(), item->second.c_str());
    }

    for (auto item = EffectToLibraryEntryMap.begin(); item != EffectToLibraryEntryMap.end(); ++item) {
        AudioEffectLibEntry *libEntry = item->second;
        AUDIO_INFO_LOG("cjw: EffectToLibraryEntryMap: libname %{public}s, address is %{public}p", libEntry->libraryName.c_str(), libEntry->audioEffectLibHandle);
    }            


    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}d", EffectToLibraryEntryMap.size());
    AUDIO_INFO_LOG("EffectChainToEffectsMap size %{public}d", EffectChainToEffectsMap.size());
    AUDIO_INFO_LOG("SceneTypeAndModeToEffectChainNameMap size %{public}d", SceneTypeAndModeToEffectChainNameMap.size());
}

int32_t AudioEffectChainManager::CreateAudioEffectChain(std::string sceneType, BufferAttr *bufferAttr)
{
    AUDIO_INFO_LOG("cjw: CreateAudioEffectChain %{public}s, 1", sceneType.c_str());
    AudioEffectChain *audioEffectChain;
    if (SceneTypeToEffectChainMap.count(sceneType)) {
        audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    } else {
        audioEffectChain = new AudioEffectChain(sceneType);
        SceneTypeToEffectChainMap[sceneType] = audioEffectChain;
    }

    AUDIO_INFO_LOG("cjw: CreateAudioEffectChain %{public}s, 2", sceneType.c_str());
    audioEffectChain->SetIOBufferConfig(true, bufferAttr->samplingRate, bufferAttr->numChanIn);
    audioEffectChain->SetIOBufferConfig(false, bufferAttr->samplingRate, bufferAttr->numChanOut);

    std::string effectMode = "EFFECT_DEFAULT";
    auto sceneMode = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT);
    if (sceneMode != AUDIO_SUPPORTED_SCENE_MODES.end()) {
        effectMode = sceneMode->second;
    }

    AUDIO_INFO_LOG("cjw: CreateAudioEffectChain %{public}s, 3", sceneType.c_str());
    if (SetAudioEffectChain(sceneType, effectMode) != SUCCESS) {
        AUDIO_INFO_LOG("cjw: CreateAudioEffectChain %{public}s, 3.5 error", sceneType.c_str());
        return ERROR;
    }
    Dump();
    AUDIO_INFO_LOG("cjw: CreateAudioEffectChain %{public}s, 4", sceneType.c_str());
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetAudioEffectChain(std::string sceneType, std::string effectMode)
{
    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 1", sceneType.c_str());
    if (!SceneTypeToEffectChainMap.count(sceneType)) {
        AUDIO_ERR_LOG("SceneType [%{public}s] does not exist, failed to set", sceneType.c_str());
        return ERROR;
    }
    AudioEffectChain *audioEffectChain = SceneTypeToEffectChainMap[sceneType];

    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 2", sceneType.c_str());
    std::string effectChain;
    std::string effectChainKey = sceneType + "_&_" + effectMode;
    if (!SceneTypeAndModeToEffectChainNameMap.count(effectChainKey)) {
        AUDIO_ERR_LOG("EffectChain key [%{public}s] does not exist, auto set to EFFECT_NONE", effectChainKey.c_str());
        effectChain = "EFFECT_NONE";
    } else {
        effectChain = SceneTypeAndModeToEffectChainNameMap[effectChainKey];
    }

    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 3", sceneType.c_str());
    if (!EffectChainToEffectsMap.count(effectChain)) {
        AUDIO_ERR_LOG("EffectChain name [%{public}s] does not exist, auto set to EFFECT_NONE", effectChain.c_str());
        effectChain = "EFFECT_NONE";
    }

    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 4", sceneType.c_str());
    // std::vector<AudioEffectHandle *> effectHandles;
    audioEffectChain->AddEffectHandleBegin();
    for (std::string effect: EffectChainToEffectsMap[effectChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = EffectToLibraryNameMap[effect];
        descriptor.effectName = effect;
        AudioEffectLibEntry *libEntry = EffectToLibraryEntryMap[effect];
        AUDIO_INFO_LOG("cjw: SetAudioEffectChain: libname %{public}s, address is %{public}p", libEntry->libraryName.c_str(), libEntry->audioEffectLibHandle);
        if (EffectToLibraryEntryMap[effect]->audioEffectLibHandle == nullptr) {
            AUDIO_INFO_LOG("EffectToLibraryEntryMap[%{public}s]->audioEffectLibHandle is nullptr", effect.c_str());
        } else {            
            EffectToLibraryEntryMap[effect]->audioEffectLibHandle->createEffect(descriptor, &handle);
        }
        // effectHandles.emplace_back(&handle);
        audioEffectChain->AddEffectHandle(handle);
    }
    audioEffectChain->AddEffectHandleEnd();

    // AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, effectHandles size is %{public}d, 4.5", sceneType.c_str(), effectHandles.size());
    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 5", sceneType.c_str());
    // audioEffectChain->SetEffectChain(effectHandles);
    audioEffectChain->Dump();
    Dump();
    AUDIO_INFO_LOG("cjw: SetAudioEffectChain %{public}s, 6", sceneType.c_str());
    return SUCCESS;
}

int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string sceneType, BufferAttr *bufferAttr) {
    if (!SceneTypeToEffectChainMap.count(sceneType)) {
        AUDIO_INFO_LOG("Scene type [%{public}s] does not exist", sceneType.c_str());
        return ERROR;
    }

    auto *audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    audioEffectChain->ApplyEffectChain(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->frameLen);

    float *bufferIn = bufferAttr->bufIn;
    float *bufferOut = bufferAttr->bufOut;
    int32_t frameLen = bufferAttr->frameLen;
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

void AudioEffectChainManager::Dump()
{
    AUDIO_INFO_LOG("--------AudioEffectChainManager::BigDump START--------");
    for (auto item = SceneTypeToEffectChainMap.begin(); item != SceneTypeToEffectChainMap.end(); ++item) {
        AudioEffectChain *audioEffectChain = item->second;
        audioEffectChain->Dump();
    }
    AUDIO_INFO_LOG("--------AudioEffectChainManager::BigDump END--------");
}

}
}