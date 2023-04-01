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

/**
 * These result codes are returned from ndk fuctions.
 *
 * @since 10
 */
typedef enum {
    /**
     * The call was successful.
     */
    AUDIOSTREAM_SUCCESS,

    /**
     * This means that the function was executed with an invalid input parameter.
     */
    AUDIOSTREAM_ERROR_INVALID_PARAM,

    /**
     * Execution status exception.
     */
    AUDIOSTREAM_ERROR_ILLEGAL_STATE,

    /**
     * An system error has occurred.
     */
    AUDIOSTREAM_ERROR_SYSTEM
} OH_AudioStream_Result;

/**
 * Define the audio stream type.
 *
 * @since 10
 */
typedef enum {
    /**
     * The type for audio stream is renderer.
     */
    AUDIOSTREAM_TYPE_RERNDERER = 1,

    /**
     * The type for audio stream is capturer.
     */
    AUDIOSTREAM_TYPE_CAPTURER = 2
} OH_AudioStream_Type;

/**
 * Define the audio stream sample format.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_SAMPLE_U8 = 0,
    AUDIOSTREAM_SAMPLE_S16LE = 1,
    AUDIOSTREAM_SAMPLE_S24LE = 2,
    AUDIOSTREAM_SAMPLE_S32LE = 3,
    AUDIOSTREAM_SAMPLE_F32LE = 4,
} OH_AudioStream_SampleFormat;

/**
 * Define the audio encoding type.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_ENCODING_TYPE_RAW = 0,
} OH_AudioStream_EncodingType;

/**
 * Define the audio stream usage.
 * Audio stream usage is used to describe what work scenario
 * the current stream is used for.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_USAGE_UNKNOWN = 0,
    AUDIOSTREAM_USAGE_MEDIA = 1,
    AUDIOSTREAM_USAGE_COMMUNICATION = 2,
    AUDIOSTREAM_USAGE_NOTIFICATION = 6,
    AUDIOSTREAM_USAGE_SYSTEM = 9
} OH_AudioStream_Usage;

/**
 * Define the audio stream content.
 * Audio stream content is used to describe the stream data
 * type.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_CONTENT_TYPE_UNKNOWN = 0,
    AUDIOSTREAM_CCONTENT_TYPE_SPEECH = 1,
    AUDIOSTREAM_CCONTENT_TYPE_MUSIC = 2,
    AUDIOSTREAM_CCONTENT_TYPE_MOVIE = 3,
    AUDIOSTREAM_CCONTENT_TYPE_SONIFICATION = 4,
    AUDIOSTREAM_CONTENT_TYPE_RINGTONE = 5,
    AUDIOSTREAM_CONTENT_TYPE_ULTRASONIC = 9,
} OH_AudioStream_Content;

/**
 * Define the audio latency mode.
 *
 * @since 10
 */
typedef enum {
    /**
     * This is a normal audio scene.
     */
    AUDIOSTREAM_LATENCY_MODE_NORMAL,
    /**
     * This is a low latency audio scene.
     */
    AUDIOSTREAM_LATENCY_MODE_FAST
} OH_AudioStream_LatencyMode;

/**
 * Define the audio event.
 *
 * @since 10
 */
typedef enum {
    /**
     * The routing of the audio has changed.
     */
    AUDIOSTREAM_EVENT_ROUTING_CHANGED
} OH_AudioStream_Event;

/**
 * The audio stream states
 *
 * @since 10
 */
typedef enum {
    /**
     * The invalid state.
     */
    AUDIOSTREAM_STATE_INVALID,
    /**
     * The stream is created. 
     */
    AUDIOSTREAM_STATE_NEW,
    /**
     * The prepared state.
     */
    AUDIOSTREAM_STATE_PREPARED,
    /**
     * The stream is running.
     */
    AUDIOSTREAM_STATE_RUNNING,
    /**
     * The stream is stopped.
     */
    AUDIOSTREAM_STATE_STOPPED,
    /**
     * The stream is paused.
     */
    AUDIOSTREAM_STATE_PAUSED,
    /**
     * The stream is released.
     */
    AUDIOSTREAM_STATE_RELEASED
} OH_AudioStream_State;

