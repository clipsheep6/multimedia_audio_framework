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

#ifndef AUDIO_SYSTEM_UTILS_H
#define AUDIO_SYSTEM_UTILS_H

#include <cstdlib>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"
#include "audio_interrupt_callback.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceDescriptor;
class AudioDeviceDescriptor : public Parcelable {
    friend class AudioSystemManager;
public:
    AudioDeviceDescriptor();
    AudioDeviceDescriptor(DeviceType type, DeviceRole role,
        int32_t interruptGroupId, int32_t volumeGroupId, std::string networkId);
    AudioDeviceDescriptor(DeviceType type, DeviceRole role);
    AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor);
    virtual ~AudioDeviceDescriptor();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioDeviceDescriptor> Unmarshalling(Parcel &parcel);

    DeviceType getType();
    DeviceRole getRole();
    void SetDeviceInfo(std::string deviceName, std::string macAddress);
    void SetDeviceCapability(const AudioStreamInfo &audioStreamInfo, int32_t channelMask);

    DeviceType deviceType_;
    DeviceRole deviceRole_;
    int32_t deviceId_;
    int32_t channelMasks_;
    std::string deviceName_;
    std::string macAddress_;
    int32_t interruptGroupId_;
    int32_t volumeGroupId_;
    std::string networkId_;
    AudioStreamInfo audioStreamInfo_ = {};
};

class InterruptGroupInfo;
class InterruptGroupInfo : public Parcelable {
    friend class AudioSystemManager;
public:
    InterruptGroupInfo();
    InterruptGroupInfo(int32_t interruptGroupId, int32_t mappingId, std::string groupName, std::string networkId,
        ConnectType type);
    virtual ~InterruptGroupInfo();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<InterruptGroupInfo> Unmarshalling(Parcel &parcel);

    int32_t interruptGroupId_;
    int32_t mappingId_;
    std::string groupName_;
    std::string networkId_;
    ConnectType connectType_;
};

class VolumeGroupInfo;
class VolumeGroupInfo : public Parcelable {
    friend class AudioSystemManager;
public:
    VolumeGroupInfo();
    VolumeGroupInfo(int32_t volumeGroupId, int32_t mappingId, std::string groupName, std::string networkId,
        ConnectType type);
    virtual ~VolumeGroupInfo();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<VolumeGroupInfo> Unmarshalling(Parcel &parcel);

    int32_t volumeGroupId_;
    int32_t mappingId_;
    std::string groupName_;
    std::string networkId_;
    ConnectType connectType_;
};

struct DeviceChangeAction {
    DeviceChangeType type;
    std::vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
};

/**
 * @brief AudioRendererFilter is used for select speficed AudioRenderer.
 */
class AudioRendererFilter;
class AudioRendererFilter : public Parcelable {
    friend class AudioSystemManager;
public:
    AudioRendererFilter();
    virtual ~AudioRendererFilter();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioRendererFilter> Unmarshalling(Parcel &in);

    int32_t uid = -1;
    AudioRendererInfo rendererInfo = {};
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    int32_t streamId = -1;
};

/**
 * @brief AudioCapturerFilter is used for select speficed audiocapturer.
 */
class AudioCapturerFilter;
class AudioCapturerFilter : public Parcelable {
    friend class AudioSystemManager;
public:
    AudioCapturerFilter();
    virtual ~AudioCapturerFilter();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioCapturerFilter> Unmarshalling(Parcel &in);

    int32_t uid = -1;
};

// AudioManagerCallback OnInterrupt is added to handle compilation error in call manager
// Once call manager adapt to new interrupt APIs, this will be removed
class AudioManagerCallback {
public:
    virtual ~AudioManagerCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param interruptAction Indicates the InterruptAction information needed by client.
     * For details, refer InterruptAction struct in audio_info.h
     */
    virtual void OnInterrupt(const InterruptAction &interruptAction) = 0;
};

class AudioManagerInterruptCallbackImpl : public AudioInterruptCallback {
public:
    explicit AudioManagerInterruptCallbackImpl();
    virtual ~AudioManagerInterruptCallbackImpl();

    void OnInterrupt(const InterruptEventInternal &interruptEvent) override;
    void SaveCallback(const std::weak_ptr<AudioManagerCallback> &callback);
private:
    std::weak_ptr<AudioManagerCallback> callback_;
    std::shared_ptr<AudioManagerCallback> cb_;
};

class AudioManagerDeviceChangeCallback {
public:
    virtual ~AudioManagerDeviceChangeCallback() = default;
    /**
     * Called when an interrupt is received.
     *
     * @param deviceChangeAction Indicates the DeviceChangeAction information needed by client.
     * For details, refer DeviceChangeAction struct
     */
    virtual void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) = 0;
};

class VolumeKeyEventCallback {
public:
    virtual ~VolumeKeyEventCallback() = default;
    /**
     * @brief VolumeKeyEventCallback will be executed when hard volume key is pressed up/down
     *
     * @param volumeEvent the volume event info.
    **/
    virtual void OnVolumeKeyEvent(VolumeEvent volumeEvent) = 0;
};

class AudioParameterCallback {
public:
    virtual ~AudioParameterCallback() = default;
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) = 0;
};

class AudioRingerModeCallback {
public:
    virtual ~AudioRingerModeCallback() = default;
    /**
     * Called when ringer mode is updated.
     *
     * @param ringerMode Indicates the updated ringer mode value.
     * For details, refer RingerMode enum in audio_info.h
     */
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) = 0;
};

class AudioSystemEventCallback {
public:
    AudioSystemEventCallback() = default;
    virtual ~AudioSystemEventCallback() = default;
    // can be extended to support common events
    virtual void OnSystemEvent(const AudioRingerMode &ringerMode);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SYSTEM_UTILS_H
