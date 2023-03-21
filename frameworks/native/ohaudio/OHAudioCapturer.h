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

#include "native_audiocapturer.h"
#include "audio_capturer.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioCapturer {
    public:
        OHAudioCapturer()
        {
        //    AudioRendererOptions rendererOptions = {};
        //    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
        //    rendererOptions.rendererInfo.rendererFlags = 0;

        //    audioRenderer_ = AudioRenderer::Create(rendererOptions);
        }
        int32_t Start();
        int32_t Pause();
        int32_t Stop();
        int32_t Flush();
        int32_t Release();
    private:
        std::unique_ptr<AudioCapturer> audioCapturer_;
};
}  // namespace AudioStandard
}  // namespace OHOS
