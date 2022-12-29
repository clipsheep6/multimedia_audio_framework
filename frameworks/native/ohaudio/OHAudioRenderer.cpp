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
#include "OHAudioRenderer.h"

static OHOS::AudioStandard::OHAudioRenderer *convertRenderer(OH_AudioRenderer* renderer)
{
    return (OHOS::AudioStandard::OHAudioRenderer*) renderer;
}

OH_AudioStream_Result OH_AudioRenderer_Start(OH_AudioRenderer* renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    audioRenderer->Start();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_Pause(OH_AudioRenderer* renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    audioRenderer->Pause();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_Stop(OH_AudioRenderer* renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    audioRenderer->Stop();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_Flush(OH_AudioRenderer* renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    audioRenderer->Flush();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_Release(OH_AudioRenderer* renderer)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);
    audioRenderer->Release();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetCurrentState(OH_AudioRenderer* renderer, OH_AudioStream_State* state)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);

    OHOS::AudioStandard::RendererState rendererState = audioRenderer->GetStatus();
    *state = (OH_AudioStream_State)rendererState;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetFrameSizeInCallback(OH_AudioRenderer* renderer, int32_t* frameSize)
{
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSamplingRate(OH_AudioRenderer* renderer, int32_t* rate)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);

    OHOS::AudioStandard::AudioRendererRate rendererRate = audioRenderer->GetRenderRate();
    *rate = (int32_t)rendererRate;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioStream_State(OH_AudioRenderer* renderer, uint32_t* streamId)
{
    OHOS::AudioStandard::OHAudioRenderer *audioRenderer = convertRenderer(renderer);

    audioRenderer->GetStreamId(*streamId);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetChannelCount(OH_AudioRenderer* renderer, int32_t* channelCount)
{
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetSampleFormat(OH_AudioRenderer* renderer,
        OH_AudioStream_SampleFormat* sampleFormat)
{
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetLatencyMode(OH_AudioRenderer* renderer,
        OH_AudioStream_LatencyMode* latencyMode)
{
    return AUDIOSTREAM_SUCCESS;
}
OH_AudioStream_Result OH_AudioRenderer_GetRendererInfo(OH_AudioRenderer* renderer,
        OH_AudioStream_Usage* usage, OH_AudioStream_Content* content)
{
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioRenderer_GetFramesWritten(OH_AudioRenderer* renderer, int32_t* frames)
{
    return AUDIOSTREAM_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
int32_t OHAudioRenderer::Start()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Start();
    }
    return 0;
}

int32_t OHAudioRenderer::Pause()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Pause();
    }
    return 0;
}

int32_t OHAudioRenderer::Stop()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Stop();
    }
    return 0;
}

int32_t OHAudioRenderer::Flush()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Flush();
    }
    return 0;
}

int32_t OHAudioRenderer::Release()
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->Release();
    }
    return 0;
}

RendererState OHAudioRenderer::GetStatus()
{
    if (audioRenderer_ != nullptr) {
        return audioRenderer_->GetStatus();
    }
    return RENDERER_INVALID;
}

AudioRendererRate OHAudioRenderer::GetRenderRate()
{
    if (audioRenderer_ != nullptr) {
        return audioRenderer_->GetRenderRate();
    }
    return RENDER_RATE_NORMAL;
}

void OHAudioRenderer::GetStreamId(uint32_t &sessionID)
{
    if (audioRenderer_ != nullptr) {
        audioRenderer_->GetAudioStreamId(sessionID);
    }
}

int32_t OHAudioRenderer::GetChannelCount()
{
    return 0;
}

int32_t OHAudioRenderer::GetLatencyMode()
{
    return 0;
}

StreamUsage OHAudioRenderer::GetStreamUsage()
{
    return STREAM_USAGE_UNKNOWN;
}

ContentType OHAudioRenderer::GetContentType()
{
    return CONTENT_TYPE_UNKNOWN;
}

int32_t OHAudioRenderer::GetFramesWritten()
{
    return 0;
}
}  // namespace AudioStandard
}  // namespace OHOS
