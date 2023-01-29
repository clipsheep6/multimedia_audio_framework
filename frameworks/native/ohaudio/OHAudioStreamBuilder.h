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

#include "native_audiostream_base.h"
#include <stdint.h>
#include "audio_info.h"

static const int32_t RENDERER_TYPE = 1;
static const int32_t CAPTURER_TYPE = 2;

namespace OHOS {
namespace AudioStandard {
class OHAudioStreamBuilder
{
    public:
        OHAudioStreamBuilder(int32_t type)
        {
            streamType_ = type;
        }

        int32_t Generate(OH_AudioRenderer* renderer);

        int32_t Generate(OH_AudioCapturer* capturer);

        // for audiorenderer setting

        /**
         * Set the sample rate
         *
         * @param rate The sample rate value that to be set.
         */
        void SetSamplingRate(int32_t rate);

        /**
         * Set the channel size.
         *
         * @param channelCount The channel size value that to be set.
         * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
         */
        OH_AudioStream_Result SetChannelCount(int32_t channelCount);
        OH_AudioStream_Result SetSampleFormat(AudioSampleFormat sampleFormat);
        OH_AudioStream_Result SetRendererInfo(StreamUsage usage, ContentType contentType);
        OH_AudioStream_Result SetEncodingType(AudioEncodingType encodingType);

    private:
        int32_t streamType_;
        int32_t samplingRate_ = 0;
        int32_t channelCount_ = 0;
        AudioSampleFormat sampleFormat_;
        StreamUsage usage_;
        ContentType contentType_;
        AudioEncodingType encodingType_;
};

}  // namespace AudioStandard
}  // namespace OHOS
//OH_AudioStream_Result OH_AudioStreamBuilder_SetChannelCount(OH_AudioStreamBuilder* builder, int32_t channelCount);

//OH_AudioStream_Result OH_AudioStreamBuilder_SetSampleFormat(OH_AudioStreamBuilder* builder,
//        OH_AudioStream_SampleFormat format);
//
//OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererInfo(OH_AudioStreamBuilder* builder,
//        OH_AudioStream_Usage usage, OH_AudioStream_Content content);
//
//OH_AudioStream_Result OH_AudioStreamBuilder_SetRncodingType(OH_AudioStreamBuilder* builder,
//        OH_AudioStream_EncodingType encodingType);
