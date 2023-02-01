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

#include "native_audiorenderer.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class OHAudioRenderModeCallback : public AudioRendererWriteCallback {
public:
    OHAudioRenderModeCallback(OH_AudioRendererCallbacks callbacks, void* userData)
    {
        callbacks_ = callbacks;
        userData_ = userData;
    }
    void OnWriteData(size_t length) override;
private:
    OH_AudioRendererCallbacks callbacks_;
    void* userData_;
};

class OHAudioRenderer {
    public:
        OHAudioRenderer()
        {
            ContentType contentType = ContentType::CONTENT_TYPE_MUSIC;
            StreamUsage streamUsage = StreamUsage::STREAM_USAGE_MEDIA;

            AudioRendererOptions rendererOptions = {};
            rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
            rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
            rendererOptions.streamInfo.format = SAMPLE_S16LE;
            rendererOptions.streamInfo.channels = STEREO;
            rendererOptions.rendererInfo.contentType = contentType;
            rendererOptions.rendererInfo.streamUsage = streamUsage;
            rendererOptions.rendererInfo.rendererFlags = 0;

            audioRenderer_ = AudioRenderer::Create(rendererOptions);
        }
        int32_t Start();
        int32_t Pause();
        int32_t Stop();
        int32_t Flush();
        int32_t Release();
        RendererState GetStatus();
        AudioRendererRate GetRenderRate();
        void GetStreamId(uint32_t& streamId);
        int32_t GetChannelCount();
        int32_t GetLatencyMode();
        StreamUsage GetStreamUsage();
        ContentType GetContentType();
        int32_t GetFramesWritten();

        void SetRendererWriteCallback(OH_AudioRendererCallbacks callbacks, void* userData);
    private:
        std::unique_ptr<AudioRenderer> audioRenderer_;
};
}  // namespace AudioStandard
}  // namespace OHOS
