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
#include "iaudio_policy.h"
#include "iport_observer.h"
#include "parser_factory.h"

#include <list>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
class AudioPolicyService : public IPortObserver {
public:
    static constexpr char HDI_SINK[] = "hdi_output";
    static constexpr char HDI_SOURCE[] = "hdi_input";
    static constexpr char BLUEZ_SINK[] = "fifo_output";
    static constexpr char BLUEZ_SOURCE[] = "fifo_input";

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

    int32_t SetDeviceActive(DeviceType deviceType, bool active);

    bool IsDeviceActive(DeviceType deviceType) const;

    int32_t SetRingerMode(AudioRingerMode ringMode);

    AudioRingerMode GetRingerMode() const;

    // Parser callbacks
    void OnAudioPortAvailable(std::shared_ptr<AudioPortInfo> portInfo);

    void OnAudioPortPinAvailable(std::shared_ptr<AudioPortPinInfo> portInfo);

    void OnDefaultOutputPortPin(DeviceType device);

    void OnDefaultInputPortPin(DeviceType device);

private:

    AudioPolicyService()
        : mAudioPolicyManager(AudioPolicyManagerFactory::GetAudioPolicyManager()),
          mConfigParser(ParserFactory::GetInstance().CreateParser(*this))
    {
    }

    virtual ~AudioPolicyService() {}

    AudioIOHandle GetAudioIOHandle(DeviceType deviceType);
    std::list<DeviceType>& GetActiveDevicesList(DeviceType deviceType)
    {
        switch (deviceType) {
            case SPEAKER:
            case BLUETOOTH_A2DP:
                return mActiveOutputDevices;
            case MIC:
            case BLUETOOTH_SCO:
                return mActiveInputDevices;
            default:
                return mActiveOutputDevices; // Default case return Output device
        }
    }

    IAudioPolicyInterface& mAudioPolicyManager;
    Parser& mConfigParser;
    std::unordered_map<std::string, AudioIOHandle> mIOHandles;
    std::list<DeviceType> mActiveOutputDevices;
    std::list<DeviceType> mActiveInputDevices;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
