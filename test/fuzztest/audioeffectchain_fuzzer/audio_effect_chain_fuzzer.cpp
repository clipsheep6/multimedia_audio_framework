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

#include <cstddef>
#include <cstdint>
#include "audio_effect.h"
#include "audio_effect_chain.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_chain_adapter.h"
#include "audio_errors.h"
#include "audio_head_tracker.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const int32_t LIMITSIZE = 4;
const char* SCENETYPEMUSIC = "SCENE_MUSIC";
const char* SESSIONIDDEFAULT = "123456";
const char* EFFECTDEFAULT = "EFFECT_DEFAULT";
const uint32_t AUDIOEFFECTSCENE_LENGTH = 6;
const uint32_t AUDIOENCODINGTYPE_LENGTH = 3;
const string EXTRASCENETYPE = "2";

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};
unordered_map<string, string> DEFAULT_MAP = {
    {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
    {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"},
};

#define DEFAULT_NUM_CHANNEL 2
#define DEFAULT_CHANNELLAYOUT 3

void EffectChainManagerInitCbFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);

    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);
    sceneType = nullptr;
    EffectChainManagerInitCb(sceneType);
    sceneType = "";
    EffectChainManagerInitCb(sceneType);
}

void EffectChainManagerCreateCbFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = SESSIONIDDEFAULT;
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
    sessionid = "";
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
}

void EffectChainManagerSetHdiParamFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);
    const char *effectMode = EFFECTDEFAULT;
    EffectChainManagerSetHdiParam(sceneType, effectMode, true);
    sceneType = nullptr;
    effectMode = nullptr;
    EffectChainManagerSetHdiParam(sceneType, effectMode, true);
}

void EffectChainManagerCheckEffectOffloadFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const char *sceneType = "";
    EffectChainManagerInitCb(sceneType);
    EffectChainManagerCheckEffectOffload();
}

void EffectChainManagerAddSessionInfoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = reinterpret_cast<const char*>(rawData);
    if (strcmp(sessionid, SESSIONIDDEFAULT) != 0 || strcmp(sessionid, "") != 0) {
        sessionid = SESSIONIDDEFAULT;
    }
    SessionInfoPack pack = {2, "3", EFFECTDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
}

void EffectChainManagerDeleteSessionInfoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0 || strcmp(sceneType, "") != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = reinterpret_cast<const char*>(rawData);
    if (strcmp(sessionid, SESSIONIDDEFAULT) != 0 || strcmp(sessionid, "") != 0) {
        sessionid = SESSIONIDDEFAULT;
    }
    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);
    EffectChainManagerDeleteSessionInfo(sceneType, sessionid);
}

void EffectChainManagerReturnEffectChannelInfoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < (sizeof(uint32_t)+sizeof(uint64_t))) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0 || strcmp(sceneType, "") != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = reinterpret_cast<const char*>(rawData);
    if (strcmp(sessionid, SESSIONIDDEFAULT) != 0 || strcmp(sessionid, "") != 0) {
        sessionid = SESSIONIDDEFAULT;
    }
    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);

    uint32_t processChannels = *reinterpret_cast<const uint32_t*>(rawData);
    uint64_t processChannelLayout = *reinterpret_cast<const uint64_t*>(rawData+sizeof(processChannels));
    EffectChainManagerReturnEffectChannelInfo(sceneType, &processChannels, &processChannelLayout);
}

void EffectChainManagerReturnMultiChannelInfoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < (sizeof(uint32_t)+sizeof(uint64_t))) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0 || strcmp(sceneType, "") != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    uint32_t processChannels = *reinterpret_cast<const uint32_t*>(rawData);
    uint64_t processChannelLayout = *reinterpret_cast<const uint64_t*>(rawData+sizeof(processChannels));
    EffectChainManagerReturnMultiChannelInfo(&processChannels, &processChannelLayout);
}

void EffectChainManagerGetSpatializationEnabledFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0 || strcmp(sceneType, "") != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    EffectChainManagerGetSpatializationEnabled();
    EffectChainManagerFlush();
}

void EffectChainManagerSceneCheckFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0 || strcmp(sceneType, "") != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);
    
    const char *sinkSceneType = reinterpret_cast<const char*>(rawData);
    EffectChainManagerSceneCheck(sinkSceneType, sceneType);
}

void EffectChainManagerProcessFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);
    
    struct BufferAttr bufferAttr;
    char sceneType2[] = "SCENE_MUSIC";
    EffectChainManagerProcess(sceneType2, &bufferAttr);
}

void EffectChainManagerMultichannelUpdateFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);
    EffectChainManagerMultichannelUpdate(nullptr);
    EffectChainManagerMultichannelUpdate(sceneType);
}

void EffectChainManagerExistFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);

    const char *effectMode = EFFECTDEFAULT;
    const char *spatializationEnabled = reinterpret_cast<const char*>(rawData);
    EffectChainManagerExist(sceneType, effectMode, spatializationEnabled);
}

void EffectChainManagerVolumeUpdateFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const char *sceneType = reinterpret_cast<const char*>(rawData);
    if (strcmp(sceneType, SCENETYPEMUSIC) != 0) {
        sceneType = SCENETYPEMUSIC;
    }
    EffectChainManagerInitCb(sceneType);
    
    const char *sessionId = reinterpret_cast<const char*>(rawData);
    if (!sessionId) {
        sessionId = SESSIONIDDEFAULT;
    }
    EffectChainManagerVolumeUpdate(sessionId);
}

void AudioEffectChainManagerFirst(const uint8_t* rawData, size_t size,
    std::shared_ptr<AudioEffectChain> audioEffectChain)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    audioEffectChain->SetEffectMode("EFFECT_DEFAULT");
    audioEffectChain->GetEffectMode();
    audioEffectChain->SetExtraSceneType(EXTRASCENETYPE);
    uint32_t currSceneType_int = *reinterpret_cast<const uint32_t*>(rawData);
    currSceneType_int = (currSceneType_int%AUDIOEFFECTSCENE_LENGTH);
    AudioEffectScene currSceneType = static_cast<AudioEffectScene>(currSceneType_int);
    audioEffectChain->SetEffectCurrSceneType(currSceneType);
    float* bufIn = const_cast<float *>(reinterpret_cast<const float*>(rawData));
    float* bufOut = const_cast<float *>(reinterpret_cast<const float*>(rawData));
    uint32_t frameLen = *reinterpret_cast<const uint32_t*>(rawData);
    AudioEffectProcInfo procInfo;
    bool headTrackingEnabled = *reinterpret_cast<const bool*>(rawData);
    bool btOffloadEnabled = *reinterpret_cast<const bool*>(rawData);
    procInfo.btOffloadEnabled = btOffloadEnabled;
    procInfo.headTrackingEnabled = headTrackingEnabled;
    audioEffectChain->ApplyEffectChain(bufIn, bufOut, frameLen, procInfo);
    audioEffectChain->IsEmptyEffectHandles();
    audioEffectChain->Dump();
    const uint32_t channels = *reinterpret_cast<const uint32_t*>(rawData);
    const uint64_t channelLayout = *reinterpret_cast<const uint64_t*>(rawData);
    audioEffectChain->UpdateMultichannelIoBufferConfig(channels, channelLayout);
    std::string sceneMode(reinterpret_cast<const char*>(rawData), size-1);
    AudioEffectConfig ioBufferConfig;
    AudioBufferConfig inputCfg;
    uint32_t samplingRate = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t channel = *reinterpret_cast<const uint32_t*>(rawData);
    uint8_t format = *reinterpret_cast<const uint8_t*>(rawData);
    uint64_t channelLayouts = *reinterpret_cast<const uint64_t*>(rawData);
    uint32_t encoding_int = *reinterpret_cast<const uint32_t*>(rawData);
    encoding_int = (encoding_int%AUDIOENCODINGTYPE_LENGTH)-1;
    AudioEncodingType encoding = static_cast<AudioEncodingType>(encoding_int);
    inputCfg.samplingRate = samplingRate;
    inputCfg.channels = channel;
    inputCfg.format = format;
    inputCfg.channelLayout = channelLayouts;
    inputCfg.encoding = encoding;
    ioBufferConfig.inputCfg = inputCfg;
    ioBufferConfig.outputCfg = inputCfg;
    audioEffectChain->StoreOldEffectChainInfo(sceneMode, ioBufferConfig);
}

void AudioEffectChainFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(uint64_t)) {
        return;
    }
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneType = "SCENE_MUSIC";
#ifdef SENSOR_ENABLE
    std::shared_ptr<HeadTracker> headTracker = nullptr;
    headTracker = std::make_shared<HeadTracker>();
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
#else
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif

    AudioEffectChainManagerFirst(rawData, size, audioEffectChain);
}
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::EffectChainManagerInitCbFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerCreateCbFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerSetHdiParamFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerCheckEffectOffloadFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerAddSessionInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerDeleteSessionInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerReturnEffectChannelInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerReturnMultiChannelInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerGetSpatializationEnabledFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerSceneCheckFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerProcessFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerMultichannelUpdateFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerExistFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectChainManagerVolumeUpdateFuzzTest(rawData, size);

    OHOS::AudioStandard::AudioEffectChainFuzzTest(rawData, size);
    return 0;
}