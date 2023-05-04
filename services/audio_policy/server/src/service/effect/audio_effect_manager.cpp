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

#include "audio_effect_manager.h"
#include <unordered_set>

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
    if (ret != 0) {
        AUDIO_ERR_LOG("AudioEffectManager->effectConfigParser failed: %{public}d", ret);
        return;
    }
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

int32_t AudioEffectManager::UpdateUnsupportedScene(std::string &scene)
{
    int isSupported = 0;
    if ((scene != "SCENE_MUSIC") &&
        (scene != "SCENE_MOVIE") &&
        (scene != "SCENE_GAME") &&
        (scene != "SCENE_SPEECH") &&
        (scene != "SCENE_RING") &&
        (scene != "SCENE_OTHERS")) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG9]:stream-> The scene of %{public}s is unsupported, and this scene is deleted!", scene.c_str());
        isSupported = -1;
    }
    return isSupported;
}

void AudioEffectManager::UpdateUnsupportedMode(Preprocess &pp, Stream &stream, std::string &mode, int i)
{
    int j;
    int isSupported = 0;
    if ((mode != "EFFECT_NONE") &&
        (mode != "PLAYBACK_DEFAULT")) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG9]:mode-> The %{public}s mode of %{public}s is unsupported, and this mode is deleted!", mode.c_str(), stream.scene.c_str());
        isSupported = -1;
    }
    if (isSupported == 0) {
        StreamAE_mode streamAE_mode;
        streamAE_mode.mode = mode;
        j = 0;
        for(auto &device: pp.device) {
            if (i == j) {
                for(auto &a: device) {
                    streamAE_mode.devicePort.push_back(a);
                }
                break;
            }
            j += 1;
        }
        i += 1;
        stream.streamAE_mode.push_back(streamAE_mode);
    }
}

int32_t AudioEffectManager::UpdateAvailableStream(ProcessNew &preProcessNew, Preprocess &pp)
{
    int i;
    int32_t isDuplicate = 0;
    int32_t isSupported;
    isSupported = UpdateUnsupportedScene(pp.stream);
    auto it = std::find_if(preProcessNew.stream.begin(), preProcessNew.stream.end(), [&pp](const Stream& x) {
        return x.scene == pp.stream;
    });
    if ((it == preProcessNew.stream.end()) && (isSupported == 0))
    {
        Stream stream;
        stream.scene = pp.stream;
        i = 0;
        for(auto &mode: pp.mode) {
            UpdateUnsupportedMode(pp, stream, mode, i);
        }
        preProcessNew.stream.push_back(stream);
    } else if (it != preProcessNew.stream.end()) {
        isDuplicate = 1;
    }
    return isDuplicate;
}

void AudioEffectManager::UpdateEffectChains(std::vector<std::string> &availableLayout)
{
    int count = 0;
    std::vector<int> delIdx;
    for(auto &ec: supportedEffectConfig_.effectChains) {
        for(auto &effectName: ec.apply) {
            auto it = std::find_if(availableEffects_.begin(), availableEffects_.end(), [&effectName](const Effect& effect) {
                return effect.name == effectName;
            });
            if (it == availableEffects_.end()) {
                delIdx.emplace_back(count);
                break;
            }
        }
        count += 1;
    }
    for (auto it = delIdx.rbegin(); it != delIdx.rend(); ++it) {
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
    int32_t isDuplicate = 0;
    int32_t ret;
    supportedEffectConfig_.effectChains = aeConfig.effectChains;
    ProcessNew preProcessNew;
    for (Preprocess &pp: aeConfig.preprocess) {
        ret = UpdateAvailableStream(preProcessNew, pp);
        if (ret == 1) {
            isDuplicate = 1;
        }
    }
    ProcessNew postProcessNew;
    for (Preprocess &pp: aeConfig.preprocess) {
        ret = UpdateAvailableStream(postProcessNew, pp);
        if (ret == 1) {
            isDuplicate = 1;
        }
    }
    if (isDuplicate == 1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG2]:stream-> The duplicate stream is deleted, and the first configuration is retained!");
    }
    supportedEffectConfig_.preProcessNew = preProcessNew;
    supportedEffectConfig_.postProcessNew = postProcessNew;
}

// 重复配置EFFECT_NONE，删除此mode, log
void AudioEffectManager::UpdateDuplicateBypassMode(ProcessNew &preProcessNew)
{
    int flag, count;
    flag = 0;
    std::vector<int> delIdx;
    for(auto &stream: preProcessNew.stream) {
        count = 0;
        delIdx.clear();
        for (auto &streamAE_mode: stream.streamAE_mode) {
            if (streamAE_mode.mode == "EFFECT_NONE") {
                delIdx.push_back(count);
            }
            count += 1;
        }
        for (auto it = delIdx.rbegin(); it != delIdx.rend(); ++it) {
            stream.streamAE_mode[*it].devicePort = {};
            flag = -1;
        }
    }
    if (flag == -1) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG3]:mode-> EFFECT_NONE can not configure by deveploer!");
    }
}

