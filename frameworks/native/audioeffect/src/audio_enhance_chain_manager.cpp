/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChainManager"

#include "audio_enhance_chain_manager.h"

#include "securec.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_effect.h"
#include "audio_enhance_chain.h"
#include "audio_enhance_chain_adapter.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t SCENE_TYPE_MASK = 0x00FF0000;
constexpr uint32_t CAPTURER_ID_MASK = 0x0000FF00;
constexpr uint32_t RENDERER_ID_MASK = 0x000000FF;
constexpr uint32_t VOLUME_FACTOR = 100;

static int32_t FindEnhanceLib(const std::string &enhance,
    const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList,
    std::shared_ptr<AudioEffectLibEntry> &libEntry, std::string &libName)
{
    for (const std::shared_ptr<AudioEffectLibEntry> &lib : enhanceLibraryList) {
        for (const auto &effectName : lib->effectName) {
            if (effectName == enhance) {
                libName = lib->libraryName;
                libEntry = lib;
                return SUCCESS;
            }
        }
    }
    return ERROR;
}

static int32_t CheckValidEnhanceLibEntry(const std::shared_ptr<AudioEffectLibEntry> &libEntry,
    const std::string &enhance, const std::string &libName)
{
    CHECK_AND_RETURN_RET_LOG(libEntry, ERROR, "Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle, ERROR,
        "AudioEffectLibHandle of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->createEffect, ERROR,
        "CreateEffect function of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    CHECK_AND_RETURN_RET_LOG(libEntry->audioEffectLibHandle->releaseEffect, ERROR,
        "ReleaseEffect function of Enhance [%{public}s] in lib [%{public}s] is nullptr",
        enhance.c_str(), libName.c_str());
    return SUCCESS;
}

AudioEnhanceChainManager::AudioEnhanceChainManager()
{
    sceneTypeToEnhanceChainMap_.clear();
    sceneTypeToEnhanceChainCountMap_.clear();
    sceneTypeAndModeToEnhanceChainNameMap_.clear();
    enhanceChainToEnhancesMap_.clear();
    enhanceToLibraryEntryMap_.clear();
    enhanceToLibraryNameMap_.clear();
    captureIdToDeviceMap_.clear();
    renderIdToDeviceMap_.clear();
    enhanceBuffer_ = nullptr;
    isInitialized_ = false;
}

AudioEnhanceChainManager::~AudioEnhanceChainManager()
{
    AUDIO_INFO_LOG("~AudioEnhanceChainManager destroy");
}

AudioEnhanceChainManager *AudioEnhanceChainManager::GetInstance()
{
    static AudioEnhanceChainManager audioEnhanceChainManager;
    return &audioEnhanceChainManager;
}

void AudioEnhanceChainManager::ResetInfo()
{
    sceneTypeToEnhanceChainMap_.clear();
    sceneTypeToEnhanceChainCountMap_.clear();
    sceneTypeAndModeToEnhanceChainNameMap_.clear();
    enhanceChainToEnhancesMap_.clear();
    enhanceToLibraryEntryMap_.clear();
    enhanceToLibraryNameMap_.clear();
    enhancePropertyMap_.clear();
    captureIdToDeviceMap_.clear();
    renderIdToDeviceMap_.clear();
    FreeEnhanceBuffer();
    isInitialized_ = false;
    sessionId_ = 0;
    volumeType_ = STREAM_MUSIC;
    systemVol_ = 0.0f;
    streamVol_ = 0.0f;
    isMute_ = false;
}

