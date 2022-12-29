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
#include "OHAudio.h"
#include "OHAudioStreamBuilder.h"

using OHOS::AudioStandard::OHAudioStreamBuilder;

static OHAudioStreamBuilder *convertBuilder(OH_AudioStreamBuilder* builder)
{
    return (OHAudioStreamBuilder*) builder;
}

OH_AudioStream_Result OH_AudioStream_CreateBuilder(OH_AudioStreamBuilder** builder, OH_AudioStream_Type type)
{
    int32_t streamType = type == AUDIOSTREAM_TYPE_RERNDERER ? RENDERER_TYPE : CAPTURER_TYPE;
    OHAudioStreamBuilder *streamBuilder = new OHAudioStreamBuilder(streamType);

    *builder = (OH_AudioStreamBuilder*) streamBuilder;

    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRendererBuilder_Generate(OH_AudioStreamBuilder* builder, OH_AudioRenderer** audioRenderer)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    audioStreamBuilder->Generate(*audioRenderer);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioStreamBuilder_Destroy(OH_AudioStreamBuilder* builder)
{
    OHAudioStreamBuilder *audioStreamBuilder = convertBuilder(builder);
    if (audioStreamBuilder != nullptr) {
        delete audioStreamBuilder;
        return AUDIOSTREAM_SUCCESS;
    }
    return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
}
