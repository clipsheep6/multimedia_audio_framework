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

#include "OHAudioStreamBuilder.h"
#include <memory>
#include "OHAudioRenderer.h"

using OHOS::AudioStandard::OHAudioStreamBuilder;
using OHOS::AudioStandard::AudioSampleFormat;
using OHOS::AudioStandard::StreamUsage;
using OHOS::AudioStandard::AudioEncodingType;
using OHOS::AudioStandard::ContentType;

static OHOS::AudioStandard::OHAudioStreamBuilder *convertBuilder(OH_AudioStreamBuilder* builder)
{
    return (OHOS::AudioStandard::OHAudioStreamBuilder*) builder;
}

OH_AudioStream_Result OH_AudioRendererBuilder_SetSamplingRate(OH_AudioStreamBuilder* builder, int32_t rate)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    audioStreamBuilder->SetSamplingRate(rate);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRendererBuilder_SetChannelCount(OH_AudioStreamBuilder* builder, int32_t channelCount)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    audioStreamBuilder->SetChannelCount(channelCount);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudiRendererBuilder_SetSampleFormat(OH_AudioStreamBuilder* builder,
        OH_AudioStream_SampleFormat format)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    AudioSampleFormat sampleFormat = (AudioSampleFormat)format;
    audioStreamBuilder->SetSampleFormat(sampleFormat);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRendererBuilder_SetRendererInfo(OH_AudioStreamBuilder* builder,
        OH_AudioStream_Usage usage, OH_AudioStream_Content content)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    StreamUsage streamUsage = (StreamUsage)usage;
    ContentType contentType = (ContentType)content;
    audioStreamBuilder->SetRendererInfo(streamUsage, contentType);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRendererBuilder_SetEncodingType(OH_AudioStreamBuilder* builder,
        OH_AudioStream_EncodingType encodingType)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    AudioEncodingType type = (AudioEncodingType)encodingType;
    audioStreamBuilder->SetEncodingType(type);
    return AUDIOSTREAM_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
void OHAudioStreamBuilder::SetSamplingRate(int32_t rate)
{
    samplingRate_ =rate;
}

OH_AudioStream_Result OHAudioStreamBuilder::SetChannelCount(int32_t channelCount)
{
    channelCount_ = channelCount;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OHAudioStreamBuilder::SetSampleFormat(AudioSampleFormat sampleFormat)
{
    sampleFormat_ = sampleFormat;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OHAudioStreamBuilder::SetRendererInfo(StreamUsage usage, ContentType contentType)
{
    usage_ = usage;
    contentType_ = contentType;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OHAudioStreamBuilder::SetEncodingType(AudioEncodingType encodingType)
{
    encodingType_ = encodingType;
    return AUDIOSTREAM_SUCCESS;
}

int32_t OHAudioStreamBuilder::Generate(OH_AudioRenderer* renderer)
{
    OHAudioRenderer *audioRenderer = new OHAudioRenderer();
    renderer = (OH_AudioRenderer*)audioRenderer;
    return 0;
}

}  // namespace AudioStandard
}  // namespace OHOS
