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

    // 远程播放设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> RemoteRenderDevices;

    // 远程录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> RemoteCaptureDevices;

    // 隐私通话设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CommRenderPrivacyDevices;

    // 公共通话设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CommRenderPublicDevices;

    // 隐私通话录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CommCapturePrivacyDevices;

    // 公共通话录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CommCapturePublicDevices;

    // 私有媒体设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> MediaPrivacyDevices;

    // 公共媒体设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> MediaPublicDevices;

    // 私有录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CapturePrivacyDevices;

    // 公共录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> CapturePublicDevices;


    void AddNewDevice(const AudioDeviceDescriptor &devDesc);

    void RemoveNewDevice (const AudioDeviceDescriptor &devDesc);

    void OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData);

    int32_t GetDeviceUsageFromType(const DeviceType devType)const;

    std::vector<unique_ptr<AudioDeviceDescriptor>> getRemoteRenderDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getRemoteCaptureDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCommRenderPrivacyDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCommRenderPublicDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCommCapturePrivacyDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCommCapturePublicDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getMediaRenderDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getMediaCaptureDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCapturePrivacyDevices();

    std::vector<unique_ptr<AudioDeviceDescriptor>> getCapturePublicDevices();

};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H