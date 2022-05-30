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
 
#ifndef AUDIO_STREAM_TRACKER_H
#define AUDIO_STREAM_TRACKER_H

//#include "audio_stream.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {

//constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);

class AudioStreamTracker {
public:
    void RegisterTracker(int32_t sessionId, State state,
        AudioRendererInfo &rendererInfo, AudioCapturerInfo &capturerInfo);
    void UpdateTracker(int32_t sessionId, State state,
        AudioRendererInfo &rendererInfo, AudioCapturerInfo &capturerInfo);

private:
    int32_t clientUID_ = -1;
    uint32_t sessionId_ = INVALID_SESSION_ID;
    AudioMode eMode_; // to determine renderer or capturer
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_TRACKER_H
