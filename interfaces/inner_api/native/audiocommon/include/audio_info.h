/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef AUDIO_INFO_H
#define AUDIO_INFO_H

#ifdef __MUSL__
#include <stdint.h>
#endif // __MUSL__

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <unistd.h>

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_NUM_STREAMS = 3;
constexpr int32_t RENDERER_STREAM_USAGE_SHIFT = 16;
constexpr int32_t MINIMUM_BUFFER_SIZE_MSEC = 5;
constexpr int32_t MAXIMUM_BUFFER_SIZE_MSEC = 20;
constexpr int32_t MIN_SERVICE_COUNT = 2;
constexpr int32_t ROOT_UID = 0;

const std::string MICROPHONE_PERMISSION = "ohos.permission.MICROPHONE";
const std::string MODIFY_AUDIO_SETTINGS_PERMISSION = "ohos.permission.MODIFY_AUDIO_SETTINGS";
const std::string ACCESS_NOTIFICATION_POLICY_PERMISSION = "ohos.permission.ACCESS_NOTIFICATION_POLICY";

enum DeviceFlag {
    /**
     * Device flag none.
     */
    DEVICE_FLAG_NONE = -1,
    /**
     * Indicates all output audio devices.
     */
    OUTPUT_DEVICES_FLAG = 1,
    /**
     * Indicates all input audio devices.
     */
    INPUT_DEVICES_FLAG = 2,
    /**
     * Indicates all audio devices.
     */
    ALL_DEVICES_FLAG = 3,
    /**
     * Device flag max count.
     */
    DEVICE_FLAG_MAX
};

enum DeviceRole {
    /**
     * Device role none.
     */
    DEVICE_ROLE_NONE = -1,
    /**
     * Input device role.
     */
    INPUT_DEVICE = 1,
    /**
     * Output device role.
     */
    OUTPUT_DEVICE = 2,
    /**
     * Device role max count.
     */
    DEVICE_ROLE_MAX
};

enum DeviceType {
    /**
     * Indicates device type none.
     */
    DEVICE_TYPE_NONE = -1,
    /**
     * Indicates invalid device
     */
    DEVICE_TYPE_INVALID = 0,
    /**
     * Indicates a speaker built in a device.
     */
    DEVICE_TYPE_SPEAKER = 2,
    /**
     * Indicates a headset, which is the combination of a pair of headphones and a microphone.
     */
    DEVICE_TYPE_WIRED_HEADSET = 3,
    /**
     * Indicates a Bluetooth device used for telephony.
     */
    DEVICE_TYPE_BLUETOOTH_SCO = 7,
    /**
     * Indicates a Bluetooth device supporting the Advanced Audio Distribution Profile (A2DP).
     */
    DEVICE_TYPE_BLUETOOTH_A2DP = 8,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_MIC = 15,
    /**
     * Indicates a microphone built in a device.
     */
    DEVICE_TYPE_USB_HEADSET = 22,
    /**
     * Indicates a debug sink device
     */
    DEVICE_TYPE_FILE_SINK = 50,
    /**
     * Indicates a debug source device
     */
    DEVICE_TYPE_FILE_SOURCE = 51,
    /**
     * Indicates device type max count.
     */
    DEVICE_TYPE_MAX
};

enum ActiveDeviceType {
    ACTIVE_DEVICE_TYPE_NONE = -1,
    SPEAKER = 2,
    BLUETOOTH_SCO = 7,
    FILE_SINK_DEVICE = 50,
    ACTIVE_DEVICE_TYPE_MAX
};

