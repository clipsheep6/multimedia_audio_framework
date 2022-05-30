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

#include "audio_capturer_state_change_listener_proxy.h"
#include "audio_errors.h"
#include "audio_renderer_state_change_listener_proxy.h"
#include "audio_stream_collector.h"
#include "i_standard_renderer_state_change_listener.h"
#include "i_standard_capturer_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
AudioStreamCollector::AudioStreamCollector() : mDispatcherService
    (AudioStreamEventDispatcher::GetAudioStreamEventDispatcher())
{
    AUDIO_INFO_LOG("AudioStreamCollector::AudioStreamCollector()");
}

AudioStreamCollector::~AudioStreamCollector()
{
    AUDIO_INFO_LOG("AudioStreamCollector::~AudioStreamCollector()");
}

int32_t AudioStreamCollector::RegisterAudioRendererEventListener(int32_t clientUID, const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("AudioStreamCollector: RegisterAudioRendererEventListener client id %{public}d done", clientUID);
    std::lock_guard<std::mutex> lock(rendererStateChangeEventMutex_);

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "AudioStreamCollector:set renderer state change event listner object is nullptr");

    sptr<IStandardRendererStateChangeListener> listener = iface_cast<IStandardRendererStateChangeListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "AudioStreamCollector: renderer listener obj cast failed");

    std::shared_ptr<AudioRendererStateChangeCallback> callback =
         std::make_shared<AudioRendererStateChangeListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "AudioStreamCollector: failed to  create cb obj");

    mDispatcherService.addRendererListener(clientUID, callback); 
    return SUCCESS;
}

int32_t AudioStreamCollector::UnregisterAudioRendererEventListener(int32_t clientUID)
{
    AUDIO_INFO_LOG("AudioStreamCollector::UnregisterAudioRendererEventListener()");
    mDispatcherService.removeRendererListener(clientUID);
    return SUCCESS;
}

int32_t AudioStreamCollector::RegisterAudioCapturerEventListener(int32_t clientUID, const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("AudioStreamCollector: RegisterAudioCapturerEventListener for client id %{public}d done", clientUID);
    std::lock_guard<std::mutex> lock(capturerStateChangeEventMutex_);

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "AudioStreamCollector:set capturer event listner object is nullptr");

    sptr<IStandardCapturerStateChangeListener> listener = iface_cast<IStandardCapturerStateChangeListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "AudioStreamCollector: capturer obj cast failed");

    std::shared_ptr<AudioCapturerStateChangeCallback> callback =
        std::make_shared<AudioCapturerStateChangeListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "AudioStreamCollector: failed to create capturer cb obj");

    mDispatcherService.addCapturerListener(clientUID, callback); 
    return SUCCESS;
}

int32_t AudioStreamCollector::UnregisterAudioCapturerEventListener(int32_t clientUID)
{
    AUDIO_INFO_LOG("AudioStreamCollector: UnregisterAudioCapturerEventListener client id %{public}d done", clientUID);
    mDispatcherService.removeCapturerListener(clientUID);
    return SUCCESS;
}

int32_t AudioStreamCollector::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        AUDIO_INFO_LOG("AudioStreamCollector: RegisterTracker playback client id %{public}d ",
        streamChangeInfo.audioRendererChangeInfo.clientUID);
        rendererStatequeue_[streamChangeInfo.audioRendererChangeInfo.clientUID] = streamChangeInfo.audioRendererChangeInfo.rendererState;
        unique_ptr<AudioRendererChangeInfo> RendererChangeInfo = make_unique<AudioRendererChangeInfo>();
        RendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
        RendererChangeInfo->sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
        RendererChangeInfo->rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
        RendererChangeInfo->rendererInfo = streamChangeInfo.audioRendererChangeInfo.rendererInfo;
        audioRendererChangeInfos_.push_back(move(RendererChangeInfo));
        mDispatcherService.SendRendererInfoEventToDispatcher(mode, audioRendererChangeInfos_);
        return SUCCESS;
    }

    // mode = AUDIO_MODE_RECORD
    AUDIO_INFO_LOG("AudioStreamCollector: RegisterTracker recording client id %{public}d ",
        streamChangeInfo.audioCapturerChangeInfo.clientUID);
    capturerStatequeue_[streamChangeInfo.audioCapturerChangeInfo.clientUID] =
        streamChangeInfo.audioCapturerChangeInfo.capturerState;
    unique_ptr<AudioCapturerChangeInfo> CapturerChangeInfo = make_unique<AudioCapturerChangeInfo>();
    CapturerChangeInfo->clientUID = streamChangeInfo.audioCapturerChangeInfo.clientUID;
    CapturerChangeInfo->sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    CapturerChangeInfo->capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    CapturerChangeInfo->capturerInfo = streamChangeInfo.audioCapturerChangeInfo.capturerInfo;
    audioCapturerChangeInfos_.push_back(move(CapturerChangeInfo));
    mDispatcherService.SendCapturerInfoEventToDispatcher(mode, audioCapturerChangeInfos_);
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    // update the stream change info
    if (mode == AUDIO_MODE_PLAYBACK) {
        AUDIO_INFO_LOG("AudioStreamCollector: RegisterTracker playback client id %{public}d state %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.rendererState);
        if (streamChangeInfo.audioRendererChangeInfo.rendererState == rendererStatequeue_[streamChangeInfo.audioRendererChangeInfo.clientUID])
        {
            // Renderer state not changed
            AUDIO_DEBUG_LOG("AudioStreamCollector: UpdateTracker playback state not changed");
            return SUCCESS; 
        }
        // Update the renderer info in vector
        for(auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
            AudioRendererChangeInfo audioRendererChangeInfo = **it;
            if (audioRendererChangeInfo.clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID) {
                if (audioRendererChangeInfo.rendererState == RENDERER_RELEASED) {
                    audioRendererChangeInfos_.erase(it);
                } else {
                    audioRendererChangeInfo.rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
                }
            }
        }

        mDispatcherService.SendRendererInfoEventToDispatcher(mode, audioRendererChangeInfos_);
    }

    // mode = AUDIO_MODE_RECORD
    AUDIO_INFO_LOG("AudioStreamCollector: UpdateTracker recording client id %{public}d state %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.capturerState);
    if (streamChangeInfo.audioCapturerChangeInfo.capturerState == capturerStatequeue_[streamChangeInfo.audioRendererChangeInfo.clientUID])
    {
        // Capturer state not changed
        AUDIO_DEBUG_LOG("AudioStreamCollector: UpdateTracker recording state not changed");
        return SUCCESS; 
    }
    for(auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfo = **it;
        if (audioCapturerChangeInfo.clientUID == streamChangeInfo.audioCapturerChangeInfo.clientUID) {
            if (audioCapturerChangeInfo.capturerState == CAPTURER_RELEASED) {
                audioCapturerChangeInfos_.erase(it);
            } else {
                audioCapturerChangeInfo.capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
            }
        }
    }

    mDispatcherService.SendCapturerInfoEventToDispatcher(mode, audioCapturerChangeInfos_);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS