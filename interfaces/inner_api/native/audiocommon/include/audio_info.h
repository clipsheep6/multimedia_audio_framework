/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <unordered_map>
#include <parcel.h>
#include <audio_source_type.h>
#include <audio_device_info.h>
#include <audio_interrupt_info.h>
#include <audio_stream_info.h>
#include <audio_errors.h>

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_NUM_STREAMS = 3;
constexpr int32_t RENDERER_STREAM_USAGE_SHIFT = 16;
constexpr int32_t MINIMUM_BUFFER_SIZE_MSEC = 5;
constexpr int32_t MAXIMUM_BUFFER_SIZE_MSEC = 20;
constexpr int32_t MIN_SERVICE_COUNT = 2;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t INTELL_VOICE_SERVICR_UID = 1042;
constexpr int32_t NETWORK_ID_SIZE = 80;
constexpr int32_t DEFAULT_VOLUME_GROUP_ID = 1;
constexpr int32_t DEFAULT_VOLUME_INTERRUPT_ID = 1;
constexpr uint32_t STREAM_FLAG_FAST = 1;

const std::string MICROPHONE_PERMISSION = "ohos.permission.MICROPHONE";
const std::string MANAGE_INTELLIGENT_VOICE_PERMISSION = "ohos.permission.MANAGE_INTELLIGENT_VOICE";
const std::string MANAGE_AUDIO_CONFIG = "ohos.permission.MANAGE_AUDIO_CONFIG";
const std::string MODIFY_AUDIO_SETTINGS_PERMISSION = "ohos.permission.MODIFY_AUDIO_SETTINGS";
const std::string ACCESS_NOTIFICATION_POLICY_PERMISSION = "ohos.permission.ACCESS_NOTIFICATION_POLICY";
const std::string USE_BLUETOOTH_PERMISSION = "ohos.permission.USE_BLUETOOTH";
const std::string CAPTURER_VOICE_DOWNLINK_PERMISSION = "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO";
const std::string LOCAL_NETWORK_ID = "LocalDevice";
const std::string REMOTE_NETWORK_ID = "RemoteDevice";

constexpr int32_t WAKEUP_LIMIT = 2;
constexpr std::string_view PRIMARY_WAKEUP = "Built_in_wakeup";
constexpr std::string_view PRIMARY_WAKEUP_MIRROR = "Built_in_wakeup_mirror";
constexpr std::string_view WAKEUP_NAMES[WAKEUP_LIMIT] = {
    PRIMARY_WAKEUP,
    PRIMARY_WAKEUP_MIRROR
};

#ifdef FEATURE_DTMF_TONE
// Maximun number of sine waves in a tone segment
constexpr uint32_t TONEINFO_MAX_WAVES = 3;

// Maximun number of segments in a tone descriptor
constexpr uint32_t TONEINFO_MAX_SEGMENTS = 12;
constexpr uint32_t TONEINFO_INF = 0xFFFFFFFF;
class ToneSegment : public Parcelable {
public:
    uint32_t duration;
    uint16_t waveFreq[TONEINFO_MAX_WAVES+1];
    uint16_t loopCnt;
    uint16_t loopIndx;
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};

class ToneInfo : public Parcelable {
public:
    ToneSegment segments[TONEINFO_MAX_SEGMENTS+1];
    uint32_t segmentCnt;
    uint32_t repeatCnt;
    uint32_t repeatSegment;
    ToneInfo() {}
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};
#endif

enum VolumeAdjustType {
    /**
     * Adjust volume up
     */
    VOLUME_UP = 0,
    /**
     * Adjust volume down
     */
    VOLUME_DOWN = 1,
};

enum ChannelBlendMode {
    /**
     * No channel process.
     */
    MODE_DEFAULT = 0,
    /**
     * Blend left and right channel.
     */
    MODE_BLEND_LR = 1,
    /**
     * Replicate left to right channel.
     */
    MODE_ALL_LEFT = 2,
    /**
     * Replicate right to left channel.
     */
    MODE_ALL_RIGHT = 3,
};

enum ConnectType {
    /**
     * Group connect type of local device
     */
    CONNECT_TYPE_LOCAL = 0,
    /**
     * Group connect type of distributed device
     */
    CONNECT_TYPE_DISTRIBUTED
};

typedef AudioStreamType AudioVolumeType;

enum FocusType {
    /**
     * Recording type.
     */
    FOCUS_TYPE_RECORDING = 0,
};

enum API_VERSION {
    API_7 = 7,
    API_8 = 8,
    API_9 = 9
};

// Ringer Mode
enum AudioRingerMode {
    RINGER_MODE_SILENT = 0,
    RINGER_MODE_VIBRATE = 1,
    RINGER_MODE_NORMAL = 2
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
    CONTENT_TYPE_RINGTONE = 5,
    // other ContentType
    CONTENT_TYPE_PROMPT = 6,
    CONTENT_TYPE_GAME = 7,
    CONTENT_TYPE_DTMF = 8,
    CONTENT_TYPE_ULTRASONIC = 9
};

/**
 * Enumerates audio stream privacy type for playback capture.
 */
