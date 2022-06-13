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

#ifndef AUDIO_STREAM_COLLECTOR_H
#define AUDIO_STREAM_COLLECTOR_H

#include "audio_info.h"
#include "audio_stream_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {
class AudioStreamCollector {
public:
    static AudioStreamCollector& GetAudioStreamCollector()
    {
        static AudioStreamCollector audioStreamCollector;
        return audioStreamCollector;
    }

    AudioStreamCollector();
    ~AudioStreamCollector();
    int32_t RegisterAudioRendererEventListener(int32_t clientUID, const sptr<IRemoteObject> &object);
    int32_t UnregisterAudioRendererEventListener(int32_t clientUID);
    int32_t RegisterAudioCapturerEventListener(int32_t clientUID, const sptr<IRemoteObject> &object);
    int32_t UnregisterAudioCapturerEventListener(int32_t clientUID);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo, const sptr<IRemoteObject> &object);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    int32_t GetCurrentRendererChangeInfos(vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos);
    int32_t GetCurrentCapturerChangeInfos(vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos);
    void RegisteredClientDied(int32_t uid);

private:
    AudioStreamEventDispatcher &mDispatcherService;
    std::mutex rendererStateChangeEventMutex_;
    std::mutex capturerStateChangeEventMutex_;
    std::unordered_map<int32_t, int32_t> rendererStatequeue_;
    std::unordered_map<int32_t, int32_t> capturerStatequeue_;
    std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos_;
    std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos_;
    std::unordered_map<int32_t, std::shared_ptr<AudioClientTracker>> clientTracker_;
	int32_t AddRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t AddCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateRendererStream(AudioStreamChangeInfo &streamChangeInfo);
    int32_t UpdateCapturerStream(AudioStreamChangeInfo &streamChangeInfo);
    void DisplayInternalStreamInfo();
};
} // namespace AudioStandard
} // namespace OHOS
#endif