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

#ifndef ST_AUDIO_SPATIALIZATION_MANAGER_H
#define ST_AUDIO_SPATIALIZATION_MANAGER_H

#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"
#include "audio_interrupt_callback.h"
#include "audio_group_manager.h"
#include "audio_routing_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceDescriptor;
class AudioDeviceDescriptor : public Parcelable {
    friend class AudioSystemManager;
public:
    DeviceType getType();
    DeviceRole getRole();
    DeviceType deviceType_;
    DeviceRole deviceRole_;
    int32_t deviceId_;
    int32_t channelMasks_;
    int32_t channelIndexMasks_;
    std::string deviceName_;
    std::string macAddress_;
    int32_t interruptGroupId_;
    int32_t volumeGroupId_;
    std::string networkId_;
    std::string displayName_;
    AudioStreamInfo audioStreamInfo_ = {};

    AudioDeviceDescriptor();
    AudioDeviceDescriptor(DeviceType type, DeviceRole role, int32_t interruptGroupId, int32_t volumeGroupId,
        std::string networkId);
    AudioDeviceDescriptor(DeviceType type, DeviceRole role);
    AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor);
    AudioDeviceDescriptor(const sptr<AudioDeviceDescriptor> &deviceDescriptor);
    virtual ~AudioDeviceDescriptor();

    bool Marshalling(Parcel &parcel) const override;
    static sptr<AudioDeviceDescriptor> Unmarshalling(Parcel &parcel);
    void SetDeviceInfo(std::string deviceName, std::string macAddress);
    void SetDeviceCapability(const AudioStreamInfo &audioStreamInfo, int32_t channelMask,
        int32_t channelIndexMasks = 0);
};

class AudioFocusInfoChangeCallback {
public:
    virtual ~AudioFocusInfoChangeCallback() = default;
    /**
     * Called when focus info change.
     *
     * @param focusInfoList Indicates the focusInfoList information needed by client.
     * For details, refer audioFocusInfoList_ struct in audio_policy_server.h
     * @since 9
     */
    virtual void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) = 0;
};

class AudioFocusInfoChangeCallbackImpl : public AudioFocusInfoChangeCallback {
public:
    explicit AudioFocusInfoChangeCallbackImpl();
    virtual ~AudioFocusInfoChangeCallbackImpl();

    /**
     * Called when focus info change.
     *
     * @param focusInfoList Indicates the focusInfoList information needed by client.
     * For details, refer audioFocusInfoList_ struct in audio_policy_server.h
     * @since 9
     */
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override;
    void SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);

    /**
     *  Cancel when focus info change.
     *
     * @since 9
     */
    void RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback);
private:
    std::list<std::weak_ptr<AudioFocusInfoChangeCallback>> callbackList_;
    std::shared_ptr<AudioFocusInfoChangeCallback> cb_;
};

/**
 * @brief The AudioSystemManager class is an abstract definition of audio manager.
 *        Provides a series of client/interfaces for audio management
 */

class AudioSystemManager {
public:
    static AudioSystemManager *GetInstance();

    /**
     * @brief Get volume groups manager
     *
     * @param networkId networkId
     * @return Returns AudioGroupManager
     * @since 8
     */
    std::shared_ptr<AudioGroupManager> GetGroupManager(int32_t groupId);

    /**
     * @brief Get active output deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 8
     */
    std::vector<sptr<AudioDeviceDescriptor>> GetActiveOutputDeviceDescriptors();

    /**
     * @brief Get preferred input device deviceDescriptors
     *
     * @return Returns AudioDeviceDescriptor
     * @since 10
     */
    int32_t GetPreferredInputDeviceDescriptors();

    /**
     * @brief Get audio focus info
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);

    /**
     * @brief Register callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t RegisterFocusInfoChangeCallback(const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    /**
     * @brief Unregister callback to listen audio focus info change event
     *
     * @return Returns success or not
     * @since 10
     */
    int32_t UnregisterFocusInfoChangeCallback(
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback = nullptr);

    /**
     * @brief Ask audio native process to request thread priority for client
     *
     * @param tid Target thread id
     * @since 10
     */
    void RequestThreadPriority(uint32_t tid);

    int32_t SetAudioCapturerSourceCallback(const std::shared_ptr<AudioCapturerSourceCallback> &callback);

    int32_t SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback> &callback);

    /**
     * @brief Set whether or not absolute volume is supported for the specified Bluetooth device
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

    /**
     * @brief Set the absolute volume value for the specified Bluetooth device
     *
     * @return Returns success or not
     * @since 11
     */
    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi);
private:
    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t CONST_FACTOR = 100;
    static const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_;

    AudioSystemManager();
    virtual ~AudioSystemManager();

    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();
    uint32_t GetCallingPid();
    std::string GetSelfBundleName();

    int32_t RegisterWakeupSourceCallback();

    int32_t cbClientId_ = -1;
    int32_t volumeChangeClientPid_ = -1;
    AudioRingerMode ringModeBackup_ = RINGER_MODE_NORMAL;
    std::shared_ptr<AudioManagerDeviceChangeCallback> deviceChangeCallback_ = nullptr;
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;
    std::shared_ptr<AudioRingerModeCallback> ringerModeCallback_ = nullptr;
    std::shared_ptr<AudioFocusInfoChangeCallback> audioFocusInfoCallback_ = nullptr;
    std::vector<std::shared_ptr<AudioGroupManager>> groupManagerMap_;
    std::mutex ringerModeCallbackMutex_;

    std::shared_ptr<AudioCapturerSourceCallback> audioCapturerSourceCallback_ = nullptr;
    std::shared_ptr<WakeUpSourceCloseCallback> audioWakeUpSourceCloseCallback_ = nullptr;

    std::shared_ptr<WakeUpCallbackImpl> remoteWakeUpCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SPATIALIZATION_MANAGER_H