void AudioEnhanceChainManager::ConstructEnhanceChainMgrMaps(std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    std::set<std::string> enhanceSet;
    for (EffectChain enhanceChain : enhanceChains) {
        for (std::string enhance : enhanceChain.apply) {
            enhanceSet.insert(enhance);
        }
    }
    // Construct enhanceToLibraryEntryMap_ that stores libEntry for each effect name
    std::shared_ptr<AudioEffectLibEntry> libEntry = nullptr;
    std::string libName;
    for (std::string enhance : enhanceSet) {
        int32_t ret = FindEnhanceLib(enhance, enhanceLibraryList, libEntry, libName);
        CHECK_AND_CONTINUE_LOG(ret != ERROR, "Couldn't find libEntry of effect %{public}s", enhance.c_str());
        ret = CheckValidEnhanceLibEntry(libEntry, enhance, libName);
        enhanceToLibraryEntryMap_[enhance] = libEntry;
        enhanceToLibraryNameMap_[enhance] = libName;
    }
    // Construct enhanceChainToEnhancesMap_ that stores all effect names of each effect chain
    for (EffectChain enhanceChain : enhanceChains) {
        std::string key = enhanceChain.name;
        std::vector<std::string> enhances;
        for (std::string enhanceName : enhanceChain.apply) {
            if (enhanceToLibraryEntryMap_.count(enhanceName)) {
                enhances.emplace_back(enhanceName);
            }
        }
        enhanceChainToEnhancesMap_[key] = enhances;
    }
    // Construct sceneTypeAndModeToEnhanceChainNameMap_ that stores effectMode associated with the effectChainName
    sceneTypeAndModeToEnhanceChainNameMap_ = managerParam.sceneTypeToChainNameMap;
    // Construct enhancePropertyMap_ that stores effect's property
    enhancePropertyMap_ = managerParam.effectDefaultProperty;
}

void AudioEnhanceChainManager::InitAudioEnhanceChainManager(std::vector<EffectChain> &enhanceChains,
    const EffectChainManagerParam &managerParam, std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    normalSceneLimit_ = managerParam.maxExtraNum;
    priorSceneSet_.insert(managerParam.priorSceneList.begin(), managerParam.priorSceneList.end());
    defaultScene_ = managerParam.defaultSceneName;
    AUDIO_INFO_LOG("defaultSceneName is %{public}s", defaultScene_.c_str());
    for (const auto &priorScene : priorSceneSet_) {
        AUDIO_INFO_LOG("prioScene contains %{public}s", priorScene.c_str());
    }
    AUDIO_INFO_LOG("normalSceneLimit is %{public}u", normalSceneLimit_);
    ConstructEnhanceChainMgrMaps(enhanceChains, managerParam, enhanceLibraryList);

    AUDIO_INFO_LOG("enhanceToLibraryEntryMap_ size %{public}zu \
        enhanceChainToEnhancesMap_ size %{public}zu \
        sceneTypeAndModeToEnhanceChainNameMap_ size %{public}zu",
        enhanceToLibraryEntryMap_.size(),
        enhanceChainToEnhancesMap_.size(),
        sceneTypeAndModeToEnhanceChainNameMap_.size());
    isInitialized_ = true;
}

