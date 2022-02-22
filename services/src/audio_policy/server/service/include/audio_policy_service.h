/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_POLICY_SERVICE_H
#define ST_AUDIO_POLICY_SERVICE_H

#include "audio_info.h"
#include "audio_policy_manager_factory.h"
#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "parser_factory.h"

#include <list>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
class AudioPolicyService : public IPortObserver, public IDeviceStatusObserver {
public:
    static AudioPolicyService& GetAudioPolicyService()
    {
        static AudioPolicyService audioPolicyService;
        return audioPolicyService;
    }

    bool Init(void);
    void Deinit(void);

    int32_t SetStreamVolume(AudioStreamType streamType, float volume) const;

    float GetStreamVolume(AudioStreamType streamType) const;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute) const;

    bool GetStreamMute(AudioStreamType streamType) const;

    bool IsStreamActive(AudioStreamType streamType) const;

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType) const;

    int32_t SetRingerMode(AudioRingerMode ringMode);

    bool IsAudioInterruptEnabled() const;

    auto& GetAudioFocusTable() const
    {
        return focusTable_;
    }
    AudioRingerMode GetRingerMode() const;

    int32_t SetAudioScene(AudioScene audioScene);

    AudioScene GetAudioScene() const;

    // Parser callbacks
    void OnAudioPortAvailable(const AudioModuleInfo &moduleInfo);

    void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata);

    void OnAudioInterruptEnable(bool enable);

    void OnDeviceStatusUpdated(DeviceType deviceType, bool connected, void *privData);
    void OnServiceConnected();

    int32_t SetAudioSessionCallback(AudioSessionCallback *callback);

    int32_t SetDeviceChangeCallback(const sptr<IRemoteObject> &object);
private:

    AudioPolicyService()
        : mAudioPolicyManager(AudioPolicyManagerFactory::GetAudioPolicyManager()),
          mConfigParser(ParserFactory::GetInstance().CreateParser(*this))
    {
        mDeviceStatusListener = std::make_unique<DeviceStatusListener>(*this);
    }

    virtual ~AudioPolicyService() {}

    AudioIOHandle GetAudioIOHandle(InternalDeviceType deviceType);
    InternalDeviceType GetDeviceType(const std::string &deviceName);
    void TriggerDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);

    DeviceRole GetDeviceRole(DeviceType deviceType)
    {
        switch (deviceType) {
            case DeviceType::DEVICE_TYPE_SPEAKER:
            case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
                return DeviceRole::OUTPUT_DEVICE;
            case DeviceType::DEVICE_TYPE_MIC:
                return DeviceRole::INPUT_DEVICE;
            default:
                return DeviceRole::DEVICE_ROLE_NONE; // Default case return Output device
        }
    }

    DeviceRole GetDeviceRole(const std::string &role)
    {
        if (role == ROLE_SINK) {
            return DeviceRole::OUTPUT_DEVICE;
        } else if (role == ROLE_SOURCE) {
            return DeviceRole::INPUT_DEVICE;
        } else {
            return DeviceRole::DEVICE_ROLE_NONE;
        }
    }

    IAudioPolicyInterface& mAudioPolicyManager;
    Parser& mConfigParser;
    std::unique_ptr<DeviceStatusListener> mDeviceStatusListener;
    std::unordered_map<std::string, AudioIOHandle> mIOHandles;
    std::vector<sptr<AudioDeviceDescriptor>> mActiveDevices;
    std::list<sptr<IStandardAudioPolicyManagerListener>> deviceChangeCallbackList_;
    std::string GetPortName(InternalDeviceType deviceType);
    bool interruptEnabled_ = true;
    AudioScene mAudioScene = AUDIO_SCENE_DEFAULT;
    AudioFocusEntry focusTable_[MAX_NUM_STREAMS][MAX_NUM_STREAMS];
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo_ = {};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
