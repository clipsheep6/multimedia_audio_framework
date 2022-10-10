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

#ifndef ST_AUDIO_SYSTEM_MANAGER_H
#define ST_AUDIO_SYSTEM_MANAGER_H

#include <cstdlib>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "event_handler.h"
#include "event_runner.h"
#include "audio_info.h"
#include "audio_system_utils.h"
#include "audio_interrupt_callback.h"
#include "audio_group_manager.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief The AudioSystemManager class is an abstract definition of audio manager.
 *        Provides a series of client/interfaces for audio management
 */
class AudioSystemManager : public AppExecFwk::EventHandler {
public:
    const std::vector<AudioVolumeType> GET_STREAM_ALL_VOLUME_TYPES {
        STREAM_MUSIC,
        STREAM_RING,
        STREAM_VOICE_CALL,
        STREAM_VOICE_ASSISTANT
    };
    static AudioSystemManager *GetInstance();
    static float MapVolumeToHDI(int32_t volume);
    static int32_t MapVolumeFromHDI(float volume);
    static AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
    int32_t SetVolume(AudioVolumeType volumeType, int32_t volume) const;
    int32_t GetVolume(AudioVolumeType volumeType) const;
    int32_t SetLowPowerVolume(int32_t streamId, float volume) const;
    float GetLowPowerVolume(int32_t streamId) const;
    float GetSingleStreamVolume(int32_t streamId) const;
    int32_t GetMaxVolume(AudioVolumeType volumeType);
    int32_t GetMinVolume(AudioVolumeType volumeType);
    int32_t SetMute(AudioVolumeType volumeType, bool mute) const;
    bool IsStreamMute(AudioVolumeType volumeType) const;
    int32_t SetMicrophoneMute(bool isMute);
    bool IsMicrophoneMute(void);
    int32_t SelectOutputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;
    int32_t SelectInputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;
    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const;
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const;
    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    const std::string GetAudioParameter(const std::string key);
    void SetAudioParameter(const std::string &key, const std::string &value);
    const char *RetrieveCookie(int32_t &size);
    uint64_t GetTransactionId(DeviceType deviceType, DeviceRole deviceRole);
    int32_t SetDeviceActive(ActiveDeviceType deviceType, bool flag) const;
    bool IsDeviceActive(ActiveDeviceType deviceType) const;
    DeviceType GetActiveOutputDevice();
    DeviceType GetActiveInputDevice();
    bool IsStreamActive(AudioVolumeType volumeType) const;
    int32_t SetRingerMode(AudioRingerMode ringMode) const;
    AudioRingerMode GetRingerMode() const;
    int32_t SetAudioScene(const AudioScene &scene);
    AudioScene GetAudioScene() const;
    int32_t SetDeviceChangeCallback(const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>
        &callback);
    int32_t UnsetDeviceChangeCallback();
    int32_t SetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback);
    int32_t UnsetRingerModeCallback(const int32_t clientId);
    int32_t RegisterVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback);
    int32_t UnregisterVolumeKeyEventCallback(const int32_t clientPid);

    // Below APIs are added to handle compilation error in call manager
    // Once call manager adapt to new interrupt APIs, this will be removed
    int32_t SetAudioManagerCallback(const AudioVolumeType streamType,
        const std::shared_ptr<AudioManagerCallback> &callback);
    int32_t UnsetAudioManagerCallback(const AudioVolumeType streamType) const;
    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt);
    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt) const;
    int32_t SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback);
    int32_t UnsetAudioManagerInterruptCallback();
    int32_t RequestAudioFocus(const AudioInterrupt &audioInterrupt);
    int32_t AbandonAudioFocus(const AudioInterrupt &audioInterrupt);
    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);
    bool RequestIndependentInterrupt(FocusType focusType);
    bool AbandonIndependentInterrupt(FocusType focusType);
    int32_t GetAudioLatencyFromXml() const;
    uint32_t GetSinkLatencyFromXml() const;
    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
        AudioStreamType audioStreamType);
    AudioPin GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const;
    DeviceType GetTypeValueFromPin(AudioPin pin) const;
    std::vector<sptr<VolumeGroupInfo>> GetVolumeGroups(std::string networkId);
    std::shared_ptr<AudioGroupManager> GetGroupManager(int32_t groupId);
protected:
    virtual void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
private:
    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t CONST_FACTOR = 100;
    enum {
        RINGER_MODE_CHANGE_EVENT = 0,
    };

    AudioSystemManager();
    virtual ~AudioSystemManager();
    void init();
    bool IsAlived();
    uint32_t GetCallingPid();
    void DispatchRingerModeChangeEvent();

    static const std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> streamTypeMap_;
    static std::map<std::pair<ContentType, StreamUsage>, AudioStreamType> CreateStreamMap();

    int32_t cbClientId_ = -1;
    int32_t volumeChangeClientPid_ = -1;
    std::shared_ptr<AudioManagerDeviceChangeCallback> deviceChangeCallback_ = nullptr;
    std::shared_ptr<AudioInterruptCallback> audioInterruptCallback_ = nullptr;

    std::mutex mutex_;
    std::vector<std::shared_ptr<AudioGroupManager>> groupManagerMap_;
    std::vector<std::shared_ptr<AudioRingerModeCallback>> ringerModeCbs_;

    std::shared_ptr<AppExecFwk::EventRunner> mainRunner_ = nullptr;
    std::shared_ptr<AudioSystemEventCallback> eventCallback_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SYSTEM_MANAGER_H
