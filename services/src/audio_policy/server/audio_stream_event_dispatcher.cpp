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

#include <memory>
#include "audio_stream_event_dispatcher.h"
#include "i_standard_capturer_state_change_listener.h"
#include "i_standard_renderer_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
AudioStreamEventDispatcher::AudioStreamEventDispatcher() : activeThread_(1)
{
    AUDIO_INFO_LOG("AudioStreamEventDispatcher::AudioStreamEventDispatcher()");
}

AudioStreamEventDispatcher::~AudioStreamEventDispatcher()
{
    AUDIO_INFO_LOG("AudioStreamEventDispatcher::~AudioStreamEventDispatcher()");
}

void AudioStreamEventDispatcher::addRendererListener(int32_t clientUID, 
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    rendererCBMap_[clientUID] = callback;
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::addRendererListener:client %{public}d added", clientUID);
}

void AudioStreamEventDispatcher::removeRendererListener(int32_t clientUID)
{
// std::lock_guard<std::mutex> lock(volumeKeyEventMutex_); check why and if this mutex needed for this also?
// this check for both add and remove listners..in stream_colletor its there.currently..based on role decide
    if (rendererCBMap_.erase(clientUID)) {
        AUDIO_INFO_LOG("AudioStreamEventDispatcher::removeRendererListener:client %{public}d done", clientUID);
        return;
    }
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::removeRendererListener:client %{public}d not present", clientUID);
}

void AudioStreamEventDispatcher::addCapturerListener(int32_t clientUID, 
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    capturerCBMap_[clientUID] = callback;
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::addCapturerListener:client %{public}d added", clientUID);
}

void AudioStreamEventDispatcher::removeCapturerListener(int32_t clientUID)
{
    if (capturerCBMap_.erase(clientUID)) {
        AUDIO_INFO_LOG("AudioStreamEventDispatcher::removeCapturerListener:client %{public}d done", clientUID);
        return;
    }
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::removeCapturerListener:client %{public}d not present", clientUID);
}

void AudioStreamEventDispatcher::SendRendererInfoEventToDispatcher(AudioMode &mode,
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{ 
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::SendRendererInfoEventToDispatcher:mode %{public}d ", mode);
    unique_ptr<StreamStateChangeRequest> streamStateChangeRequest = make_unique<StreamStateChangeRequest>();
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    for (const auto &e : audioRendererChangeInfos) rendererChangeInfos.push_back(std::make_unique<AudioRendererChangeInfo>(*e));
    streamStateChangeRequest->mode = mode;
    streamStateChangeRequest->audioRendererChangeInfos = move(rendererChangeInfos);
    streamStateChangeQueue_.push(move(streamStateChangeRequest));
    DispatcherEvent(); 
}

void AudioStreamEventDispatcher::SendCapturerInfoEventToDispatcher(AudioMode &mode,
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_DEBUG_LOG("AudioStreamEventDispatcher::SendCapturerInfoEventToDispatcher:mode %{public}d ", mode);
    unique_ptr<StreamStateChangeRequest> streamStateChangeRequest = make_unique<StreamStateChangeRequest>();
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    for (const auto &e : audioCapturerChangeInfos) capturerChangeInfos.push_back(std::make_unique<AudioCapturerChangeInfo>(*e));
    streamStateChangeRequest->mode = mode;
    streamStateChangeRequest->audioCapturerChangeInfos = move(capturerChangeInfos);
    streamStateChangeQueue_.push(move(streamStateChangeRequest));
    DispatcherEvent();
}

void AudioStreamEventDispatcher::HandleStreamStateChange()
{
    AUDIO_DEBUG_LOG("HandleStreamStateChange entered");
    while (!streamStateChangeQueue_.empty()) {
        std::unique_ptr<StreamStateChangeRequest> streamStateChangeRequest = std::move(streamStateChangeQueue_.front());
        if (streamStateChangeRequest->mode == AUDIO_MODE_PLAYBACK) {
            for (auto it = rendererCBMap_.begin(); it != rendererCBMap_.end(); ++it) {
                std::shared_ptr<AudioRendererStateChangeCallback> rendererStateChangeCb = it->second;
                if (rendererStateChangeCb == nullptr) {
                    AUDIO_ERR_LOG("rendererStateChangeCb : nullptr for client : %{public}d", it->first);
                    continue;
                }
                AUDIO_DEBUG_LOG("rendererStateChangeCb : client = %{public}d", it->first);
                rendererStateChangeCb->OnRendererStateChange(streamStateChangeRequest->audioRendererChangeInfos);
            }
        } else {
            for (auto it = capturerCBMap_.begin(); it != capturerCBMap_.end(); ++it) {
                std::shared_ptr<AudioCapturerStateChangeCallback> capturerStateChangeCb = it->second;
                if (capturerStateChangeCb == nullptr) {
                    AUDIO_ERR_LOG("capturerStateChangeCb : nullptr for client : %{public}d", it->first);
                    continue;
                }
                AUDIO_DEBUG_LOG("capturerStateChangeCb : client = %{public}d", it->first);
                capturerStateChangeCb->OnCapturerStateChange(streamStateChangeRequest->audioCapturerChangeInfos);
            }
        }
        streamStateChangeQueue_.pop(); 
    }
    activeThread_ --;  
}

void AudioStreamEventDispatcher::DispatcherEvent()
{
    AUDIO_DEBUG_LOG("DispatcherEvent entered");
    size_t c = MAX_THREAD - activeThread_;
    for (size_t i = 0; i <c ; i++) {
        if (!streamStateChangeQueue_.empty() && (activeThread_ < streamStateChangeQueue_.size())) {
            std::thread(&AudioStreamEventDispatcher::HandleStreamStateChange, this).detach();
            activeThread_++;
        } 
    }
}
} // namespace AudioStandard
} // namespace OHOS