// 再去每个stream下mode重复的配置
void AudioEffectManager::UpdateDuplicateMode(ProcessNew &preProcessNew)
{
    std::unordered_set<std::string> seen;
    std::vector<int> to_remove;
    uint32_t i;
    for(auto &stream: preProcessNew.stream) { 
        seen.clear();
        to_remove.clear();
        for (i = 0; i < stream.streamAE_mode.size(); i++) {
            if (seen.count(stream.streamAE_mode[i].mode)) {
                to_remove.push_back(i);
            } else {
                seen.insert(stream.streamAE_mode[i].mode);
            }
        }
        for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG4]:mode-> The duplicate mode of %{public}s configuration is deleted, and the first configuration is retained!", stream.scene.c_str());
            stream.streamAE_mode.erase(stream.streamAE_mode.begin() + *it);
        }
    }
}

// 再去每个mode下devicePort重复的配置
void AudioEffectManager::UpdateDuplicateDevice(ProcessNew &preProcessNew)
{
    std::unordered_set<std::string> seen;
    std::vector<int> to_remove;
    uint32_t i;
    for(auto &stream: preProcessNew.stream) { 
        for (auto &streamAE_mode: stream.streamAE_mode) {
            seen.clear();
            to_remove.clear();
            for (i = 0; i < streamAE_mode.devicePort.size(); i++) {
                if (seen.count(streamAE_mode.devicePort[i].type)) {
                    to_remove.push_back(i);
                } else {
                    seen.insert(streamAE_mode.devicePort[i].type);
                }
            }
            for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it) {
                AUDIO_INFO_LOG("[supportedEffectConfig LOG5]:device-> The duplicate device of %{public}s's %{public}s mode configuration is deleted, and the first configuration is retained!", stream.scene.c_str(), streamAE_mode.mode.c_str());
                streamAE_mode.devicePort.erase(streamAE_mode.devicePort.begin() + *it);
            }
        }
    }
}

int32_t AudioEffectManager::UpdateUnavailableEffectChains(std::vector<std::string> &availableLayout, ProcessNew &preProcessNew)
{
    int count, count1;
    int ret = 0;
    std::vector<int> delIdx;
    std::vector<int> delIdx1;
    for(auto &stream: preProcessNew.stream) { 
        delIdx1.clear();
        count1 = 0;
        for (auto &streamAE_mode: stream.streamAE_mode) {
            delIdx.clear();
            count = 0;
            for (auto &devicePort: streamAE_mode.devicePort) {
                auto index = std::find(availableLayout.begin(), availableLayout.end(), devicePort.chain);
                if (index == availableLayout.end()) {
                    delIdx.push_back(count);
                }
                count += 1;
            }
            if (streamAE_mode.devicePort.size() != delIdx.size() && delIdx.size() != 0) { // 如果不是所有的device都要删除
                AUDIO_INFO_LOG("[supportedEffectConfig LOG6]:device-> The unavailable effectChain of %{public}s's %{public}s mode are set to LAYOUT_BYPASS!", stream.scene.c_str(), streamAE_mode.mode.c_str());
                for (auto it = delIdx.rbegin(); it != delIdx.rend(); ++it) {
                    streamAE_mode.devicePort[*it].chain = "LAYOUT_BYPASS";
                }
            } else { // 如果是所有的device都要删除，就删除mode
                for (auto it = delIdx.rbegin(); it != delIdx.rend(); ++it) {
                    streamAE_mode.devicePort.erase(streamAE_mode.devicePort.begin() + *it);
                    if (streamAE_mode.devicePort.empty()) {
                        delIdx1.push_back(count1);
                    }
                }
            }
            count1 += 1;
        }
        for (auto it = delIdx1.rbegin(); it != delIdx1.rend(); ++it) {
            AUDIO_INFO_LOG("[supportedEffectConfig LOG7]:mode-> %{public}s's %{public}s mode is deleted!", stream.scene.c_str(), stream.streamAE_mode[*it].mode.c_str());
            if (stream.streamAE_mode[*it].mode == "PLAYBACK_DEAFULT") { // 最后查看如果mode里已经没有了PLAYBACK_DEAFULT，要返回错误码并提示
                ret = -1;
            }
            stream.streamAE_mode.erase(stream.streamAE_mode.begin() + *it);
            if(stream.streamAE_mode.empty()) {
                AUDIO_INFO_LOG("[supportedEffectConfig LOG8]:mode-> %{public}s's mode is only EFFECT_NONE!", stream.scene.c_str());
                StreamAE_mode streamAE_mode;
                streamAE_mode.mode = "EFFECT_NONE";
                stream.streamAE_mode.push_back(streamAE_mode);
            }
        }
    }
    return ret;
}

void AudioEffectManager::GetAvailableAEConfig()
{   
    int32_t ret;
    std::vector<std::string> availableLayout; // 有效的effectlayout名字
    existDefault_ = 1;
    if (oriEffectConfig_.effectChains.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG12]: effectChains is none!");
    }

    if (oriEffectConfig_.effectChains.size() == 0 || oriEffectConfig_.preprocess.size() == 0 || oriEffectConfig_.postprocess.size() == 0) {
        AUDIO_INFO_LOG("[supportedEffectConfig LOG11]: process is none!");
        return;
    }

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
    ret = UpdateUnavailableEffectChains(availableLayout, supportedEffectConfig_.preProcessNew);
    if (ret != 0) {
        existDefault_ = -1;
    }
}

} // namespce AudioStandard
} // namespace OHOS
  