enum AudioStreamType {
    /**
     * Indicates audio streams default.
     */
    STREAM_DEFAULT = -1,
    /**
     * Indicates audio streams of voices in calls.
     */
    STREAM_VOICE_CALL = 0,
    /**
     * Indicates audio streams for music playback.
     */
    STREAM_MUSIC = 1,
    /**
     * Indicates audio streams for ringtones.
     */
    STREAM_RING = 2,
    /**
     * Indicates audio streams media.
     */
    STREAM_MEDIA = 3,
    /**
     * Indicates Audio streams for voice assistant
     */
    STREAM_VOICE_ASSISTANT = 4,
    /**
     * Indicates audio streams for system sounds.
     */
    STREAM_SYSTEM = 5,
    /**
     * Indicates audio streams for alarms.
     */
    STREAM_ALARM = 6,
    /**
     * Indicates audio streams for notifications.
     */
    STREAM_NOTIFICATION = 7,
    /**
     * Indicates audio streams for voice calls routed through a connected Bluetooth device.
     */
    STREAM_BLUETOOTH_SCO = 8,
    /**
     * Indicates audio streams for enforced audible.
     */
    STREAM_ENFORCED_AUDIBLE = 9,
    /**
     * Indicates audio streams for dual-tone multi-frequency (DTMF) tones.
     */
    STREAM_DTMF = 10,
    /**
     * Indicates audio streams exclusively transmitted through the speaker (text-to-speech) of a device.
     */
    STREAM_TTS =  11,
    /**
     * Indicates audio streams used for prompts in terms of accessibility.
     */
    STREAM_ACCESSIBILITY = 12
};

enum AudioEncodingType {
    ENCODING_PCM = 0,
    ENCODING_INVALID = -1
};

// Ringer Mode
enum AudioRingerMode {
    RINGER_MODE_NORMAL = 0,
    RINGER_MODE_SILENT = 1,
    RINGER_MODE_VIBRATE = 2
};

// format
enum AudioSampleFormat {
    SAMPLE_U8 = 8,
    SAMPLE_S16LE = 16,
    SAMPLE_S24LE = 24,
    SAMPLE_S32LE = 32,
    SAMPLE_F32LE = 32,
    INVALID_WIDTH = -1
};

// channel
enum AudioChannel {
    MONO = 1,
    STEREO = 2,
    CHANNEL_3 = 3,
    CHANNEL_4 = 4,
    CHANNEL_5 = 5,
    CHANNEL_6 = 6,
    CHANNEL_7 = 7,
    CHANNEL_8 = 8
};

// sampling rate
enum AudioSamplingRate {
    SAMPLE_RATE_8000 = 8000,
    SAMPLE_RATE_11025 = 11025,
    SAMPLE_RATE_12000 = 12000,
    SAMPLE_RATE_16000 = 16000,
    SAMPLE_RATE_22050 = 22050,
    SAMPLE_RATE_24000 = 24000,
    SAMPLE_RATE_32000 = 32000,
    SAMPLE_RATE_44100 = 44100,
    SAMPLE_RATE_48000 = 48000,
    SAMPLE_RATE_64000 = 64000,
    SAMPLE_RATE_96000 = 96000
};

typedef enum {
    /** Invalid audio source */
    AUDIO_SOURCE_INVALID = -1,
    /** Default audio source */
    AUDIO_SOURCE_DEFAULT = 0,
    /** Microphone */
    AUDIO_MIC = 1,
    /** Uplink voice */
    AUDIO_VOICE_UPLINK = 2,
    /** Downlink voice */
    AUDIO_VOICE_DOWNLINK = 3,
    /** Voice call */
    AUDIO_VOICE_CALL = 4,
    /** Camcorder */
    AUDIO_CAMCORDER = 5,
    /** Voice recognition */
    AUDIO_VOICE_RECOGNITION = 6,
    /** Voice communication */
    AUDIO_VOICE_COMMUNICATION = 7,
    /** Remote submix */
    AUDIO_REMOTE_SUBMIX = 8,
    /** Unprocessed audio */
    AUDIO_UNPROCESSED = 9,
    /** Voice performance */
    AUDIO_VOICE_PERFORMANCE = 10,
    /** Echo reference */
    AUDIO_ECHO_REFERENCE = 1997,
    /** Radio tuner */
    AUDIO_RADIO_TUNER = 1998,
    /** Hotword */
    AUDIO_HOTWORD = 1999,
    /** Extended remote submix */
    AUDIO_REMOTE_SUBMIX_EXTEND = 10007,
} AudioSourceType;