int32_t AudioEnhanceChainManager::InitEnhanceBuffer()
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    uint32_t len = 0;
    uint32_t lenEc = 0;
    uint32_t lenMicRef = 0;
    for (auto &[scode, chain] : sceneTypeToEnhanceChainMap_) {
        if (chain) {
            len = std::max(len, chain->GetAlgoBufferSize());
            lenEc = std::max(lenEc, chain->GetAlgoBufferSizeEc());
            lenMicRef = std::max(lenMicRef, chain->GetAlgoBufferSizeMicRef());
        }
    }
    if (enhanceBuffer_ == nullptr) {
        enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
        enhanceBuffer_->micBufferIn.resize(len);
        enhanceBuffer_->micBufferOut.resize(len);
        enhanceBuffer_->ecBuffer.resize(lenEc);
        enhanceBuffer_->micRefBuffer.resize(lenMicRef);
        AUDIO_INFO_LOG("enhanceBuffer_ init len:%{public}u lenEc:%{public}u lenMicRef:%{public}u",
            len, lenEc, lenMicRef);
        return SUCCESS;
    }
    if ((len > enhanceBuffer_->micBufferIn.size())) {
        enhanceBuffer_->micBufferIn.resize(len);
        enhanceBuffer_->micBufferOut.resize(len);
    }
    if (lenEc > enhanceBuffer_->ecBuffer.size()) {
        enhanceBuffer_->ecBuffer.resize(lenEc);
    }
    if (lenMicRef > enhanceBuffer_->micRefBuffer.size()) {
        enhanceBuffer_->micRefBuffer.resize(lenMicRef);
    }
    AUDIO_INFO_LOG("enhanceBuffer_ update len:%{public}u lenEc:%{public}u lenMicRef:%{public}u",
        len, lenEc, lenMicRef);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ParseSceneKeyCode(const uint32_t sceneKeyCode, std::string &sceneType,
    std::string &capturerDeviceStr, std::string &rendererDeviceStr)
{
    uint32_t sceneTypeMask = SCENE_TYPE_MASK;
    uint32_t sceneCode = (sceneKeyCode & sceneTypeMask) >> 16;
    AudioEnhanceScene scene = static_cast<AudioEnhanceScene>(sceneCode);
    auto item = AUDIO_ENHANCE_SUPPORTED_SCENE_TYPES.find(scene);
    if (item != AUDIO_ENHANCE_SUPPORTED_SCENE_TYPES.end()) {
        sceneType = item->second;
    } else {
        return ERROR;
    }
    uint32_t captureIdMask = CAPTURER_ID_MASK;
    uint32_t captureId = (sceneKeyCode & captureIdMask) >> 8;
    DeviceType capturerDevice = captureIdToDeviceMap_[captureId];
    uint32_t renderIdMask = RENDERER_ID_MASK;
    uint32_t renderId = (sceneKeyCode & renderIdMask);
    DeviceType rendererDevice = renderIdToDeviceMap_[renderId];

    auto deviceItem = SUPPORTED_DEVICE_TYPE.find(capturerDevice);
    if (deviceItem != SUPPORTED_DEVICE_TYPE.end()) {
        capturerDeviceStr = deviceItem->second;
    } else {
        return ERROR;
    }
    deviceItem = SUPPORTED_DEVICE_TYPE.find(rendererDevice);
    if (deviceItem != SUPPORTED_DEVICE_TYPE.end()) {
        rendererDeviceStr = deviceItem->second;
    } else {
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CreateAudioEnhanceChainDynamic(const uint32_t sceneKeyCode,
    const AudioEnhanceDeviceAttr &deviceAttr)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);

    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    auto it = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (it != sceneTypeToEnhanceChainMap_.end() && it->second != nullptr) {
        sceneTypeToEnhanceChainCountMap_[sceneKeyCode]++;
        audioEnhanceChain = it->second;
        if (audioEnhanceChain->IsEmptyEnhanceHandles()) {
            return ERROR;
        }
        return audioEnhanceChain->IsDefaultChain();
    }
    bool createFlag = false;
    bool defaultFlag = false;
    if (CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr, createFlag, defaultFlag)) {
        return ERROR;
    }
    // means map to default chain
    if (!createFlag) {
        return audioEnhanceChain->IsDefaultChain();
    }
    std::string effectChainName = GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    if (AddAudioEnhanceChainHandles(audioEnhanceChain, effectChainName) != SUCCESS) {
        AUDIO_ERR_LOG("%{public}u create failed.", sceneKeyCode);
        return ERROR;
    }
    AUDIO_INFO_LOG("%{public}u create success", sceneKeyCode);
    return audioEnhanceChain->IsDefaultChain();
}

