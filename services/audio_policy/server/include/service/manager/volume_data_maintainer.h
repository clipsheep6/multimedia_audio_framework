/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef VOLUME_DATA_MAINTAINER_H
#define VOLUME_DATA_MAINTAINER_H

#include <list>
#include <unordered_map>
#include <cinttypes>

#include "os_account_manager.h"
#include "ipc_skeleton.h"
#include "datashare_helper.h"
#include "errors.h"
#include "mutex"
#include "data_ability_observer_stub.h"

#include "audio_log.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t NUM_TEN = 10;
constexpr int32_t MIN_USER_ACCOUNT = 100;
class AudioSettingsProvider;
class VolumeDataMainTainer {
public:
    enum VolumeDataMainTainerStreamType {  // 对标双框架流定义进行区分
        VT_STREAM_DEFAULT = -1,
        VT_STREAM_VOICE_CALL = 0,
        VT_STREAM_SYSTEM  = 1,
        VT_STREAM_RING = 2,
        VT_STREAM_MUSIC  = 3,
        VT_STREAM_ALARM = 4,
        VT_STREAM_NOTIFICATION = 5,
        VT_STREAM_BLUETOOTH_SCO = 6,
        VT_STREAM_SYSTEM_ENFORCED = 7,
        VT_STREAM_DTMF = 8,
        VT_STREAM_TTS = 9,
        VT_STREAM_ACCESSIBILITY = 10,
        VT_STREAM_ASSISTANT = 11,
    };

    enum SafeStatus : int32_t {
        SAFE_UNKNOWN = -1,
        SAFE_INACTIVE = 0,
        SAFE_ACTIVE = 1,
    };

    static VolumeDataMainTainer& GetVolumeDataMainTainer()
    {
        static VolumeDataMainTainer volumeDataMainTainer;
        return volumeDataMainTainer;
    }
    ~VolumeDataMainTainer();

    bool SetFirstBoot(bool fristBoot);
    bool GetFirstBoot(bool &firstBoot);

    bool SaveVolume(DeviceType type, AudioStreamType streamType, int32_t volumeLevel);
    bool GetVolume(DeviceType deviceType, AudioStreamType streamType,
        std::unordered_map<AudioStreamType, int32_t> &volumeLevelMap);
    bool SaveMuteStatus(DeviceType deviceType, AudioStreamType streamType,
        bool muteStatus);
    bool GetMuteStatus(DeviceType deviceType, AudioStreamType streamType);
    bool GetStreamMute(AudioStreamType streamType);
    void UpdateMuteStatusForVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel);

    bool GetMuteAffected(int32_t &affected);
    bool GetMuteTransferStatus(bool &status);
    bool SetMuteAffectedToMuteStatusDataBase(int32_t affected);
    bool SaveMuteTransferStatus(bool status);

    bool SaveRingerMode(AudioRingerMode ringerMode);
    bool GetRingerMode(AudioRingerMode &ringerMode);
    bool SaveSafeStatus(DeviceType deviceType, SafeStatus safeStatus);
    bool GetSafeStatus(DeviceType deviceType, SafeStatus &safeStatus);
    bool SaveSafeVolumeTime(DeviceType deviceType, int64_t time);
    bool GetSafeVolumeTime(DeviceType deviceType, int64_t &time);

private:
    VolumeDataMainTainer();
    std::string GetVolumeKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    std::string GetMuteKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    AudioStreamType GetStreamForVolumeMap(AudioStreamType streamType);
    std::string GetDeviceTypeName(DeviceType deviceType);

    std::unordered_map<AudioStreamType, bool> muteStatusMap_; // save voumemute map
};

class AudioSettingObserver : public AAFwk::DataAbilityObserverStub {
public:
    AudioSettingObserver() = default;
    ~AudioSettingObserver() = default;
    void OnChange() override;
    void SetKey(const std::string& key);
    const std::string& GetKey();

    using UpdateFunc = std::function<void(const std::string&)>;
    void SetUpdateFunc(UpdateFunc& func);

private:
    std::string key_ {};
    UpdateFunc update_ = nullptr;
};

class AudioSettingProvider : public NoCopyable {
public:
    static AudioSettingProvider& GetInstance(int32_t systemAbilityId);
    ErrCode GetStringValue(const std::string& key, std::string& value, std::string tableType = "");
    ErrCode GetIntValue(const std::string& key, int32_t& value, std::string tableType = "");
    ErrCode GetLongValue(const std::string& key, int64_t& value, std::string tableType = "");
    ErrCode GetBoolValue(const std::string& key, bool& value, std::string tableType = "");
    ErrCode PutStringValue(const std::string& key, const std::string& value,
        std::string tableType = "", bool needNotify = true);
    ErrCode PutIntValue(const std::string& key, int32_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutLongValue(const std::string& key, int64_t value, std::string tableType = "", bool needNotify = true);
    ErrCode PutBoolValue(const std::string& key, bool value, std::string tableType = "", bool needNotify = true);
    bool IsValidKey(const std::string& key);
    sptr<AudioSettingObserver> CreateObserver(const std::string& key, AudioSettingObserver::UpdateFunc& func);
    static void ExecRegisterCb(const sptr<AudioSettingObserver>& observer);
    ErrCode RegisterObserver(const sptr<AudioSettingObserver>& observer);
    ErrCode UnregisterObserver(const sptr<AudioSettingObserver>& observer);

protected:
    ~AudioSettingProvider() override;

private:
    static AudioSettingProvider* instance_;
    static std::mutex mutex_;
    static sptr<IRemoteObject> remoteObj_;
    static std::string SettingSystemUrlProxy_;

    static void Initialize(int32_t systemAbilityId);
    static std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper(std::string tableType = "");
    static bool ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper>& helper);
    static Uri AssembleUri(const std::string& key, std::string tableType = "");
    static int32_t GetCurrentUserId();
};
} // namespace AudioStandard
} // namespace OHOS
#endif // VOLUME_DATA_MAINTAINER_H