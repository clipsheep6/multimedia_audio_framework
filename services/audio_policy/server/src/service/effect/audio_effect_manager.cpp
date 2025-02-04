/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioEffectManager"
#endif

#include "audio_effect_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioEffectManager::AudioEffectManager()
{
    AUDIO_INFO_LOG("AudioEffectManager ctor");
}

AudioEffectManager::~AudioEffectManager()
{
}

void AudioEffectManager::EffectManagerInit()
{
    // load XML
    std::unique_ptr<AudioEffectConfigParser> effectConfigParser = std::make_unique<AudioEffectConfigParser>();
    int32_t ret = effectConfigParser->LoadEffectConfig(oriEffectConfig_);
    CHECK_AND_RETURN_LOG(ret == 0, "AudioEffectManager->effectConfigParser failed: %{public}d", ret);
}

void AudioEffectManager::GetAvailableEffects(std::vector<Effect> &availableEffects)
{
    availableEffects = availableEffects_;
}

void AudioEffectManager::GetOriginalEffectConfig(OriginalEffectConfig &oriEffectConfig)
{
    oriEffectConfig = oriEffectConfig_;
}

void AudioEffectManager::UpdateAvailableEffects(std::vector<Effect> &newAvailableEffects)
{
    availableEffects_ = newAvailableEffects;
}

int32_t AudioEffectManager::QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig)
{
    supportedEffectConfig = supportedEffectConfig_;
    return existDefault_;
}

void AudioEffectManager::GetSupportedEffectConfig(SupportedEffectConfig &supportedEffectConfig)
{
    supportedEffectConfig = supportedEffectConfig_;
}

static int32_t UpdateUnsupportedScene(std::string &scene)
{
    int32_t isSupported = 0;
    if ((scene != "SCENE_MUSIC") &&
        (scene != "SCENE_MOVIE") &&
        (scene != "SCENE_GAME") &&
        (scene != "SCENE_SPEECH") &&
        (scene != "SCENE_RING") &&
        (scene != "SCENE_VOIP_3A") &&
        (scene != "SCENE_RECORD") &&
        (scene != "SCENE_MEETING") &&
        (scene != "SCENE_OTHERS")) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG9]:stream-> The scene of %{public}s is unsupported, \
            and this scene is deleted!", scene.c_str());
        isSupported = -1;
    }
    return isSupported;
}

static int32_t UpdateUnsupportedEnhanceScene(std::string &scene)
{
    int32_t isSupported = 0;
    if ((scene != "SCENE_VOIP_3A") &&
        (scene != "SCENE_RECORD")) {
        AUDIO_INFO_LOG("[supportedEnhanceConfig LOG9]:stream-> The scene of %{public}s is unsupported, \
            and this scene is deleted!", scene.c_str());
        isSupported = -1;
    }
    return isSupported;
}

static void UpdateUnsupportedDevicePre(Preprocess &pp, Stream &stream, const std::string &mode, int32_t i, int32_t j)
{
    StreamEffectMode streamEffectMode;
    streamEffectMode.mode = mode;
    j = 0;
    for (auto &device: pp.device) {
        if (i == j) {
            for (auto &eachDevice: device) {
                streamEffectMode.devicePort.push_back(eachDevice);
            }
            break;
        }
        j += 1;
    }
    stream.streamEffectMode.push_back(streamEffectMode);
}

static void UpdateUnsupportedModePre(Preprocess &pp, Stream &stream, std::string &mode, int32_t i)
{
    int32_t isSupported = 0;
    if ((mode != "ENHANCE_NONE") &&
        (mode != "ENHANCE_DEFAULT")) {
        AUDIO_INFO_LOG("[supportedEnhanceConfig LOG10]:mode-> The %{public}s mode of %{public}s is unsupported, \
            and this mode is deleted!", mode.c_str(), stream.scene.c_str());
        isSupported = -1;
    }
    if (isSupported == 0) {
        int32_t j = 0;
        UpdateUnsupportedDevicePre(pp, stream, mode, i, j);
    }
}