std::string AudioEnhanceChainManager::GetEnhanceChainNameBySceneCode(const uint32_t sceneKeyCode,
    const bool defaultFlag)
{
    std::string enhanceNone = AUDIO_ENHANCE_SUPPORTED_SCENE_MODES.find(ENHANCE_NONE)->second;
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce)) {
        return enhanceNone;
    }
    if (defaultFlag) {
        AUDIO_INFO_LOG("sceneType %{public}s set to defaultScene %{public}s", sceneType.c_str(),
            defaultScene_.c_str());
        sceneType = defaultScene_;
    }
    // first check specific device, then check no device
    std::string enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT" + "_&_" + capturerDevice;
    auto mapIter = sceneTypeAndModeToEnhanceChainNameMap_.find(enhanceChainKey);
    if (mapIter == sceneTypeAndModeToEnhanceChainNameMap_.end()) {
        enhanceChainKey = sceneType + "_&_" + "ENHANCE_DEFAULT";
        mapIter = sceneTypeAndModeToEnhanceChainNameMap_.find(enhanceChainKey);
    }
    if (mapIter == sceneTypeAndModeToEnhanceChainNameMap_.end() ||
        !enhanceChainToEnhancesMap_.count(mapIter->second)) {
        AUDIO_ERR_LOG("EnhanceChain key [%{public}s] does not exist, auto set to %{public}s",
            enhanceChainKey.c_str(), enhanceNone.c_str());
        return enhanceNone;
    } else {
        return mapIter->second;
    }
}

