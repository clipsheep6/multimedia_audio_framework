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
        return ERROR;
    }
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
    effectMode = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT)->second;
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

void AudioEffectChain::Dump()
{
    for (AudioEffectHandle handle: standByEffectHandles) {
        AUDIO_INFO_LOG("Dump standByEffectHandle for [%{public}s], handle address is %{public}p", sceneType.c_str(), handle);
    }
}

void AudioEffectChain::SetEffectMode(std::string mode) {
    effectMode = mode;
}

void AudioEffectChain::AddEffectHandleBegin() {
    standByEffectHandles.clear();
}

template <typename T>
int32_t GetKeyFromValue(const std::unordered_map<T, std::string> &map, std::string &value)
{
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (it->second == value) {
            return it->first;
        }
    }
    return -1;
}

void AudioEffectChain::AddEffectHandle(AudioEffectHandle handle) {
    int32_t ret;
    int32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    ret = (*handle)->command(handle, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_INIT fail");
        return;
    }
    ret = (*handle)->command(handle, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_ENABLE fail");
        return;
    }
    ret = (*handle)->command(handle, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_CONFIG fail");
        return;
    }
    // Set param
    AudioEffectParam *effectParam = new AudioEffectParam[sizeof(AudioEffectParam) + NUM_SET_EFFECT_PARAM * sizeof(int32_t)];
    effectParam->status = 0;
    effectParam->paramSize = sizeof(int32_t);
    effectParam->valueSize = 0;
    effectParam->data[0] = EFFECT_SET_PARAM;
    effectParam->data[1] = GetKeyFromValue(AUDIO_SUPPORTED_SCENE_TYPES, sceneType);
    effectParam->data[2] = GetKeyFromValue(AUDIO_SUPPORTED_SCENE_MODES, effectMode);
    AUDIO_INFO_LOG("cjw: data[0]: %{public}d, data[1]: %{public}d, data[2]: %{public}d", effectParam->data[0],  effectParam->data[1],  effectParam->data[2]);
    cmdInfo = {sizeof(AudioEffectParam) + sizeof(int32_t) * NUM_SET_EFFECT_PARAM, &effectParam};
    ret = (*handle)->command(handle, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
    delete[] effectParam;
    if (ret != 0) {
        AUDIO_ERR_LOG("EFFECT_CMD_SET_PARAM fail");
        return;
    }
    standByEffectHandles.emplace_back(handle);
}

void AudioEffectChain::AddEffectHandleEnd() {
    // used for crossfading in the near future
}

void AudioEffectChain::ApplyEffectChain(float *bufIn, float *bufOut, uint32_t frameLen) {    
    if (standByEffectHandles.empty()) {
        AUDIO_INFO_LOG("Effectchain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
        for (uint32_t i = 0; i < frameLen * ioBufferConfig.outputCfg.channels; ++i) {
            bufOut[i] = bufIn[i];
        }
        return;
    }

    audioBufIn.frameLength = frameLen;
    audioBufOut.frameLength = frameLen;
    int count = 0;
    for (AudioEffectHandle handle: standByEffectHandles) {
        if (count % 2 == 0) {
            audioBufIn.raw = bufIn;
            audioBufOut.raw = bufOut;
        } else {
            audioBufOut.raw = bufIn;
            audioBufIn.raw = bufOut;
        }        
        (*handle)->process(handle, &audioBufIn, &audioBufOut);
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

int32_t FindEffectLib(std::string effect, std::vector<std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList, 
    AudioEffectLibEntry **libEntry, std::string &libName) {
    for (const std::unique_ptr<AudioEffectLibEntry> &lib : effectLibraryList) {
        if (lib->libraryName == effect) {
            libName = lib->libraryName;
            *libEntry = lib.get();
            return SUCCESS;
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
void AudioEffectChainManager::InitAudioEffectChainManager(std::vector<EffectChain> &effectChains,
    std::unordered_map<std::string, std::string> &map,
    std::vector<std::unique_ptr<AudioEffectLibEntry>> &effectLibraryList) {
    
    std::set<std::string> effectSet;
    for (EffectChain efc: effectChains){
        for(std::string effect: efc.apply){
            effectSet.insert(effect);
        }
    }

    // Construct EffectToLibraryEntryMap that stores libEntry for each effect name
    AudioEffectLibEntry *libEntry = nullptr;
    int32_t errorCode;
    std::string libName;
    for (std::string effect: effectSet) {
        errorCode = FindEffectLib(effect, effectLibraryList, &libEntry, libName);
        if (errorCode == ERROR) {
            AUDIO_INFO_LOG("Couldn't find libEntry of effect %{public}s", effect.c_str());
            continue;
        }
        EffectToLibraryEntryMap[effect] = libEntry;        
        EffectToLibraryNameMap[effect] = libName;        
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

    // Constrcut SceneTypeAndModeToEffectChainNameMap that stores effectMode associated with the effectChainName
    for (auto item = map.begin(); item != map.end(); ++item) {
        SceneTypeAndModeToEffectChainNameMap[item->first] = item->second;
    }

    AUDIO_INFO_LOG("EffectToLibraryEntryMap size %{public}d", (int32_t)EffectToLibraryEntryMap.size());
    AUDIO_INFO_LOG("EffectChainToEffectsMap size %{public}d", (int32_t)EffectChainToEffectsMap.size());
    AUDIO_INFO_LOG("SceneTypeAndModeToEffectChainNameMap size %{public}d", (int32_t)SceneTypeAndModeToEffectChainNameMap.size());
}

int32_t AudioEffectChainManager::CreateAudioEffectChain(std::string sceneType, BufferAttr *bufferAttr)
{
    AudioEffectChain *audioEffectChain;
    if (SceneTypeToEffectChainMap.count(sceneType)) {
        audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    } else {
        audioEffectChain = new AudioEffectChain(sceneType);
        SceneTypeToEffectChainMap[sceneType] = audioEffectChain;
    }

    audioEffectChain->SetIOBufferConfig(true, bufferAttr->samplingRate, bufferAttr->numChanIn); // input buffer config
    audioEffectChain->SetIOBufferConfig(false, bufferAttr->samplingRate, bufferAttr->numChanOut); // output buffer config

    std::string effectMode = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_DEFAULT)->second;
    if (SetAudioEffectChain(sceneType, effectMode) != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEffectChainManager::SetAudioEffectChain(std::string sceneType, std::string effectMode)
{    
    if (!SceneTypeToEffectChainMap.count(sceneType)) {
        AUDIO_ERR_LOG("SceneType [%{public}s] does not exist, failed to set", sceneType.c_str());
        return ERROR;
    }
    AudioEffectChain *audioEffectChain = SceneTypeToEffectChainMap[sceneType];

    std::string effectChain;
    std::string effectChainKey = sceneType + "_&_" + effectMode;
    std::string effectNone = AUDIO_SUPPORTED_SCENE_MODES.find(EFFECT_NONE)->second;
    if (!SceneTypeAndModeToEffectChainNameMap.count(effectChainKey)) {
        AUDIO_ERR_LOG("EffectChain key [%{public}s] does not exist, auto set to %{public}s", effectChainKey.c_str(), effectNone.c_str());
        effectChain = effectNone;
    } else {
        effectChain = SceneTypeAndModeToEffectChainNameMap[effectChainKey];
    }

    if (effectChain != effectNone && !EffectChainToEffectsMap.count(effectChain)) {
        AUDIO_ERR_LOG("EffectChain name [%{public}s] does not exist, auto set to %{public}s", effectChain.c_str(), effectNone.c_str());
        effectChain = effectNone;
    }

    audioEffectChain->SetEffectMode(effectMode);
    audioEffectChain->AddEffectHandleBegin();
    for (std::string effect: EffectChainToEffectsMap[effectChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = EffectToLibraryNameMap[effect];
        descriptor.effectName = effect;
        if (!EffectToLibraryEntryMap[effect]) {
            AUDIO_ERR_LOG("EffectToLibraryEntryMap[%{public}s] is nullptr", effect.c_str());
            continue;
        }
        if (!EffectToLibraryEntryMap[effect]->audioEffectLibHandle) {
            AUDIO_ERR_LOG("EffectToLibraryEntryMap[%{public}s]->audioEffectLibHandle is nullptr", effect.c_str());
            continue;               
        }
        EffectToLibraryEntryMap[effect]->audioEffectLibHandle->createEffect(descriptor, &handle);
        audioEffectChain->AddEffectHandle(handle);
    }
    audioEffectChain->AddEffectHandleEnd();
    
    return SUCCESS;
}

int32_t AudioEffectChainManager::ApplyAudioEffectChain(std::string sceneType, BufferAttr *bufferAttr) {
    if (!SceneTypeToEffectChainMap.count(sceneType)) {
        AUDIO_INFO_LOG("Scene type [%{public}s] does not exist", sceneType.c_str());
        return ERROR;
    }

    auto *audioEffectChain = SceneTypeToEffectChainMap[sceneType];
    audioEffectChain->ApplyEffectChain(bufferAttr->bufIn, bufferAttr->bufOut, bufferAttr->frameLen);
    
    return SUCCESS;
}

void AudioEffectChainManager::Dump()
{
    AUDIO_INFO_LOG("<-------- AudioEffectChainManager::Dump START -------->");
    for (auto item = SceneTypeToEffectChainMap.begin(); item != SceneTypeToEffectChainMap.end(); ++item) {
        AudioEffectChain *audioEffectChain = item->second;
        audioEffectChain->Dump();
    }
    AUDIO_INFO_LOG("<-------- AudioEffectChainManager::Dump END -------->");
}

}
}