static void UpdateUnsupportedDevicePost(EffectSceneStream &ess, Stream &stream, const std::string &mode, int32_t i)
{
    StreamEffectMode streamEffectMode;
    streamEffectMode.mode = mode;
    int32_t j = 0;
    for (auto &device: ess.device) {
        if (i == j) {
            for (auto &a: device) {
                streamEffectMode.devicePort.push_back(a);
            }
            break;
        }
        j += 1;
    }
    stream.streamEffectMode.push_back(streamEffectMode);
}

static void UpdateUnsupportedModePost(EffectSceneStream &ess, Stream &stream, std::string &mode, int32_t i)
{
    int32_t isSupported = 0;
    if ((mode != "EFFECT_NONE") &&
        (mode != "EFFECT_DEFAULT")) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG10]:mode-> The %{public}s mode of %{public}s is unsupported, \
            and this mode is deleted!", mode.c_str(), stream.scene.c_str());
        isSupported = -1;
    }
    if (isSupported == 0) {
        UpdateUnsupportedDevicePost(ess, stream, mode, i);
    }
}

static int32_t UpdateAvailableStreamPre(ProcessNew &preProcessNew, Preprocess &pp)
{
    bool isDuplicate = false;
    int32_t isSupported = UpdateUnsupportedEnhanceScene(pp.stream);
    auto it = std::find_if(preProcessNew.stream.begin(), preProcessNew.stream.end(), [&pp](const Stream& x) {
        return x.scene == pp.stream;
    });
    if ((it == preProcessNew.stream.end()) && (isSupported == 0)) {
        Stream stream;
        stream.scene = pp.stream;
        int32_t i = 0;
        for (auto &mode: pp.mode) {
            UpdateUnsupportedModePre(pp, stream, mode, i);
        }
        preProcessNew.stream.push_back(stream);
    } else if (it != preProcessNew.stream.end()) {
        isDuplicate = true;
    }
    return isDuplicate;
}

static int32_t UpdateAvailableStreamPost(ProcessNew &postProcessNew, EffectSceneStream &ess)
{
    bool isDuplicate = false;
    int32_t isSupported = UpdateUnsupportedScene(ess.stream);
    auto it = std::find_if(postProcessNew.stream.begin(), postProcessNew.stream.end(), [&ess](const Stream& x) {
        return x.scene == ess.stream;
    });
    if ((it == postProcessNew.stream.end()) && (isSupported == 0)) {
        Stream stream;
        stream.scene = ess.stream;
        int32_t i = 0;
        for (auto &mode: ess.mode) {
            UpdateUnsupportedModePost(ess, stream, mode, i);
        }
        postProcessNew.stream.push_back(stream);
    } else if (it != postProcessNew.stream.end()) {
        isDuplicate = true;
    }
    return isDuplicate;
}

static int32_t UpdateAvailableSceneMapPost(SceneMappingItem &item, std::vector<SceneMappingItem> &postProcessSceneMap)
{
    bool isDuplicate = false;
    auto it = std::find_if(postProcessSceneMap.begin(), postProcessSceneMap.end(),
        [&item](const SceneMappingItem& x) {
        return x.name == item.name;
    });
    if ((it == postProcessSceneMap.end())) {
        postProcessSceneMap.push_back(item);
    } else {
        isDuplicate = true;
    }
    return isDuplicate;
}

bool AudioEffectManager::VerifySceneMappingItem(const SceneMappingItem &item)
{
    return STREAM_USAGE_MAP.find(item.name) != STREAM_USAGE_MAP.end() &&
        std::find(postSceneTypeSet_.begin(), postSceneTypeSet_.end(), item.sceneType) != postSceneTypeSet_.end();
}

void AudioEffectManager::UpdateEffectChains(std::vector<std::string> &availableLayout)
{
    int32_t count = 0;
    std::vector<int> deviceDelIdx;
    for (const auto &ec: supportedEffectConfig_.effectChains) {
        for (auto &effectName: ec.apply) {
            auto it = std::find_if(availableEffects_.begin(), availableEffects_.end(),
                [&effectName](const Effect& effect) {
                return effect.name == effectName;
            });
            if (it == availableEffects_.end()) {
                deviceDelIdx.emplace_back(count);
                break;
            }
        }
        count += 1;
    }
    for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
        supportedEffectConfig_.effectChains.erase(supportedEffectConfig_.effectChains.begin() + *it);
    }
    if (supportedEffectConfig_.effectChains.empty()) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG1]:effectChains-> all effectChains are unavailable");
    }
    for (auto ec: supportedEffectConfig_.effectChains) {
        availableLayout.emplace_back(ec.name);
    }
}