int32_t AudioEnhanceChainManager::CreateEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const uint32_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr, bool &createFlag, bool &defaultFlag)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce)) {
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
    createFlag = true;
    // normal scene
    if (priorSceneSet_.find(sceneType) == priorSceneSet_.end()) {
        // effect instance exceeded
        if (captureId2SceneCount_[captureId] == normalSceneLimit_) {
            // default enhance chain not exsist
            if (captureId2DefaultChainCount_[captureId] != 0) {
                createFlag = false;
                audioEnhanceChain = captureId2DefaultChain_[captureId];
                // add sceneType change after integration supported
                AUDIO_INFO_LOG("captureId %{public}u defaultChainExsist", captureId);
            } else {
                AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
                    capturerDevice, rendererDeivce, defaultScene_};
                audioEnhanceChain = std::make_shared<AudioEnhanceChain>(defaultScene_, algoParam, deviceAttr, 1);
                captureId2DefaultChain_[captureId] = audioEnhanceChain;
                AUDIO_INFO_LOG("captureId %{public}u defaultChain not exsist, create it", captureId);
            }
            captureId2DefaultChainCount_[captureId]++;
            defaultFlag = true;
        } else {
            AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
                capturerDevice, rendererDeivce, sceneType};
            audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, 0);
            captureId2SceneCount_[captureId]++;
            AUDIO_INFO_LOG("captureId %{public}u create normal scene %{public}s", captureId, sceneType.c_str());
        }
    } else {
        AudioEnhanceParamAdapter algoParam = {(uint32_t)isMute_, (uint32_t)(systemVol_ * VOLUME_FACTOR),
            capturerDevice, rendererDeivce, sceneType};
        audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, 0);
        AUDIO_INFO_LOG("priorScene %{public}s chain created", sceneType.c_str());
    }
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, ERROR, "AudioEnhanceChain construct failed.");
    sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    sceneTypeToEnhanceChainCountMap_[sceneKeyCode] = 1;
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::AddAudioEnhanceChainHandles(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const std::string &enhanceChain)
{
    for (std::string enhance : enhanceChainToEnhancesMap_[enhanceChain]) {
        AudioEffectHandle handle = nullptr;
        AudioEffectDescriptor descriptor;
        descriptor.libraryName = enhanceToLibraryNameMap_[enhance];
        descriptor.effectName = enhance;

        AUDIO_INFO_LOG("libraryName: %{public}s effectName:%{public}s",
            descriptor.libraryName.c_str(), descriptor.effectName.c_str());
        int32_t ret = enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle->createEffect(descriptor, &handle);
        CHECK_AND_CONTINUE_LOG(ret == 0, "EnhanceToLibraryEntryMap[%{public}s] createEffect fail",
            enhance.c_str());
        auto propIter = enhancePropertyMap_.find(enhance);
        audioEnhanceChain->AddEnhanceHandle(handle, enhanceToLibraryEntryMap_[enhance]->audioEffectLibHandle,
            enhance, propIter == enhancePropertyMap_.end() ? "" : propIter->second);
    }

    if (audioEnhanceChain->IsEmptyEnhanceHandles()) {
        AUDIO_ERR_LOG("EnhanceChain is empty, copy bufIn to bufOut like EFFECT_NONE mode");
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::FreeEnhanceBuffer()
{
    if (enhanceBuffer_ != nullptr) {
        std::vector<uint8_t>().swap(enhanceBuffer_->ecBuffer);
        std::vector<uint8_t>().swap(enhanceBuffer_->micBufferIn);
        std::vector<uint8_t>().swap(enhanceBuffer_->micBufferOut);
        std::vector<uint8_t>().swap(enhanceBuffer_->micRefBuffer);
        enhanceBuffer_ = nullptr;
        AUDIO_INFO_LOG("release EnhanceBuffer success");
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ReleaseAudioEnhanceChainDynamic(const uint32_t sceneKeyCode)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");

    auto chainMapIter = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (chainMapIter == sceneTypeToEnhanceChainMap_.end() || chainMapIter->second == nullptr) {
        sceneTypeToEnhanceChainCountMap_.erase(sceneKeyCode);
        sceneTypeToEnhanceChainMap_.erase(sceneKeyCode);
        return SUCCESS;
    }
    auto chainCountIter = sceneTypeToEnhanceChainCountMap_.find(sceneKeyCode);
    if (chainCountIter->second > 1) {
        chainCountIter->second--;
        return SUCCESS;
    }
    if (DeleteEnhanceChainInner(chainMapIter->second, sceneKeyCode) != SUCCESS) {
        return ERROR;
    }
    AUDIO_INFO_LOG("release %{public}u", sceneKeyCode);
    if (sceneTypeToEnhanceChainMap_.size() == 0) {
        FreeEnhanceBuffer();
    }
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::DeleteEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
    const uint32_t sceneKeyCode)
{
    std::string sceneType = "";
    std::string capturerDevice = "";
    std::string rendererDeivce = "";
    if (ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDevice, rendererDeivce)) {
        return ERROR;
    }
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
    sceneTypeToEnhanceChainCountMap_.erase(sceneKeyCode);
    sceneTypeToEnhanceChainMap_.erase(sceneKeyCode);
    // not prior scene
    if (priorSceneSet_.find(sceneType) == priorSceneSet_.end()) {
        // default chain
        if (audioEnhanceChain->IsDefaultChain()) {
            auto &defaultChainCount = captureId2DefaultChainCount_.find(captureId)->second;
            defaultChainCount--;
            // delete default chain
            if (!defaultChainCount) {
                captureId2DefaultChain_[captureId] = nullptr;
                AUDIO_INFO_LOG("captureId %{public}u default chain release", captureId);
            }
        } else {
            captureId2SceneCount_[captureId]--;
        }
    }
    return SUCCESS;
}

bool AudioEnhanceChainManager::ExistAudioEnhanceChain(const uint32_t sceneKeyCode)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, false, "has not been initialized");

    if (!sceneTypeToEnhanceChainMap_.count(sceneKeyCode)) {
        return false;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneKeyCode];
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, false, "null sceneTypeToEnhanceChainMap_[%{public}u]",
        sceneKeyCode);
    return !audioEnhanceChain->IsEmptyEnhanceHandles();
}

