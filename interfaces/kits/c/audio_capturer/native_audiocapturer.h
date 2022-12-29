
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

#ifndef ST_NATIVE_AUDIOCAPTURER_H
#define ST_NATIVE_AUDIOCAPTURER_H

#include "native_audiostream_base.h"
#ifdef __cplusplus
extern "C" {
#endif
OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer);

OH_AudioStream_Result OH_AudioCapturer_Start(OH_AudioCapturer* capturer);

OH_AudioStream_Result OH_AudioCapturer_Pause(OH_AudioCapturer* capturer);

OH_AudioStream_Result OH_AudioCapturer_Stop(OH_AudioCapturer* capturer);

OH_AudioStream_Result OH_AudioCapturer_Flush(OH_AudioCapturer* capturer);

OH_AudioStream_Result OH_AudioCapturer_GetLatencyMode(OH_AudioCapturer* capturer,
        OH_AudioStream_LatencyMode* latencyMode);

OH_AudioStream_Result OH_AudioCapturer_GetFramesRead(OH_AudioCapturer* capturer, int32_t* frames);


OH_AudioStream_Result OH_AudioCapturer_GetStreamId(OH_AudioCapturer*, uint32_t*);

OH_AudioStream_Result OH_AudioCapturer_GetSamplingRate(OH_AudioRenderer*, int32_t*);

OH_AudioStream_Result OH_AudioCapturer_GetChannelCount(OH_AudioRenderer*, int32_t*);

OH_AudioStream_Result OH_AudioCapturer_GetSampleFormat(OH_AudioRenderer*, OH_AudioStream_SampleFormat*);

OH_AudioStream_Result OH_AudioCapturer_GetEncodingType(OH_AudioRenderer*, OH_AudioStream_EncodingType*);

OH_AudioStream_Result OH_AudioCapturer_GetCapturerInfo(OH_AudioCapturer*, OH_AudioStream_Usage*, OH_AudioStream_Content);

#ifdef __cplusplus
}
#endif
#endif // ST_NATIVE_AUDIORENDERER_H