/**
 * Defines the audio interrupt type.
 *
 * @since 10
 */
typedef enum {
    /**
     * Force type, system change audio state.
     */
    AUDIOSTREAM_INTERRUPT_FORCE = 0,
    /**
     * Share type, application change audio state.
     */
    AUDIOSTREAM_INTERRUPT_SHARE

} OH_AudioInterrupt_ForceType;

/**
 * Defines the audio interrupt hint type.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_INTERRUPT_HINT_NONE = 0,
    AUDIOSTREAM_INTERRUPT_HINT_RESUME,
    AUDIOSTREAM_INTERRUPT_HINT_PAUSE,
    AUDIOSTREAM_INTERRUPT_HINT_STOP,
    AUDIOSTREAM_INTERRUPT_HINT_DUCK,
    AUDIOSTREAM_INTERRUPT_HINT_UNDUCK
} OH_AudioInterrupt_Hint;

/**
 * Defines the audio source type.
 *
 * @since 10
 */
typedef enum {
    AUDIOSTREAM_SOURCE_TYPE_INVALID = -1,
    AUDIOSTREAM_SOURCE_TYPE_MIC,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION = 1,
    AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION = 7
} OH_AudioStream_SourceType;

/**
 * Declaring the audio stream builder.
 * The instance of builder is used for creating audio stream.
 *
 * @since 10
 */
typedef struct OH_AudioStreamBuilderStruct OH_AudioStreamBuilder;

/**
 * Declaring the audio renderer stream.
 * The instance of renderer stream is used for playing audio data.
 *
 * @since 10
 */
typedef struct OH_AudioRendererStruct OH_AudioRenderer;

/**
 * Declaring the audio capturer stream.
 * The instance of renderer stream is used for capturing audio data.
 *
 * @since 10
 */
typedef struct OH_AudioCapturerStruct OH_AudioCapturer;

/**
 * Declaring the callback struct for renderer stream.
 *
 * @since 10
 */
typedef struct OH_AudioRenderer_Callbacks_Struct {
    /**
     * This function pointer will point to the callback function that
     * is used to write audio data
     */
    int32_t (*OH_AudioRenderer_OnWriteData)(
            OH_AudioRenderer* renderer,
            void* userData,
            void* buffer,
            int32_t lenth);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio renderer stream events.
     */
    int32_t (*OH_AudioRenderer_OnStreamEvent)(
            OH_AudioRenderer* renderer,
            void* userData,
            OH_AudioStream_Event event);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio interrupt events.
     */
    int32_t (*OH_AudioRenderer_OnInterrptEvent)(
            OH_AudioRenderer* renderer,
            void* userData,
            OH_AudioInterrupt_ForceType,
            OH_AudioInterrupt_Hint);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio error result.
     */
    int32_t (*OH_AudioRenderer_OnError)(
            OH_AudioRenderer* renderer,
            void* userData,
            OH_AudioStream_Result error);
} OH_AudioRenderer_Callbacks;

/**
 * Declaring the callback struct for capturer stream.
 *
 * @since 10
 */
typedef struct OH_AudioCapturer_Callbacks_Struct {
    /**
     * This function pointer will point to the callback function that
     * is used to read audio data.
     */
    int32_t (*OH_AudioCapturer_OnReadData)(
            OH_AudioCapturer* capturer,
            void* userData,
            void* buffer,
            int32_t lenth);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio capturer stream events.
     */
    int32_t (*OH_AudioCapturer_OnStreamEvent)(
            OH_AudioCapturer* capturer,
            void* userData,
            OH_AudioStream_Event event);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio interrupt events.
     */
    int32_t (*OH_AudioCapturer_OnInterrptEvent)(
            OH_AudioCapturer* renderer,
            void* userData,
            OH_AudioInterrupt_ForceType,
            OH_AudioInterrupt_Hint);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio error result.
     */
    int32_t (*OH_AudioCapturer_OnError)(
            OH_AudioCapturer* capturer,
            void* userData ,
            OH_AudioStream_Result error);
} OH_AudioCapturer_Callbacks;
#ifdef __cplusplus
}
#endif

#endif // ST_NATIVE_AUDIOSTREAM_BASE_H