struct AudioStreamParams {
    uint32_t samplingRate;
    uint8_t encoding;
    uint8_t format;
    uint8_t channels;
};

/**
* Enumerates the audio content type.
*/
enum ContentType {
    CONTENT_TYPE_UNKNOWN = 0,
    CONTENT_TYPE_SPEECH = 1,
    CONTENT_TYPE_MUSIC = 2,
    CONTENT_TYPE_MOVIE = 3,
    CONTENT_TYPE_SONIFICATION = 4,
    CONTENT_TYPE_RINGTONE = 5
};

/**
* Enumerates the stream usage.
*/
enum StreamUsage {
    STREAM_USAGE_UNKNOWN = 0,
    STREAM_USAGE_MEDIA = 1,
    STREAM_USAGE_VOICE_COMMUNICATION = 2,
    STREAM_USAGE_VOICE_ASSISTANT = 4,
    STREAM_USAGE_NOTIFICATION_RINGTONE = 6
};

/**
* Enumerates the capturer source type
*/
enum SourceType {
    SOURCE_TYPE_INVALID = -1,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_COMMUNICATION = 7
};

/**
* Enumerates the renderer playback speed.
*/
enum AudioRendererRate {
    RENDER_RATE_NORMAL = 0,
    RENDER_RATE_DOUBLE = 1,
    RENDER_RATE_HALF = 2,
};

enum InterruptType {
    INTERRUPT_TYPE_BEGIN = 1,
    INTERRUPT_TYPE_END = 2,
};

enum InterruptHint {
    INTERRUPT_HINT_NONE = 0,
    INTERRUPT_HINT_RESUME,
    INTERRUPT_HINT_PAUSE,
    INTERRUPT_HINT_STOP,
    INTERRUPT_HINT_DUCK,
    INTERRUPT_HINT_UNDUCK
};

enum InterruptForceType {
    /**
     * Force type, system change audio state.
     */
    INTERRUPT_FORCE = 0,
    /**
     * Share type, application change audio state.
     */
    INTERRUPT_SHARE
};

enum ActionTarget {
    CURRENT = 0,
    INCOMING,
    BOTH
};

struct InterruptEvent {
    /**
     * Interrupt event type, begin or end
     */
    InterruptType eventType;
    /**
     * Interrupt force type, force or share
     */
    InterruptForceType forceType;
    /**
     * Interrupt hint type. In force type, the audio state already changed,
     * but in share mode, only provide a hint for application to decide.
     */
    InterruptHint hintType;
};

// Used internally only by AudioFramework
struct InterruptEventInternal {
    InterruptType eventType;
    InterruptForceType forceType;
    InterruptHint hintType;
    float duckVolume;
};

struct AudioFocusEntry {
    InterruptForceType forceType;
    InterruptHint hintType;
    ActionTarget actionOn;
    bool isReject;
};

struct AudioInterrupt {
    StreamUsage streamUsage;
    ContentType contentType;
    AudioStreamType streamType;
    uint32_t sessionID;
    bool pauseWhenDucked;
};

struct VolumeEvent {
    AudioStreamType volumeType;
    int32_t volume;
    bool updateUi;
};

struct AudioParameters {
    AudioSampleFormat format;
    AudioChannel channels;
    AudioSamplingRate samplingRate;
    AudioEncodingType encoding;
    ContentType contentType;
    StreamUsage usage;
    DeviceRole deviceRole;
    DeviceType deviceType;
};

struct AudioStreamInfo {
    AudioSamplingRate samplingRate;
    AudioEncodingType encoding;
    AudioSampleFormat format;
    AudioChannel channels;
};

struct AudioRendererInfo {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t rendererFlags = 0;
};

struct AudioCapturerInfo {
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t capturerFlags = 0;
};

struct AudioRendererDesc {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
};

struct AudioRendererOptions {
    AudioStreamInfo streamInfo;
    AudioRendererInfo rendererInfo;
};

enum DeviceChangeType {
    CONNECT = 0,
    DISCONNECT = 1,
};

