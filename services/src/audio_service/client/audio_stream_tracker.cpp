/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_stream_tracker.h"

namespace OHOS {
namespace AudioStandard {

AudioStreamTracker::AudioStreamTracker(AudioMode mode, int32_t clientUID)
{
    eMode_ = mode;
    clientUID_ = clientUID;
}

AudioStreamTracker::~AudioStreamTracker() {}

void AudioStreamTracker::RegisterTracker(int32_t sessionId, State state,
        AudioRendererInfo &rendererInfo, AudioCapturerInfo &capturerInfo)
{
    AudioStreamChangeInfo streamChangeInfo;

    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = clientUID_;
        streamChangeInfo.audioRendererChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioRendererChangeInfo.rendererState = static_cast<RendererState>(state);
        streamChangeInfo.audioRendererChangeInfo.rendererInfo = rendererInfo;
    } else 
    {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = clientUID_;
        streamChangeInfo.audioCapturerChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioCapturerChangeInfo.capturerState = static_cast<CapturerState>(state);
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo = capturerInfo;
    }

    AudioPolicyManager::GetInstance().RegisterTracker(eMode_, streamChangeInfo);
}

void AudioStreamTracker::UpdateTracker(int32_t sessionId, State state,
    AudioRendererInfo &rendererInfo, AudioCapturerInfo &capturerInfo) 
{
    AudioStreamChangeInfo streamChangeInfo;

    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        streamChangeInfo.audioRendererChangeInfo.clientUID = clientUID_;
        streamChangeInfo.audioRendererChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioRendererChangeInfo.rendererState = static_cast<RendererState>(state);
        streamChangeInfo.audioRendererChangeInfo.rendererInfo = rendererInfo;
    } else 
    {
        streamChangeInfo.audioCapturerChangeInfo.clientUID = clientUID_;
        streamChangeInfo.audioCapturerChangeInfo.sessionId = sessionId;
        streamChangeInfo.audioCapturerChangeInfo.capturerState = static_cast<CapturerState>(state);
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo = capturerInfo;
    }

    AudioPolicyManager::GetInstance().UpdateTracker(eMode_, streamChangeInfo);
}
} // namespace AudioStandard
} // namespace OHOS