void AudioEffectManager::UpdateAvailableAEConfig(OriginalEffectConfig &aeConfig)
{
    bool isDuplicate = false;
    bool ret;
    supportedEffectConfig_.effectChains = aeConfig.effectChains;
    ProcessNew preProcessNew;
    for (Preprocess &pp: aeConfig.preProcess) {
        ret = UpdateAvailableStreamPre(preProcessNew, pp);
        if (ret == 1) {
            isDuplicate = true;
        }
    }
    ProcessNew postProcessNew;
    for (EffectSceneStream &ess: aeConfig.postProcess.effectSceneStreams) {
        ret = UpdateAvailableStreamPost(postProcessNew, ess);
        if (ret == 1) {
            isDuplicate = true;
        }
    }
    if (isDuplicate == true) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG2]:stream-> The duplicate stream is deleted, \
            and the first configuration is retained!");
    }
    supportedEffectConfig_.preProcessNew = preProcessNew;
    supportedEffectConfig_.postProcessNew = postProcessNew;

    for (Stream &ss: supportedEffectConfig_.postProcessNew.stream) {
        postSceneTypeSet_.push_back(ss.scene);
    }
    AUDIO_INFO_LOG("postSceneTypeSet_ size is %{public}zu", supportedEffectConfig_.postProcessNew.stream.size());
    std::vector<SceneMappingItem> postSceneMap;
    for (SceneMappingItem &item: aeConfig.postProcess.sceneMap) {
        if (!VerifySceneMappingItem(item)) {
            AUDIO_WARNING_LOG("Invalid %{public}s-%{public}s pair has been ignored",
                item.name.c_str(), item.sceneType.c_str());
            continue;
        }
        if (UpdateAvailableSceneMapPost(item, postSceneMap)) {
            AUDIO_WARNING_LOG("The duplicate streamUsage-sceneType pair is deleted, \
                and the first configuration is retained!");
        }
    }
    supportedEffectConfig_.postProcessSceneMap = postSceneMap;
}

void AudioEffectManager::UpdateDuplicateBypassMode(ProcessNew &preProcessNew)
{
    int32_t flag = 0;
    std::vector<int32_t> deviceDelIdx;
    for (auto &stream: preProcessNew.stream) {
        int32_t count = 0;
        deviceDelIdx.clear();
        for (const auto &streamEffectMode: stream.streamEffectMode) {
            if (streamEffectMode.mode == "EFFECT_NONE") {
                deviceDelIdx.push_back(count);
            }
            count += 1;
        }
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            stream.streamEffectMode[*it].devicePort = {};
            flag = -1;
        }
    }
    if (flag == -1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG3]:mode-> EFFECT_NONE can not configure by deveploer!");
    }
}

void AudioEffectManager::UpdateDuplicateMode(ProcessNew &preProcessNew)
{
    std::unordered_set<std::string> seen;
    std::vector<int32_t> toRemove;
    uint32_t i;
    for (auto &stream: preProcessNew.stream) {
        seen.clear();
        toRemove.clear();
        for (i = 0; i < stream.streamEffectMode.size(); i++) {
            if (seen.count(stream.streamEffectMode[i].mode)) {
                toRemove.push_back(i);
            } else {
                seen.insert(stream.streamEffectMode[i].mode);
            }
        }
        for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG4]:mode-> The duplicate mode of %{public}s configuration \
                is deleted, and the first configuration is retained!", stream.scene.c_str());
            stream.streamEffectMode.erase(stream.streamEffectMode.begin() + *it);
        }
    }
}

