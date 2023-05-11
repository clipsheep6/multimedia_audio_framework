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

#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_log.h"
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioStreamManager *AudioStreamManager::GetInstance()
{
    static AudioStreamManager audioStreamManager;
    return &audioStreamManager;
}

int32_t AudioStreamManager::RegisterAudioRendererEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioRendererEventListener client id: %{public}d", clientPid);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("callback is null");
        return ERR_INVALID_PARAM;
    }
    return AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(clientPid, callback);
}

int32_t AudioStreamManager::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    AUDIO_INFO_LOG("UnregisterAudioRendererEventListener client id: %{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(clientPid);
}

int32_t AudioStreamManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioCapturerEventListener client id: %{public}d", clientPid);
    if (callback == nullptr) {
        AUDIO_ERR_LOG("callback is null");
        return ERR_INVALID_PARAM;
    }
    return AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientPid, callback);
}

int32_t AudioStreamManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    AUDIO_INFO_LOG("UnregisterAudioCapturerEventListener client id: %{public}d", clientPid);
    return AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientPid);
}

int32_t AudioStreamManager::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioStreamManager::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    return AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

bool AudioStreamManager::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_DEBUG_LOG("IsAudioRendererLowLatencySupported");
    return AudioPolicyManager::GetInstance().IsAudioRendererLowLatencySupported(audioStreamInfo);
}

static void UpdateEffectInfoArray(vector<unique_ptr<AudioSceneEffectInfo>> &audioSceneEffectInfo,
    SupportedEffectConfig &supportedEffectConfig, AudioSceneEffectInfo &audioSceneEffectInfoTmp, int isSupported, int i)
{
    int j;
    if (isSupported == 1) {
        for (j = 0; j < supportedEffectConfig.preProcessNew.stream[i].streamAE_mode.size(); j++) {
            if (supportedEffectConfig.preProcessNew.stream[i].streamAE_mode[j].mode == "EFFECT_NONE") {
                audioSceneEffectInfoTmp.mode.push_back(EFFECT_NONE);
            } else if (supportedEffectConfig.preProcessNew.stream[i].streamAE_mode[j].mode == "EFFECT_DEFAULT") {
                audioSceneEffectInfoTmp.mode.push_back(EFFECT_DEFAULT);
            } else {
                AUDIO_INFO_LOG("[supportedEffectConfig LOG10]:mode-> The %{public}s mode of %{public}s scene is \
                    unsupported, and this mode is setted to EFFECT_NONE!",
                    supportedEffectConfig.preProcessNew.stream[i].streamAE_mode[j].mode.c_str(),
                    supportedEffectConfig.preProcessNew.stream[i].scene.c_str());
                audioSceneEffectInfoTmp.mode.push_back(EFFECT_NONE);
            }
        }
        audioSceneEffectInfo.push_back(make_unique<AudioSceneEffectInfo>(audioSceneEffectInfoTmp));
    }
}

int32_t AudioStreamManager::GetEffectInfoArray(vector<unique_ptr<AudioSceneEffectInfo>> &audioSceneEffectInfo)
{
    int i, isSupported;
    SupportedEffectConfig supportedEffectConfig;
    int ret = AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    int streamNum = supportedEffectConfig.preProcessNew.stream.size();
    if (streamNum > 0) {
        for (i = 0; i < streamNum; i++) {
            isSupported = 1;
            AudioSceneEffectInfo audioSceneEffectInfoTmp;
            if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_MUSIC") {
                audioSceneEffectInfoTmp.scene = SCENE_MUSIC;
            } else if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_MOVIE") {
                audioSceneEffectInfoTmp.scene = SCENE_MOVIE;
            } else if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_GAME") {
                audioSceneEffectInfoTmp.scene = SCENE_GAME;
            } else if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_SPEECH") {
                audioSceneEffectInfoTmp.scene = SCENE_SPEECH;
            } else if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_RING") {
                audioSceneEffectInfoTmp.scene = SCENE_RING;
            } else if (supportedEffectConfig.preProcessNew.stream[i].scene == "SCENE_OTHERS") {
                audioSceneEffectInfoTmp.scene = SCENE_OTHERS;
            } else {
                AUDIO_INFO_LOG("[supportedEffectConfig LOG9]:stream-> The scene of %{public}s is unsupported, \
                    and this scene is deleted!", supportedEffectConfig.preProcessNew.stream[i].scene.c_str());
                isSupported = -1;
            }
            UpdateEffectInfoArray(audioSceneEffectInfo, supportedEffectConfig, audioSceneEffectInfoTmp, isSupported, i);
        }
    }
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
