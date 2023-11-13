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

#ifndef I_STREAM_MANAGER_H
#define I_STREAM_MANAGER_H

#include "i_renderer_stream.h"
#include "i_capturer_stream.h"

namespace OHOS {
namespace AudioStandard {
enum ManagerType : int32_t {
    PLAYBACK =  0,
    RECORDER,
};

class IStreamManager {
public:
    ~IStreamManager() = default;
    
    static IStreamManager &GetPlaybackManager();
    static IStreamManager &GetRecorderManager();

    virtual int32_t CreateRender(AudioStreamParams params, AudioStreamType audioType,
        std::shared_ptr<IRendererStream> &stream) = 0;
    virtual int32_t ReleaseRender(uint32_t streamIndex_) = 0;
    virtual int32_t CreateCapturer(AudioStreamParams params, AudioStreamType audioType,
        std::shared_ptr<ICapturerStream> &stream) = 0;
    virtual int32_t ReleaseCapturer(uint32_t streamIndex_) = 0;
    virtual int32_t GetInfo() = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STREAM_MANAGER_H
