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

#ifndef AUDIO_ENHANCE_CHAIN_H
#define AUDIO_ENHANCE_CHAIN_H

#include <vector>
#include <mutex>
#include <map>

#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {

const std::vector<std::string> NEED_EC_SCENE = {
    "SCENE_VOIP_UP",
    "SCENE_PRE_ENHANCE",
};

struct EnhanceBuffer {
    std::vector<uint8_t> micBufferIn;
    std::vector<uint8_t> micBufferOut;
    std::vector<uint8_t> ecBuffer;
    std::vector<uint8_t> micRefBuffer;
    uint32_t length;
    uint32_t lengthEc;
    uint32_t lengthMicRef;
};

struct AlgoAttr {
    uint32_t bitDepth;
    uint32_t batchLen;
    uint32_t byteLenPerFrame;
};

struct AlgoCache {
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
};

struct AudioEnhanceDeviceAttr {
    uint32_t micRate;
    uint32_t micChannels;
    uint32_t micFormat;
    bool needEc;
    uint32_t ecRate;
    uint32_t ecChannels;
    uint32_t ecFormat;
    bool needMicRef;
    uint32_t micRefRate;
    uint32_t micRefChannels;
    uint32_t micRefFormat;
};

class AudioEnhanceChain {
public:
    AudioEnhanceChain(const std::string &scene, const AudioEnhanceParam &algoParam);
    ~AudioEnhanceChain();
    void AddEnhanceHandle(AudioEffectHandle handle, AudioEffectLibrary *libHandle, const std::string &enhance,
        const std::string &property);
    bool IsEmptyEnhanceHandles();
    int32_t SetDeviceAttr(const AudioEnhanceDeviceAttr &deviceAttr);
    void GetAlgoConfig(AudioBufferConfig &algoConfig);
    uint32_t GetAlgoBufferSize();
    uint32_t GetAlgoBufferSizeEc();
    uint32_t GetAlgoBufferSizeMicRef();
    int32_t ApplyEnhanceChain(std::unique_ptr<EnhanceBuffer> &enhanceBuffer, uint32_t length);
    int32_t SetEnhanceProperty(const std::string &effect, const std::string &property);
    int32_t SetEnhanceParamToHandle(AudioEffectHandle handle);

private:
    void InitAudioEnhanceChain();
    void InitDump();
    void ReleaseEnhanceChain();
    int32_t GetOneFrameInputData(std::unique_ptr<EnhanceBuffer> &enhanceBuffer);
    int32_t SetPropertyToHandle(AudioEffectHandle handle, const std::string &property);
    int32_t UpdateAlgoConfig();

    bool setConfigFlag_;
    std::mutex chainMutex_;
    std::string sceneType_;
    AlgoAttr algoAttr_;
    AlgoConfig algoSupportedConfig_;
    AlgoCache algoCache_;
    AudioEnhanceParam algoParam_;
    AudioEnhanceDeviceAttr deviceAttr_;
    FILE *dumpFileIn_ = nullptr;
    FILE *dumpFileEc_ = nullptr;
    FILE *dumpFileMicRef_ = nullptr;
    FILE *dumpFileOut_ = nullptr;
    bool needEcFlag_;
    std::vector<AudioEffectHandle> standByEnhanceHandles_;
    std::vector<std::string> enhanceNames_;
    std::vector<AudioEffectLibrary*> enhanceLibHandles_;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_H