enum AudioPrivacyType {
    PRIVACY_TYPE_PUBLIC = 0,
    PRIVACY_TYPE_PRIVATE = 1
};

/**
* Enumerates the renderer playback speed.
*/
enum AudioRendererRate {
    RENDER_RATE_NORMAL = 0,
    RENDER_RATE_DOUBLE = 1,
    RENDER_RATE_HALF = 2,
};

enum ActionTarget {
    CURRENT = 0,
    INCOMING,
    BOTH
};

enum AudioFocuState {
    ACTIVE = 0,
    DUCK,
    PAUSE,
    STOP
};

struct AudioFocusEntry {
    InterruptForceType forceType;
    InterruptHint hintType;
    ActionTarget actionOn;
    bool isReject;
};

struct AudioFocusType {
    AudioStreamType streamType;
    SourceType sourceType;
    bool isPlay;
    bool operator==(const AudioFocusType &value) const
    {
        return streamType == value.streamType && sourceType == value.sourceType && isPlay == value.isPlay;
    }

    bool operator<(const AudioFocusType &value) const
    {
        return streamType < value.streamType || (streamType == value.streamType && sourceType < value.sourceType);
    }

    bool operator>(const AudioFocusType &value) const
    {
        return streamType > value.streamType || (streamType == value.streamType && sourceType > value.sourceType);
    }
};

class AudioInterrupt : public Parcelable {
public:
    StreamUsage streamUsage;
    ContentType contentType;
    AudioFocusType audioFocusType;
    uint32_t sessionID;
    bool pauseWhenDucked;
    int32_t pid { -1 };
    InterruptMode mode { SHARE_MODE };

    AudioInterrupt() = default;
    AudioInterrupt(StreamUsage streamUsage_, ContentType contentType_, AudioFocusType audioFocusType_,
        uint32_t sessionID_);
    ~AudioInterrupt() = default;
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};

struct VolumeEvent {
    AudioStreamType volumeType;
    int32_t volume;
    bool updateUi;
    int32_t volumeGroupId;
    std::string networkId;
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

struct AudioRendererInfo {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t rendererFlags = 0;
};

class AudioCapturerInfo : public Parcelable {
public:
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t capturerFlags = 0;
    AudioCapturerInfo(SourceType sourceType_, int32_t capturerFlags_);
    AudioCapturerInfo(const AudioCapturerInfo &audioCapturerInfo);
    AudioCapturerInfo();
    ~AudioCapturerInfo();
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};

struct AudioRendererDesc {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
};

struct AudioRendererOptions {
    AudioStreamInfo streamInfo;
    AudioRendererInfo rendererInfo;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
};

struct MicStateChangeEvent {
    bool mute;
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

struct CaptureFilterOptions {
    std::vector<StreamUsage> usages;
};

struct AudioPlaybackCaptureConfig {
    CaptureFilterOptions filterOptions;
    bool silentCapture {false};
};

struct AudioCapturerOptions {
    AudioStreamInfo streamInfo;
    AudioCapturerInfo capturerInfo;
    AudioPlaybackCaptureConfig playbackCaptureConfig;
};

struct AppInfo {
    int32_t appUid { INVALID_UID };
    uint32_t appTokenId { 0 };
    int32_t appPid { 0 };
    uint64_t appFullTokenId { 0 };
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

struct SinkInfo {
    uint32_t sinkId; // sink id
    std::string sinkName;
    std::string adapterName;
};

struct SinkInput {
    int32_t streamId;
    AudioStreamType streamType;

    // add for routing stream.
    int32_t uid; // client uid
    int32_t pid; // client pid
    uint32_t paStreamId; // streamId
    uint32_t deviceSinkId; // sink id
    std::string sinkName; // sink name
    int32_t statusMark; // mark the router status
    uint64_t startTime; // when this router is created
};

struct SourceOutput {
    int32_t streamId;
    AudioStreamType streamType;

    // add for routing stream.
    int32_t uid; // client uid
    int32_t pid; // client pid
    uint32_t paStreamId; // streamId
    uint32_t deviceSourceId; // sink id
    int32_t statusMark; // mark the router status
    uint64_t startTime; // when this router is created
};

typedef uint32_t AudioIOHandle;

static inline bool FLOAT_COMPARE_EQ(const float& x, const float& y)
{
    return (std::abs((x) - (y)) <= (std::numeric_limits<float>::epsilon()));
}

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

enum State {
    /** INVALID */
    INVALID = -1,
    /** New */
    NEW,
    /** Prepared */
    PREPARED,
    /** Running */
    RUNNING,
    /** Stopped */
    STOPPED,
    /** Released */
    RELEASED,
    /** Paused */
    PAUSED,
    /** Stopping */
    STOPPING
};

enum StateChangeCmdType {
    CMD_FROM_CLIENT = 0,
    CMD_FROM_SYSTEM = 1
};

enum AudioMode {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

struct AudioProcessConfig {
    AppInfo appInfo;

    AudioStreamInfo streamInfo;

    AudioMode audioMode;

    AudioRendererInfo rendererInfo;

