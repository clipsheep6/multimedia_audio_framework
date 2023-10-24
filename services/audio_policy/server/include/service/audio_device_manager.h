#ifndef ST_AUDIO_DEVICE_MANAGER_H
#define ST_AUDIO_DEVICE_MANAGER_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"
#include "ipc_skeleton.h"

#include "iaudio_accessibility_config_observer.h"
#endif
#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "parser_factory.h"
#include "audio_effect_manager.h"
#include "audio_volume_config.h"
#include "policy_provider_stub.h"
#include "audio_policy_service.h"
#include "audio_device_config.h"

namespace OHOS {
namespace AudioStandard {
Class AudioDeviceManager {
public:
    static AudioDeviceManager& GetAudioDeviceManager()
    {
        static AudioDeviceManager audioDeviceManager;
        return audioDeviceManager;
    }

    std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};

    // 所有蓝牙设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> bluetoothDevices_;

    // 所有远程设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> remoteDevices_;

    // 所有通话设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> communicationDevices_;

    // 所有媒体设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaDevices_;

    // 所有录音设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> captureDevices_;

    // 所有隐私设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> privacyDevices_;

    // 所有公共属性设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> publicDevices_;

    // 所有默认设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> defaultDevices_;


    void AddNewDevice(const AudioDeviceDescriptor &devDesc);

    void RemoveNewDevice (const AudioDeviceDescriptor &devDesc);

    void OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData);

    int32_t GetDeviceUsageFromType(const DeviceType devType)const;

    void GetDeviceForPrivacyWithType(const DeviceType devType) const

    // 非通话的隐私设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetPrivacyDevices();

    // 非通话的公共属性设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetPublicDevices();

    // 非通话的默认设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetDefautlDevices();

    // 非通话的所有设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices();

    // 通话的隐私设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetPrivacyComDevices();

    // 通话的公共属性设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetPublicComDevices();

    // 通话的默认设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetDefautlComDevices();

    // 通话的所有设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableComDevices();

    // 录音的所有设备列表
    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableRecordDevices();

};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H