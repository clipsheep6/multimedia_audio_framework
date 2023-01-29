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

#ifndef ST_NATIVE_AUDIOSTREAM_BUILDER_H
#define ST_NATIVE_AUDIOSTREAM_BUILDER_H

#include "native_audiostream_base.h"
#include "native_audiorenderer.h"
#ifdef __cplusplus
extern "C" {
#endif

enum OH_AudioStream_Result OH_AudioRendererBuilder_SetSamplingRate(OH_AudioStreamBuilder* builder, int32_t rate);

enum OH_AudioStream_Result OH_AudioRendererBuilder_SetChannelCount(OH_AudioStreamBuilder* builder, int32_t channelCount);

enum OH_AudioStream_Result OH_AudiRendererBuilder_SetSampleFormat(OH_AudioStreamBuilder* builder,
        enum OH_AudioStream_SampleFormat format);

enum OH_AudioStream_Result OH_AudioRendererBuilder_SetRendererInfo(OH_AudioStreamBuilder* builder,
        enum OH_AudioStream_Usage usage, enum OH_AudioStream_Content content);

enum OH_AudioStream_Result OH_AudioRendererBuilder_SetEncodingType(OH_AudioStreamBuilder* builder,
        enum OH_AudioStream_EncodingType encodingType);

enum OH_AudioStream_Result OH_AudioStreamBuilder_SetCallback(OH_AudioStreamBuilder* builder,
        struct OH_AudioRendererCallbacks callbacks, void*);

enum OH_AudioStream_Result OH_AudioRendererBuilder_Generate(OH_AudioStreamBuilder* builder,
        OH_AudioRenderer** audioRenderer);

enum OH_AudioStream_Result OH_AudioCapturerBuilder_Generate(OH_AudioStreamBuilder* builder,
        OH_AudioCapturer** audioCapturer);
#ifdef __cplusplus
}
#endif

#endif // ST_NATIVE_AUDIOSTREAM_BUILDER_H
