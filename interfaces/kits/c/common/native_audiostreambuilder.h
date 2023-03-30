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

/**
 * Create a stremBuilder can be used to open a renderer or capturer client.
 *
 * OH_AudioStreamBuilder_Destroy() must be called when you are done using the builder.
 *
 * @since 10
 *
 * @param builder The builder reference to the created result.
 * @param type The stream type to be created. {@link #AUDIOSTREAM_TYPE_RERNDERER} or {@link #AUDIOSTREAM_TYPE_CAPTURER}
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_Create(OH_AudioStreamBuilder** builder, OH_AudioStream_Type type);

/**
 * Destroy a streamBulder.
 *
 * This function must be called when you are done using the builder.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_Destroy(OH_AudioStreamBuilder* builder);
/*
 * Set the channel count of the capturer client
 *
 * @param capturer Reference created by OH_AudioStreamBuilder
 * @param channelCount Pointer to a variable that will be set for the channel count.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetSamplingRate(OH_AudioStreamBuilder* builder, int32_t rate);

/*
 * Set the channel count of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param channelCount The channel count.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetChannelCount(OH_AudioStreamBuilder* builder, int32_t channelCount);

/*
 * Set the sample format of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param format Sample data format.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetSampleFormat(OH_AudioStreamBuilder* builder,
    OH_AudioStream_SampleFormat format);

/*
 * Set the encoding type of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param encodingType Encoding type for the stream client, {@link #AUDIOSTREAM_ENCODING_PCM}{
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetEncodingType(OH_AudioStreamBuilder* builder,
    OH_AudioStream_EncodingType encodingType);

/*
 * Set the encoding type of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param encodingType Encoding type for the stream client, {@link #AUDIOSTREAM_ENCODING_PCM}
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetLatencyMode(OH_AudioStreamBuilder* builder,
    OH_AudioStream_LatencyMode latencyMode);

/*
 * Set the renderer information of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param usage Set the stream usage for the renderer client.
 * @param content Set the stream content for the renderer client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererInfo(OH_AudioStreamBuilder* builder,
    OH_AudioStream_Usage usage, OH_AudioStream_Content content);
/*
 * Set the capturer information of the stream client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param sourceType Set the source type for the capturer client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetCapturerInfo(OH_AudioStreamBuilder* builder,
    OH_AudioStream_SourceType sourceType);

/*
 * Set the callbacks for the renderer client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callbacks Callbacks to the functions that will process renderer stream.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetRendererCallback(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer_Callbacks callbacks, void* userData);

/*
 * Set the callbacks for the capturer client
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param callbacks Callbacks to the functions that will process capturer stream.
 * @param userData Pointer to an application data structure that will be passed to the callback functions.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_SetCapturerCallback(OH_AudioStreamBuilder* builder,
    OH_AudioCapturer_Callbacks callbacks, void* userData);

/*
 * Create the audio renderer client.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param audioRenderer Pointer to a viriable to receive the stream client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_GenerateRenderer(OH_AudioStreamBuilder* builder,
    OH_AudioRenderer** audioRenderer);
/*
 * Create the audio capturer client.
 *
 * @since 10
 *
 * @param builder Reference provided by OH_AudioStreamBuilder_Create()
 * @param audioRenderer Pointer to a viriable to receive the stream client.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_GenerateCapturer(OH_AudioStreamBuilder* builder,
    OH_AudioCapturer** audioCapturer);
#ifdef __cplusplus
}
#endif

#endif // ST_NATIVE_AUDIOSTREAM_BUILDER_H