static void UpdateDuplicateDeviceRecord(StreamEffectMode &streamEffectMode, Stream &stream)
{
    uint32_t i;
    std::unordered_set<std::string> seen;
    std::vector<int32_t> toRemove;
    seen.clear();
    toRemove.clear();
    for (i = 0; i < streamEffectMode.devicePort.size(); i++) {
        if (seen.count(streamEffectMode.devicePort[i].type)) {
            toRemove.push_back(i);
        } else {
            seen.insert(streamEffectMode.devicePort[i].type);
        }
    }
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG5]:device-> The duplicate device of %{public}s's %{public}s \
            mode configuration is deleted, and the first configuration is retained!",
            stream.scene.c_str(), streamEffectMode.mode.c_str());
        streamEffectMode.devicePort.erase(streamEffectMode.devicePort.begin() + *it);
    }
}

void AudioEffectManager::UpdateDuplicateDevice(ProcessNew &preProcessNew)
{
    for (auto &stream: preProcessNew.stream) {
        for (auto &streamEffectMode: stream.streamEffectMode) {
            UpdateDuplicateDeviceRecord(streamEffectMode, stream);
        }
    }
}

static int32_t UpdateUnavailableModes(std::vector<int32_t> &modeDelIdx, Stream &stream)
{
    int32_t ret = 0;
    for (auto it = modeDelIdx.rbegin(); it != modeDelIdx.rend(); ++it) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG7]:mode-> %{public}s's %{public}s mode is deleted!",
            stream.scene.c_str(), stream.streamEffectMode[*it].mode.c_str());
        if (stream.streamEffectMode[*it].mode == "PLAYBACK_DEAFULT") {
            ret = -1;
        }
        stream.streamEffectMode.erase(stream.streamEffectMode.begin() + *it);
        if (stream.streamEffectMode.empty()) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG8]:mode-> %{public}s's mode is only EFFECT_NONE!",
                stream.scene.c_str());
            StreamEffectMode streamEffectMode;
            streamEffectMode.mode = "EFFECT_NONE";
            stream.streamEffectMode.push_back(streamEffectMode);
        }
    }
    if (stream.streamEffectMode.empty()) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG8]:mode-> %{public}s's mode is only EFFECT_NONE!",
            stream.scene.c_str());
        StreamEffectMode streamEffectMode;
        streamEffectMode.mode = "EFFECT_NONE";
        stream.streamEffectMode.push_back(streamEffectMode);
    }
    return ret;
}

static void UpdateUnavailableEffectChainsRecord(std::vector<std::string> &availableLayout, Stream &stream,
    StreamEffectMode &streamEffectMode, std::vector<int32_t> &modeDelIdx, int32_t modeCount)
{
    std::vector<int32_t> deviceDelIdx;
    deviceDelIdx.clear();
    int32_t deviceCount = 0;
    if (streamEffectMode.devicePort.empty()) {
        modeDelIdx.push_back(modeCount);
    }
    for (auto &devicePort: streamEffectMode.devicePort) {
        auto index = std::find(availableLayout.begin(), availableLayout.end(), devicePort.chain);
        if (index == availableLayout.end()) {
            deviceDelIdx.push_back(deviceCount);
        }
        deviceCount += 1;
    }
    if (streamEffectMode.devicePort.size() != deviceDelIdx.size() && deviceDelIdx.size() != 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG6]:device-> The unavailable effectChain \
            of %{public}s's %{public}s mode are set to LAYOUT_BYPASS!",
            stream.scene.c_str(), streamEffectMode.mode.c_str());
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            streamEffectMode.devicePort[*it].chain = "LAYOUT_BYPASS";
        }
    } else {
        for (auto it = deviceDelIdx.rbegin(); it != deviceDelIdx.rend(); ++it) {
            streamEffectMode.devicePort.erase(streamEffectMode.devicePort.begin() + *it);
            if (streamEffectMode.devicePort.empty()) {
                modeDelIdx.push_back(modeCount);
            }
        }
    }
}

int32_t AudioEffectManager::UpdateUnavailableEffectChains(std::vector<std::string> &availableLayout,
    ProcessNew &processNew)
{
    int32_t ret = 0;
    
    std::vector<int32_t> modeDelIdx;
    for (auto &stream: processNew.stream) {
        modeDelIdx.clear();
        int32_t modeCount = 0;
        for (auto &streamEffectMode: stream.streamEffectMode) {
            UpdateUnavailableEffectChainsRecord(availableLayout, stream, streamEffectMode, modeDelIdx, modeCount);
        }
        ret = UpdateUnavailableModes(modeDelIdx, stream);
    }
    return ret;
}

