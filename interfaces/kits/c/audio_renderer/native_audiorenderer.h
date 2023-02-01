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

#ifndef ST_NATIVE_AUDIORENDERER_H
#define ST_NATIVE_AUDIORENDERER_H

#include "native_audiostream_base.h"
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Request to release the renderer stream.
 *
 * @param renderer reference created by OH_AudioStreamBuilder
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_Release(OH_AudioRenderer* renderer);

/*
 * Request to start playing the renderer stream.
 *
 * @param renderer reference created by OH_AudioStreamBuilder
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_Start(OH_AudioRenderer* renderer);

/*
 * Request {bõfor the renderer stream to pause
 *
 * @param renderer reference created by OH_AudioStreamBuilder
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_Pause(OH_AudioRenderer* renderer);

/*
 * Request for the renderer stream stop
 *
 * @param renderer reference created by OH_AudioStreamBuilder
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_Stop(OH_AudioRenderer* renderer);

/*
 * Request for the renderer stream flush
 *
 * @param renderer reference created by OH_AudioStreamBuilder
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_Flush(OH_AudioRenderer* renderer);

/*
 * Query the current state of the renderer client
 *
 * This function will return the renderer state without updating the state.
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param state Pointer to a variable that will be set for the state value.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetCurrentState(OH_AudioRenderer* renderer, enum OH_AudioStream_State* state);

// 获取回调写入帧数
enum OH_AudioStream_Result OH_AudioRenderer_GetFrameSizeInCallback(OH_AudioRenderer* renderer, int32_t* frameSize);

/*
 * Query the sample rate value of the renderer client
 *
 * This function will return the renderer sample rate value without updating the state.
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param rate The state value to be updated
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetSamplingRate(OH_AudioRenderer* renderer, int32_t* rate);

/*
 * Query the stream id of the renderer client
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param stramId Pointer to a variable that will be set for the stream id.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetStreamId(OH_AudioRenderer* renderer, uint32_t* streamId);

/*
 * Query the channel count of the renderer client
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param channelCount Pointer to a variable that will be set for the channel count.
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetChannelCount(OH_AudioRenderer* renderer, int32_t* channelCount);

/*
 * Query the sample format of the renderer client
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param sampleFormat Pointer to a variable that will be set for the sample format. 
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetSampleFormat(OH_AudioRenderer* renderer,
        enum OH_AudioStream_SampleFormat* sampleFormat);

/*
 * Query the latency mode of the renderer client
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param latencyMode Pointer to a variable that will be set for the latency mode. 
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetLatencyMode(OH_AudioRenderer* renderer,
        enum OH_AudioStream_LatencyMode* latencyMode);
/*
 * Query the renderer info of the renderer client
 *
 * The rendere info includes {@link OH_AudioStream_Usage} value and {@link OH_AudioStream_Content} value.
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param usage Pointer to a variable that will be set for the stream usage. 
 * @param content Pointer to a variable that will be set for the stream content. 
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetRendererInfo(OH_AudioRenderer* renderer,
        enum OH_AudioStream_Usage* usage, enum OH_AudioStream_Content* content);

/*
 * Query the frame count number the has been written.
 *
 * @param renderer Reference created by OH_AudioStreamBuilder
 * @param frames Pointer to a variable that will be set for the frame count number. 
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
enum OH_AudioStream_Result OH_AudioRenderer_GetFramesWritten(OH_AudioRenderer* renderer, int32_t* frames);

#ifdef __cplusplus
}
#endif
#endif // ST_NATIVE_AUDIORENDERER_H
