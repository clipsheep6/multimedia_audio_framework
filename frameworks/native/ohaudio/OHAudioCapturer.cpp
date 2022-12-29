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
#include "OHAudioCapturer.h"

namespace OHOS {
namespace AudioStandard {
static OHOS::AudioStandard::OHAudioCapturer *convertRenderer(OH_AudioCapturer* capturer)
{
    return (OHOS::AudioStandard::OHAudioCapturer*) capturer;
}

OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertRenderer(capturer);
    audioCapturer->Release();
    return AUDIOSTREAM_SUCCESS;
}


OH_AudioStream_Result OH_AudioCapturer_Start(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertRenderer(capturer);
    audioCapturer->Start();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_Pause(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertRenderer(capturer);
    audioCapturer->Pause();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_Stop(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    audiocapturer->Stop();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_Flush(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    audiocapturer->Flush();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetLatencyMode(OH_AudioCapturer* capturer,
        OH_AudioStream_LatencyMode* latencyMode)
{
  //  OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetFramesRead(OH_AudioCapturer* capturer, int32_t* frames)
{
   // OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}


OH_AudioStream_Result OH_AudioCapturer_GetStreamId(OH_AudioCapturer*, uint32_t*)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSamplingRate(OH_AudioRenderer*, int32_t*)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetChannelCount(OH_AudioRenderer*, int32_t*)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSampleFormat(OH_AudioRenderer*, OH_AudioStream_SampleFormat*)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetEncodingType(OH_AudioRenderer*, OH_AudioStream_EncodingType*)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetCapturerInfo(OH_AudioCapturer*, OH_AudioStream_Usage*, OH_AudioStream_Content)
{
    //OHOS::AudioStandard::OHAudioCapturer *audiocapturer = convertRenderer(capturer);
    // wait to implement
    return AUDIOSTREAM_SUCCESS;
}

int32_t OHAudioCapturer::Start()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_->Start();
    }
    return 0;
}
    
int32_t OHAudioCapturer::Pause()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_->Pause();
    }
    return 0;
}

int32_t OHAudioCapturer::Stop()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_->Stop();
    }
    return 0;
}

int32_t OHAudioCapturer::Flush()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_->Flush();
    }
    return 0;
}

int32_t OHAudioCapturer::Release()
{
    if (audioCapturer_ != nullptr) {
        audioCapturer_->Release();
    }
    return 0;
}

}  // namespace AudioStandard
}  // namespace OHOS
