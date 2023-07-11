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

/**
 * @addtogroup OHAudio
 * @{
 *
 * @brief Provide the definition of the C interface for the audio module.
 *
 * @syscap SystemCapability.Multimedia.Audio.Core
 *
 * @since 10
 * @version 1.0
 */

/**
 * @file native_audiostream_base.h
 *
 * @brief Declare the underlying data structure.
 *
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @since 10
 * @version 1.0
 */

#ifndef ST_NATIVE_AUDIOSTREAM_BASE_H
#define ST_NATIVE_AUDIOSTREAM_BASE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define the result of the function execution.
 *
 * @since 10
 */
typedef enum {
    /**
     * The call was successful.
     */
    AUDIOSTREAM_SUCCESS = 0,

    /**
     * This means that the function was executed with an invalid input parameter.
     */
    AUDIOSTREAM_ERROR_INVALID_PARAM = 1,

    /**
     * Execution status exception.
     */
    AUDIOSTREAM_ERROR_ILLEGAL_STATE = 2,

    /**
     * An system error has occurred.
     */
    AUDIOSTREAM_ERROR_SYSTEM = 3
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
    AUDIOSTREAM_TYPE_RENDERER = 1,

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
    /**
     * Unsigned 8 format.
     */
    AUDIOSTREAM_SAMPLE_U8 = 0,
    /**
     * Signed 16 bit integer, little endian.
     */
    AUDIOSTREAM_SAMPLE_S16LE = 1,
    /**
     * Signed 24 bit integer, little endian.
     */
    AUDIOSTREAM_SAMPLE_S24LE = 2,
    /**
     * Signed 32 bit integer, little endian.
     */
    AUDIOSTREAM_SAMPLE_S32LE = 3,
} OH_AudioStream_SampleFormat;

/**
 * Define the audio encoding type.
 *
 * @since 10
 */
typedef enum {
    /**
     * PCM encoding type.
     */
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
    /**
     * Unknown usage.
     */
    AUDIOSTREAM_USAGE_UNKNOWN = 0,
    /**
     * Music usage.
     */
    AUDIOSTREAM_USAGE_MUSIC = 1,
    /**
     * Voice communication usage.
     */
    AUDIOSTREAM_USAGE_VOICE_COMMUNICATION = 2,
    /**
     * Voice assistant usage.
     */
    AUDIOSTREAM_USAGE_VOICE_ASSISTANT = 3,
    /**
     * Alarm usage.
     */
    AUDIOSTREAM_USAGE_ALARM = 4,
    /**
     * Voice message usage.
     */
    AUDIOSTREAM_USAGE_VOICE_MESSAGE = 5,
    /**
     * Ringtone usage.
     */
    AUDIOSTREAM_USAGE_RINGTONE = 6,
    /**
     * Notification usage.
     */
    AUDIOSTREAM_USAGE_NOTIFICATION = 7,
    /**
     * Accessibility usage, such as screen reader.
     */
    AUDIOSTREAM_USAGE_ACCESSIBILITY = 8,
    /**
     * Movie or video usage.
     */
    AUDIOSTREAM_USAGE_MOVIE = 10,
    /**
     * Game sound effect usage.
     */
    AUDIOSTREAM_USAGE_GAME = 11,
    /**
     * Audiobook usage.
     */
    AUDIOSTREAM_USAGE_AUDIOBOOK = 12,
    /**
     * Navigation usage.
     */
    AUDIOSTREAM_USAGE_NAVIGATION = 13,
} OH_AudioStream_Usage;

/**
 * Define the audio latency mode.
 *
 * @since 10
 */
typedef enum {
    /**
     * This is a normal audio scene.
     */
    AUDIOSTREAM_LATENCY_MODE_NORMAL = 0,
    /**
     * This is a low latency audio scene.
     */
    AUDIOSTREAM_LATENCY_MODE_FAST = 1
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
    AUDIOSTREAM_EVENT_ROUTING_CHANGED = 0
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
    AUDIOSTREAM_STATE_INVALID = -1,
    /**
     * Create new instance state.
     */
    AUDIOSTREAM_STATE_NEW = 0,
    /**
     * The prepared state.
     */
    AUDIOSTREAM_STATE_PREPARED = 1,
    /**
     * The stream is running.
     */
    AUDIOSTREAM_STATE_RUNNING = 2,
    /**
     * The stream is stopped.
     */
    AUDIOSTREAM_STATE_STOPPED = 3,
    /**
     * The stream is released.
     */
    AUDIOSTREAM_STATE_RELEASED = 4,
    /**
     * The stream is paused.
     */
    AUDIOSTREAM_STATE_PAUSED = 5,
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
    AUDIOSTREAM_INTERRUPT_SHARE = 1
} OH_AudioInterrupt_ForceType;

/**
 * Defines the audio interrupt hint type.
 *
 * @since 10
 */
typedef enum {
    /**
     * None.
     */
    AUDIOSTREAM_INTERRUPT_HINT_NONE = 0,
    /**
     * Resume the stream.
     */
    AUDIOSTREAM_INTERRUPT_HINT_RESUME = 1,
    /**
     * Pause the stream.
     */
    AUDIOSTREAM_INTERRUPT_HINT_PAUSE = 2,
    /**
     * Stop the stream.
     */
    AUDIOSTREAM_INTERRUPT_HINT_STOP = 3,
    /**
     * Ducked the stream.
     */
    AUDIOSTREAM_INTERRUPT_HINT_DUCK = 4,
    /**
     * Unducked the stream.
     */
    AUDIOSTREAM_INTERRUPT_HINT_UNDUCK = 5
} OH_AudioInterrupt_Hint;

/**
 * Defines the audio source type.
 *
 * @since 10
 */
typedef enum {
    /**
     * Invalid type.
     */
    AUDIOSTREAM_SOURCE_TYPE_INVALID = -1,
    /**
     * Mic source type.
     */
    AUDIOSTREAM_SOURCE_TYPE_MIC = 0,
    /**
     * Voice recognition source type.
     */
    AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION = 1,
    /**
     * Playback capture source type.
     */
    AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE = 2,
    /**
     * Voice communication source type.
     */
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
            OH_AudioInterrupt_ForceType type,
            OH_AudioInterrupt_Hint hint);

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
            OH_AudioInterrupt_ForceType type,
            OH_AudioInterrupt_Hint hint);

    /**
     * This function pointer will point to the callback function that
     * is used to handle audio error result.
     */
    int32_t (*OH_AudioCapturer_OnError)(
            OH_AudioCapturer* capturer,
            void* userData,
            OH_AudioStream_Result error);
} OH_AudioCapturer_Callbacks;
#ifdef __cplusplus
}
#endif

#endif // ST_NATIVE_AUDIOSTREAM_BASE_H