enum AudioScene {
    /**
     * Default audio scene
     */
    AUDIO_SCENE_DEFAULT,
    /**
     * Ringing audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_RINGING,
    /**
     * Phone call audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_PHONE_CALL,
    /**
     * Voice chat audio scene
     */
    AUDIO_SCENE_PHONE_CHAT,
};

struct AudioCapturerOptions {
    AudioStreamInfo streamInfo;
    AudioCapturerInfo capturerInfo;
};

struct AppInfo {
    int32_t appUid { 0 };
    uint32_t appTokenId { 0 };
};

// Supported audio parameters for both renderer and capturer
const std::vector<AudioSampleFormat> AUDIO_SUPPORTED_FORMATS {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE
};

const std::vector<AudioChannel> RENDERER_SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8
};

const std::vector<AudioChannel> CAPTURER_SUPPORTED_CHANNELS {
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6
};

const std::vector<AudioEncodingType> AUDIO_SUPPORTED_ENCODING_TYPES {
    ENCODING_PCM
};

const std::vector<AudioSamplingRate> AUDIO_SUPPORTED_SAMPLING_RATES {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_11025,
    SAMPLE_RATE_12000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_22050,
    SAMPLE_RATE_24000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_96000
};

struct BufferDesc {
    uint8_t* buffer;
    size_t bufLength;
    size_t dataLength;
};

struct BufferQueueState {
    uint32_t numBuffers;
    uint32_t currentIndex;
};

enum AudioRenderMode {
    RENDER_MODE_NORMAL,
    RENDER_MODE_CALLBACK
};

enum AudioCaptureMode {
    CAPTURE_MODE_NORMAL,
    CAPTURE_MODE_CALLBACK
};

typedef uint32_t AudioIOHandle;

static inline bool FLOAT_COMPARE_EQ(const float& x, const float& y)
{
    return (std::abs((x) - (y)) <= (std::numeric_limits<float>::epsilon()));
}

// Below APIs are added to handle compilation error in call manager
// Once call manager adapt to new interrupt APIs, this will be removed
enum InterruptActionType {
    TYPE_ACTIVATED = 0,
    TYPE_INTERRUPT = 1
};

struct InterruptAction {
    InterruptActionType actionType;
    InterruptType interruptType;
    InterruptHint interruptHint;
    bool activated;
};

enum AudioServiceIndex {
    HDI_SERVICE_INDEX = 0,
    AUDIO_SERVICE_INDEX
};

/**
 * @brief Enumerates the rendering states of the current device.
 */
enum RendererState {
    /** INVALID state */
    RENDERER_INVALID = -1,
    /** Create New Renderer instance */
    RENDERER_NEW,
    /** Reneder Prepared state */
    RENDERER_PREPARED,
    /** Rendere Running state */
    RENDERER_RUNNING,
    /** Renderer Stopped state */
    RENDERER_STOPPED,
    /** Renderer Released state */
    RENDERER_RELEASED,
    /** Renderer Paused state */
    RENDERER_PAUSED
};

/**
 * @brief Enumerates the capturing states of the current device.
 */
enum CapturerState {
    /** Capturer INVALID state */
    CAPTURER_INVALID = -1,
    /** Create new capturer instance */
    CAPTURER_NEW,
    /** Capturer Prepared state */
    CAPTURER_PREPARED,
    /** Capturer Running state */
    CAPTURER_RUNNING,
    /** Capturer Stopped state */
    CAPTURER_STOPPED,
    /** Capturer Released state */
    CAPTURER_RELEASED,
    /** Capturer Paused state */
    CAPTURER_PAUSED
};

enum AudioMode {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

struct AudioRendererChangeInfo {
    int32_t clientUID;
    int32_t sessionId;
    AudioRendererInfo rendererInfo;
    RendererState rendererState;
};

struct AudioCapturerChangeInfo {
    int32_t clientUID;
    int32_t sessionId;
    AudioCapturerInfo capturerInfo;
    CapturerState capturerState;
};

struct AudioStreamChangeInfo {
    AudioRendererChangeInfo audioRendererChangeInfo;
    AudioCapturerChangeInfo audioCapturerChangeInfo;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INFO_H