void AudioEffectManager::BuildAvailableAEConfig()
{
    int32_t ret = 0 ;
    std::vector<std::string> availableLayout;
    existDefault_ = 1;
    if (oriEffectConfig_.effectChains.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG12]: effectChains is none!");
    }
    if (oriEffectConfig_.preProcess.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG11]: preProcess is none!");
    }
    if (oriEffectConfig_.postProcess.effectSceneStreams.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG13]: postProcess is none!");
    }

    // Update duplicate defined modes, devices, and unsupported effect chain.
    UpdateAvailableAEConfig(oriEffectConfig_);
    UpdateEffectChains(availableLayout);

    UpdateDuplicateBypassMode(supportedEffectConfig_.preProcessNew);
    UpdateDuplicateMode(supportedEffectConfig_.preProcessNew);
    UpdateDuplicateDevice(supportedEffectConfig_.preProcessNew);
    ret = UpdateUnavailableEffectChains(availableLayout, supportedEffectConfig_.preProcessNew);
    if (ret != 0) {
        existDefault_ = -1;
    }

    UpdateDuplicateBypassMode(supportedEffectConfig_.postProcessNew);
    UpdateDuplicateMode(supportedEffectConfig_.postProcessNew);
    UpdateDuplicateDevice(supportedEffectConfig_.postProcessNew);
    ret = UpdateUnavailableEffectChains(availableLayout, supportedEffectConfig_.postProcessNew);
    if (ret != 0) {
        existDefault_ = -1;
    }
}

void AudioEffectManager::SetMasterSinkAvailable()
{
    isMasterSinkAvailable_ = true;
}

void AudioEffectManager::SetEffectChainManagerAvailable()
{
    isEffectChainManagerAvailable_ = true;
}

bool AudioEffectManager::CanLoadEffectSinks()
{
    return (isMasterSinkAvailable_ && isEffectChainManagerAvailable_);
}

template <typename T>
void AddKeyValueIntoMap(std::unordered_map<T, std::string> &map, std::string &key, std::string &value)
{
    if (map.count(key)) { // if the key already register in map
        return;
    }
    map[key] = value;
}

void AudioEffectManager::ConstructSceneTypeToEffectChainNameMap(std::unordered_map<std::string, std::string> &map)
{
    std::string sceneType;
    std::string sceneMode;
    std::string key;
    for (auto &scene: supportedEffectConfig_.postProcessNew.stream) {
        sceneType = scene.scene;
        for (auto &mode: scene.streamEffectMode) {
            sceneMode = mode.mode;
            for (auto &device: mode.devicePort) {
                key = sceneType + "_&_" + sceneMode + "_&_" + device.type;
                AddKeyValueIntoMap(map, key, device.chain);
            }
        }
    }
    AUDIO_INFO_LOG("Constructed SceneTypeAndModeToEffectChainNameMap at policy, size is %{public}d",
        (int32_t)map.size());
}

void AudioEffectManager::ConstructSceneTypeToEnhanceChainNameMap(std::unordered_map<std::string, std::string> &map)
{
    std::string sceneType;
    std::string sceneMode;
    std::string key;
    for (auto &scene: supportedEffectConfig_.preProcessNew.stream) {
        sceneType = scene.scene;
        for (auto &mode: scene.streamEffectMode) {
            sceneMode = mode.mode;
            for (auto &device: mode.devicePort) {
                key = sceneType + "_&_" + sceneMode;
                AddKeyValueIntoMap(map, key, device.chain);
            }
        }
    }
    AUDIO_INFO_LOG("Constructed SceneTypeAndModeToEnhanceChainNameMap at policy, size is %{public}d",
        (int32_t)map.size());
}
int32_t AudioEffectManager::GetSupportedAudioEffectProperty(const DeviceType &deviceType,
    AudioEffectPropertyArray &propertyArray)
{
    return AUDIO_OK;
}
int32_t AudioEffectManager::GetSupportedAudioEnhanceProperty(const DeviceType &deviceType,
    AudioEnhancePropertyArray &propertyArray)
{
    return AUDIO_OK;
}
} // namespce AudioStandard
} // namespace OHOS
