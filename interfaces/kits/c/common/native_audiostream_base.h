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

#ifndef ST_NATIVE_AUDIOSTREAM_BASE_H
#define ST_NATIVE_AUDIOSTREAM_BASE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum OH_AudioStream_Result {
    AUDIOSTREAM_SUCCESS,
    AUDIOSTREAM_ERROR_INVALID_PARAM,
    AUDIOSTREAM_ERROR_ILLEGAL_STATE
};

enum OH_AudioStream_Type {
    /**
     * The type for audio stream is renderer.
     */
    AUDIOSTREAM_TYPE_RERNDERER = 1,

    /**
     * The type for audio stream is capturer.
     */
    AUDIOSTREAM_TYPE_CAPTURER = 2
};

/**
 * Define the audio stream sample format.
 */
enum OH_AudioStream_SampleFormat {
    AUDIOSTREAM_SAMPLE_U8 = 0,
    AUDIOSTREAM_SAMPLE_S16LE = 1,
    AUDIOSTREAM_SAMPLE_S24LE = 2,
    AUDIOSTREAM_SAMPLE_S32LE = 3,
    AUDIOSTREAM_SAMPLE_F32LE = 4,
};

enum OH_AudioStream_EncodingType {
    AUDIOSTREAM_ENCODING_PCM = 0,
    AUDIOSTREAM_ENCODING_INVALID = -1
};

enum OH_AudioStream_Usage {
    AUDIOSTREAM_USAGE_MEDIA,
    AUDIOSTREAM_USAGE_ALARM,
};

enum OH_AudioStream_Content {
    AUDIOSTREAM_CONTENT_TYPE_SPEECH,
    AUDIOSTREAM_CONTENT_TYPE_MUSIC
};

enum OH_AudioStream_LatencyMode {
    AUDIOSTREAM_LATENCY_MODE_NORMAL,
    AUDIOSTREAM_LATENCY_MODE_LOW_POWER,
    AUDIOSTREAM_LATENCY_MODE_FAST,
    AUDIOSTREAM_LATENCY_MODE_EXTREME_FAST
};

enum OH_AudioStream_State {
    AUDIOSTREAM_STATE_NEW,
    AUDIOSTREAM_STATE_PREPARED,
    AUDIOSTREAM_STATE_RUNNING,
    AUDIOSTREAM_STATE_STOPPED,
    AUDIOSTREAM_STATE_PAUSED,
    AUDIOSTREAM_STATE_RELEASED
};

enum OH_AudioInterrupt_ForceType {
    /**
     * Force type, system change audio state.
     */
    AUDIOSTREAM_INTERRUPT_FORCE = 0,
    /**
     * Share type, application change audio state.
     */
    AUDIOSTREAM_INTERRUPT_SHARE

};

enum OH_AudioInterrupt_Hint {
    AUDIOSTREAM_INTERRUPT_HINT_NONE = 0,
    AUDIOSTREAM_INTERRUPT_HINT_RESUME,
    AUDIOSTREAM_INTERRUPT_HINT_PAUSE,
    AUDIOSTREAM_INTERRUPT_HINT_STOP,
    AUDIOSTREAM_INTERRUPT_HINT_DUCK,
    AUDIOSTREAM_INTERRUPT_HINT_UNDUCK
};

typedef struct OH_AudioStreamBuilderStruct OH_AudioStreamBuilder;

typedef struct OH_AudioRendererStruct OH_AudioRenderer;

typedef struct OH_AudioCapturerStruct OH_AudioCapturer;

// callback for renderer
typedef int32_t (*OH_AudioRendererOnWriteData)(
        OH_AudioRenderer* renderer,
        void* userData,
        void*,
        int32_t);

typedef int32_t (*OH_AudioRendererOnStreamEvent)(
        OH_AudioRenderer* renderer,
        void* userData,
        void*,
        int32_t);

typedef int32_t (*OH_AudioRendererOnInterrptEvent)(
        OH_AudioRenderer*,
        void*,
        enum OH_AudioInterrupt_ForceType,
        enum OH_AudioInterrupt_Hint);

typedef int32_t (*OH_AudioRendererOnError)(
        OH_AudioRenderer*,
        void*,
        enum OH_AudioStream_Result);

struct OH_AudioRendererCallbacks {
    OH_AudioRendererOnWriteData OnWriteData;
    OH_AudioRendererOnStreamEvent OnStreamEvent;
    OH_AudioRendererOnInterrptEvent OnInterruptEvent;
    OH_AudioRendererOnError OnError;
};
#ifdef __cplusplus
}
#endif

#endif // ST_NATIVE_AUDIOSTREAM_BASE_H
