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

#ifndef AUDIO_ENHANCE_CHAIN_MANAGER_H
#define AUDIO_ENHANCE_CHAIN_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <set>

#include "audio_effect.h"
#include "audio_enhance_chain.h"

namespace OHOS {
namespace AudioStandard {

class AudioEnhanceChainManager {
public:
    AudioEnhanceChainManager();
    ~AudioEnhanceChainManager();
    static AudioEnhanceChainManager* GetInstance();
    void InitAudioEnhanceChainManager(std::vector<EffectChain> &enhanceChains,
        std::unordered_map<std::string, std::string> &enhanceChainNameMap,
        std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList);
    int32_t CreateAudioEnhanceChainDynamic(const std::string &scene, const std::string &mode, const std::string &up,
        const std::string &down);
    int32_t ReleaseAudioEnhanceChainDynamic(const std::string &sceneType, const std::string &upDevice,
        const std::string &downDevice);
    bool ExistAudioEnhanceChain(const std::string &sceneKey);
    int32_t AudioEnhanceChainGetAlgoConfig(const std::string &sceneKey, AudioBufferConfig &config);
    bool IsEmptyEnhanceChain();
    int32_t InitEnhanceBuffer();
    int32_t CopyToEnhanceBuffer(void *data, uint32_t length);
    int32_t CopyFromEnhanceBuffer(void *data, uint32_t length);
    int32_t ApplyAudioEnhanceChain(const std::string &sceneKey, uint32_t length);
    int32_t SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice);
    int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice);
    int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol);
    int32_t SetMicrophoneMuteInfo(const bool &isMute);
    int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol);
	
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
private:
    int32_t SetAudioEnhanceChainDynamic(const std::string &sceneType, const std::string &sceneMode,
        const std::string &upDevice, const std::string &downDevice);

    int32_t FreeEnhanceBuffer();
    
    std::map<std::string, std::shared_ptr<AudioEnhanceChain>> sceneTypeToEnhanceChainMap_;
    std::map<std::string, int32_t> sceneTypeToEnhanceChainCountMap_;
    std::map<std::string, std::string> sceneTypeAndModeToEnhanceChainNameMap_;
    std::map<std::string, std::vector<std::string>> enhanceChainToEnhancesMap_;
    std::map<std::string, std::shared_ptr<AudioEffectLibEntry>> enhanceToLibraryEntryMap_;
    std::map<std::string, std::string> enhanceToLibraryNameMap_;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer_ = nullptr;
    std::mutex chainManagerMutex_;
    bool isInitialized_;
    uint32_t captureId_ = 0;
    uint32_t renderId_ = 0;
    uint32_t sessionId_ = 0;
    DeviceType inputDevice_ = DEVICE_TYPE_MIC;
    DeviceType outputDevice_ = DEVICE_TYPE_SPEAKER;
    AudioVolumeType volumeType_ = STREAM_MUSIC;
    float systemVol_ = 0.0f;
    float streamVol_ = 0.0f;
    bool isMute_ = false;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_MANAGER_H
