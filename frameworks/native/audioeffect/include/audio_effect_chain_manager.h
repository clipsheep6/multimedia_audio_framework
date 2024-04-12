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


#ifndef AUDIO_EFFECT_CHAIN_MANAGER_H
#define AUDIO_EFFECT_CHAIN_MANAGER_H

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <set>

#include "audio_effect.h"
#include "audio_effect_chain.h"

#ifdef SENSOR_ENABLE
#include "audio_head_tracker.h"
#endif
#include "audio_effect_hdi_param.h"
#ifdef WINDOW_MANAGER_ENABLE
#include "audio_effect_rotation.h"
#endif
#include "audio_effect_volume.h"

namespace OHOS {
namespace AudioStandard {

const uint32_t DEFAULT_FRAMELEN = 1440;
const uint32_t DEFAULT_NUM_CHANNEL = STEREO;
const uint32_t DEFAULT_MCH_NUM_CHANNEL = CHANNEL_6;
const uint64_t DEFAULT_NUM_CHANNELLAYOUT = CH_LAYOUT_STEREO;
const uint64_t DEFAULT_MCH_NUM_CHANNELLAYOUT = CH_LAYOUT_5POINT1;
const uint32_t BASE_TEN = 10;
const std::string DEFAULT_DEVICE_SINK = "Speaker";
const uint32_t SIZE_OF_SPATIALIZATION_STATE = 2;
const uint32_t HDI_ROOM_MODE_INDEX_TWO = 2;

struct sessionEffectInfo {
    std::string sceneMode;
    std::string sceneType;
    uint32_t channels;
    uint64_t channelLayout;
    std::string spatializationEnabled;
    uint32_t volume;
};

const std::vector<AudioChannelLayout> HVS_SUPPORTED_CHANNELLAYOUTS {
    CH_LAYOUT_STEREO,
    CH_LAYOUT_5POINT1,
    CH_LAYOUT_5POINT1POINT2,
    CH_LAYOUT_7POINT1,
    CH_LAYOUT_5POINT1POINT4,
    CH_LAYOUT_7POINT1POINT2,
    CH_LAYOUT_7POINT1POINT4,
    CH_LAYOUT_9POINT1POINT4,
    CH_LAYOUT_9POINT1POINT6
};

struct effectBufferAttr {
    float *bufIn;
    float *bufOut;
    int numChans;
    int frameLen;

    effectBufferAttr(float *bufIn, float *bufOut, int numChans, int frameLen)
        : bufIn(bufIn), bufOut(bufOut), numChans(numChans), frameLen(frameLen)
    {
    }
};

class AudioEffectChainManager {
public:
    AudioEffectChainManager();
    ~AudioEffectChainManager();
    static AudioEffectChainManager *GetInstance();
    void InitAudioEffectChainManager(std::vector<EffectChain> &effectChains,
        std::unordered_map<std::string, std::string> &map,
        std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList);
    bool CheckAndAddSessionID(const std::string &sessionID);
    int32_t CreateAudioEffectChainDynamic(const std::string &sceneType);
    int32_t SetAudioEffectChainDynamic(const std::string &sceneType, const std::string &effectMode);
    bool CheckAndRemoveSessionID(const std::string &sessionID);
    int32_t ReleaseAudioEffectChainDynamic(const std::string &sceneType);
    bool ExistAudioEffectChain(const std::string &sceneType, const std::string &effectMode,
        const std::string &spatializationEnabled);
    int32_t ApplyAudioEffectChain(const std::string &sceneType, const std::unique_ptr<effectBufferAttr> &bufferAttr);
    int32_t SetOutputDeviceSink(int32_t device, const std::string &sinkName);
    std::string GetDeviceTypeName();
    std::string GetDeviceSinkName();
    int32_t GetFrameLen();
    int32_t SetFrameLen(int32_t frameLen);
    bool GetOffloadEnabled();
    void Dump();
    int32_t UpdateMultichannelConfig(const std::string &sceneType);
    int32_t InitAudioEffectChainDynamic(const std::string &sceneType);
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState);
    int32_t SetHdiParam(const std::string &sceneType, const std::string &effectMode, bool enabled);
    int32_t SessionInfoMapAdd(const std::string &sessionID, const sessionEffectInfo &info);
    int32_t SessionInfoMapDelete(const std::string &sceneType, const std::string &sessionID);
    int32_t ReturnEffectChannelInfo(const std::string &sceneType, uint32_t *channels, uint64_t *channelLayout);
    int32_t ReturnMultiChannelInfo(uint32_t *channels, uint64_t *channelLayout);
    void RegisterEffectChainCountBackupMap(const std::string &sceneType, const std::string &operation);
    int32_t EffectRotationUpdate(const uint32_t rotationState);
    int32_t EffectVolumeUpdate(const std::string sessionIDString, const uint32_t volume);
    uint32_t GetLatency(const std::string &sessionId);
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType);

private:
    void UpdateSensorState();
    void DeleteAllChains();
    void RecoverAllChains();
    int32_t EffectDspVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume);
    int32_t EffectApVolumeUpdate(std::shared_ptr<AudioEffectVolume> audioEffectVolume);
    AudioEffectScene GetSceneTypeFromSpatializationSceneType(AudioEffectScene sceneType);
    void UpdateEffectChainParams(AudioEffectScene sceneType);
#ifdef WINDOW_MANAGER_ENABLE
    int32_t EffectDspRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
        const uint32_t rotationState);
    int32_t EffectApRotationUpdate(std::shared_ptr<AudioEffectRotation> audioEffectRotation,
        const uint32_t rotationState);
#endif
    std::map<std::string, std::shared_ptr<AudioEffectLibEntry>> EffectToLibraryEntryMap_;
    std::map<std::string, std::string> EffectToLibraryNameMap_;
    std::map<std::string, std::vector<std::string>> EffectChainToEffectsMap_;
    std::map<std::string, std::string> SceneTypeAndModeToEffectChainNameMap_;
    std::map<std::string, std::shared_ptr<AudioEffectChain>> SceneTypeToEffectChainMap_;
    std::map<std::string, int32_t> SceneTypeToEffectChainCountMap_;
    std::set<std::string> SessionIDSet_;
    std::map<std::string, std::set<std::string>> SceneTypeToSessionIDMap_;
    std::map<std::string, sessionEffectInfo> SessionIDToEffectInfoMap_;
    std::map<std::string, int32_t> SceneTypeToEffectChainCountBackupMap_;
    uint32_t frameLen_ = DEFAULT_FRAMELEN;
    DeviceType deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string deviceSink_ = DEFAULT_DEVICE_SINK;
    bool isInitialized_ = false;
    std::recursive_mutex dynamicMutex_;
    bool spatializationEnabled_ = false;
    bool headTrackingEnabled_ = false;
    bool offloadEnabled_ = false;
    bool initializedLogFlag_ = true;
    AudioSpatializationSceneType spatializationSceneType_ = SPATIALIZATION_SCENE_TYPE_DEFAULT;
    int32_t hdiSceneType_ = 0;
    int32_t hdiEffectMode_ = 0;

#ifdef SENSOR_ENABLE
    std::shared_ptr<HeadTracker> headTracker_;
#endif

    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam_;
    int8_t effectHdiInput[SEND_HDI_COMMAND_LEN];
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_EFFECT_CHAIN_MANAGER_H