int32_t AudioEnhanceChainManager::AudioEnhanceChainGetAlgoConfig(const uint32_t sceneKeyCode,
    AudioBufferConfig &config, bool &needEcFlag, bool &needMicRefFlag)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    if (!sceneTypeToEnhanceChainMap_.count(sceneKeyCode)) {
        AUDIO_ERR_LOG("sceneTypeToEnhanceChainMap_ have not %{public}u", sceneKeyCode);
        return ERROR;
    }
    auto audioEnhanceChain = sceneTypeToEnhanceChainMap_[sceneKeyCode];
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChain != nullptr, ERROR, "[%{public}u] get config faild",
        sceneKeyCode);
    audioEnhanceChain->GetAlgoConfig(config);
    audioEnhanceChain->GetEcAndMicRefFlag(needEcFlag, needMicRefFlag);
    return SUCCESS;
}

bool AudioEnhanceChainManager::IsEmptyEnhanceChain()
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    CHECK_AND_RETURN_RET_LOG(isInitialized_, ERROR, "has not been initialized");
    for (auto &[scode, chain] : sceneTypeToEnhanceChainMap_) {
        if (chain != nullptr && !chain->IsEmptyEnhanceHandles()) {
            return false;
        }
    }
    return true;
}

int32_t AudioEnhanceChainManager::CopyToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("length: %{public}zu chunk length: %{public}u", enhanceBuffer_->micBufferIn.size(), length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferIn.data(), enhanceBuffer_->micBufferIn.size(), data,
        length) == 0, ERROR, "memcpy error in data to enhanceBuffer->micBufferIn");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyEcToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("lengthEc: %{public}zu chunkEc length: %{public}u", enhanceBuffer_->ecBuffer.size(), length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->ecBuffer.data(), enhanceBuffer_->ecBuffer.size(), data,
        length) == 0, ERROR, "memcpy error in data to enhanceBuffer->ecBuffer");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyMicRefToEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    AUDIO_DEBUG_LOG("lengthMicRef: %{public}zu chunkMicRef length: %{public}u", enhanceBuffer_->micRefBuffer.size(),
        length);
    CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micRefBuffer.data(), enhanceBuffer_->micRefBuffer.size(),
        data, length) == 0, ERROR, "memcpy error in data to enhanceBuffer->micBufferIn");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::CopyFromEnhanceBuffer(void *data, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    if (enhanceBuffer_ == nullptr) {
        return ERROR;
    }
    if (length > enhanceBuffer_->micBufferOut.size()) {
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(memcpy_s(data, length, enhanceBuffer_->micBufferOut.data(), length) == 0,
        ERROR, "memcpy error in micBufferOut to data");
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::ApplyAudioEnhanceChain(const uint32_t sceneKeyCode, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto chainMapIter = sceneTypeToEnhanceChainMap_.find(sceneKeyCode);
    if (chainMapIter == sceneTypeToEnhanceChainMap_.end() || chainMapIter->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferOut.data(), enhanceBuffer_->micBufferOut.size(),
            enhanceBuffer_->micBufferIn.data(), length) == 0, ERROR, "memcpy error in apply enhance");
        AUDIO_ERR_LOG("Can not find %{public}u in sceneTypeToEnhanceChainMap_", sceneKeyCode);
        return ERROR;
    }
    auto audioEnhanceChain = chainMapIter->second;
    if (audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer_, length) != SUCCESS) {
        AUDIO_ERR_LOG("Apply %{public}u failed.", sceneKeyCode);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Apply %{public}u success", sceneKeyCode);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto item = captureIdToDeviceMap_.find(captureId);
    if (item == captureIdToDeviceMap_.end()) {
        captureIdToDeviceMap_[captureId] = inputDevice;
        AUDIO_INFO_LOG("set new inputdevice, captureId: %{public}d, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }
    if (item->second == inputDevice) {
        AUDIO_INFO_LOG("set same device, captureId: %{public}d, inputDevice: %{public}d", captureId, inputDevice);
        return SUCCESS;
    }
    // item->second != inputDevice
    captureIdToDeviceMap_[captureId] = inputDevice;
    std::string inputDeviceStr = "";
    auto deviceItem = SUPPORTED_DEVICE_TYPE.find(inputDevice);
    if (deviceItem != SUPPORTED_DEVICE_TYPE.end()) {
        inputDeviceStr = deviceItem->second;
    } else {
        return ERROR;
    }
    for (auto &[sceneKeyCode, chain] : sceneTypeToEnhanceChainMap_) {
        uint32_t tempId = (sceneKeyCode & CAPTURER_ID_MASK) >> 8;
        if ((tempId == captureId) && chain) {
            if (chain->SetInputDevice(inputDeviceStr) != SUCCESS) {
                AUDIO_ERR_LOG("chain:%{public}u set input device failed", tempId);
            }
        }
    }
    AUDIO_INFO_LOG("success, captureId: %{public}d, inputDevice: %{public}d", captureId, inputDevice);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    renderIdToDeviceMap_.insert_or_assign(renderId, outputDevice);
    AUDIO_INFO_LOG("success, renderId: %{public}d, outputDevice: %{public}d", renderId, outputDevice);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol)
{
    volumeType_ = volumeType;
    systemVol_ = systemVol;
    AUDIO_INFO_LOG("success, volumeType: %{public}d, systemVol: %{public}f", volumeType_, systemVol_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetMicrophoneMuteInfo(const bool &isMute)
{
    isMute_ = isMute;
    AUDIO_INFO_LOG("success, isMute: %{public}d", isMute_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol)
{
    sessionId_ = sessionId;
    streamVol_ = streamVol;
    AUDIO_INFO_LOG("success, sessionId: %{public}d, streamVol: %{public}f", sessionId_, streamVol_);
    return SUCCESS;
}

int32_t AudioEnhanceChainManager::SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    int32_t ret = 0;
    for (const auto &property : propertyArray.property) {
        enhancePropertyMap_.insert_or_assign(property.enhanceClass, property.enhanceProp);
        for (const auto &[sceneType, enhanceChain] : sceneTypeToEnhanceChainMap_) {
            if (enhanceChain) {
                ret = enhanceChain->SetEnhanceProperty(property.enhanceClass, property.enhanceProp);
                CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "set property failed");
            }
        }
    }
    return 0;
}

int32_t AudioEnhanceChainManager::GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    propertyArray.property.clear();
    for (const auto &[effect, prop] : enhancePropertyMap_) {
        if (!prop.empty()) {
            propertyArray.property.emplace_back(AudioEnhanceProperty{effect, prop});
            AUDIO_INFO_LOG("effect %{public}s is now %{public}s mode",
                effect.c_str(), prop.c_str());
        }
    }
    return AUDIO_OK;
}

int32_t AudioEnhanceChainManager::ApplyAudioEnhanceChainDefault(const uint32_t captureId, uint32_t length)
{
    std::lock_guard<std::mutex> lock(chainManagerMutex_);
    auto chainMapIter = captureId2DefaultChain_.find(captureId);
    if (chainMapIter == captureId2DefaultChain_.end() || chainMapIter->second == nullptr) {
        CHECK_AND_RETURN_RET_LOG(memcpy_s(enhanceBuffer_->micBufferOut.data(), enhanceBuffer_->micBufferOut.size(),
            enhanceBuffer_->micBufferIn.data(), length) == 0, ERROR, "memcpy error in apply enhance");
        AUDIO_ERR_LOG("Can not find %{public}u in captureId2DefaultChain_", captureId);
        return ERROR;
    }
    auto audioEnhanceChain = chainMapIter->second;
    if (audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer_, length) != SUCCESS) {
        AUDIO_ERR_LOG("Apply default chain failed with captureId %{public}u.", captureId);
        return ERROR;
    }
    AUDIO_DEBUG_LOG("Apply default chain success with captureId %{public}u.", captureId);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