    AudioCapturerInfo capturerInfo;

    AudioStreamType streamType;
};

struct Volume {
    bool isMute = false;
    float volumeFloat = 1.0f;
    uint32_t volumeInt = 0;
};

enum StreamSetState {
    STREAM_PAUSE,
    STREAM_RESUME
};

struct StreamSetStateEventInternal {
    StreamSetState streamSetState;
    AudioStreamType audioStreamType;
};

class AudioRendererChangeInfo : Parcelable {
public:
    int32_t createrUID;
    int32_t clientUID;
    int32_t sessionId;
    int32_t tokenId;
    AudioRendererInfo rendererInfo;
    RendererState rendererState;
    DeviceInfo outputDeviceInfo;

    AudioRendererChangeInfo(const AudioRendererChangeInfo &audioRendererChangeInfo);
    AudioRendererChangeInfo();
    ~AudioRendererChangeInfo();
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};

class AudioCapturerChangeInfo : Parcelable {
public:
    int32_t createrUID;
    int32_t clientUID;
    int32_t sessionId;
    AudioCapturerInfo capturerInfo;
    CapturerState capturerState;
    DeviceInfo inputDeviceInfo;
    bool muted;

    AudioCapturerChangeInfo(const AudioCapturerChangeInfo &audioCapturerChangeInfo);
    AudioCapturerChangeInfo();
    ~AudioCapturerChangeInfo();
    bool Marshalling(Parcel &parcel) const override;
    void Unmarshalling(Parcel &parcel);
};

struct AudioStreamChangeInfo {
    AudioRendererChangeInfo audioRendererChangeInfo;
    AudioCapturerChangeInfo audioCapturerChangeInfo;
};

enum AudioPin {
    AUDIO_PIN_NONE = 0, // Invalid pin
    AUDIO_PIN_OUT_SPEAKER = 1 << 0, // Speaker output pin
    AUDIO_PIN_OUT_HEADSET = 1 << 1, // Wired headset pin for output
    AUDIO_PIN_OUT_LINEOUT = 1 << 2, // Line-out pin
    AUDIO_PIN_OUT_HDMI = 1 << 3, // HDMI output pin
    AUDIO_PIN_OUT_USB = 1 << 4, // USB output pin
    AUDIO_PIN_OUT_USB_EXT = 1 << 5, // Extended USB output pin
    AUDIO_PIN_OUT_BLUETOOTH_SCO = 1 << 6, // Bluetooth SCO output pin
    AUDIO_PIN_OUT_DAUDIO_DEFAULT = 1 << 7, // Daudio default output pin
    AUDIO_PIN_OUT_HEADPHONE = 1 << 8, // Wired headphone output pin
    AUDIO_PIN_OUT_USB_HEADSET = 1 << 9,  // Arm usb output pin
    AUDIO_PIN_IN_MIC = 1 << 27 | 1 << 0, // Microphone input pin
    AUDIO_PIN_IN_HS_MIC = 1 << 27 | 1 << 1, // Wired headset microphone pin for input
    AUDIO_PIN_IN_LINEIN = 1 << 27 | 1 << 2, // Line-in pin
    AUDIO_PIN_IN_USB_EXT = 1 << 27 | 1 << 3, // Extended USB input pin
    AUDIO_PIN_IN_BLUETOOTH_SCO_HEADSET = 1 << 27 | 1 << 4, // Bluetooth SCO headset input pin
    AUDIO_PIN_IN_DAUDIO_DEFAULT = 1 << 27 | 1 << 5, // Daudio default input pin
    AUDIO_PIN_IN_USB_HEADSET = 1 << 27 | 1 << 6,  // Arm usb input pin
};

enum AudioParamKey {
    NONE = 0,
    VOLUME = 1,
    INTERRUPT = 2,
    RENDER_STATE = 5,
    A2DP_SUSPEND_STATE = 6,  // for bluetooth sink
    BT_HEADSET_NREC = 7,
    BT_WBS = 8,
    USB_DEVICE = 101, // Check USB device type ARM or HIFI
    PARAM_KEY_LOWPOWER = 1000,
};

struct DStatusInfo {
    char networkId[NETWORK_ID_SIZE];
    AudioPin hdiPin = AUDIO_PIN_NONE;
    int32_t mappingVolumeId = 0;
    int32_t mappingInterruptId = 0;
    int32_t deviceId;
    int32_t channelMasks;
    std::string deviceName = "";
    bool isConnected = false;
    std::string macAddress;
    AudioStreamInfo streamInfo = {};
    ConnectType connectType = CONNECT_TYPE_LOCAL;
};

struct AudioRendererDataInfo {
    uint8_t *buffer;
    size_t flag;
};

enum AudioPermissionState {
    AUDIO_PERMISSION_START = 0,
    AUDIO_PERMISSION_STOP = 1,
};

class AudioRendererPolicyServiceDiedCallback {
public:
    virtual ~AudioRendererPolicyServiceDiedCallback() = default;

    /**
     * Called when audio policy service died.
     * @since 10
     */
    virtual void OnAudioPolicyServiceDied() = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INFO_H
