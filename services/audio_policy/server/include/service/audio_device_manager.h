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
#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "parser_factory.h"
#include "audio_effect_manager.h"
#include "audio_volume_config.h"
#include "policy_provider_stub.h"
//#include "audio_policy_service.h"
#include "audio_device_info.h"
// #include "audio_device_parser.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceManager {
public:
    // static AudioDeviceManager& GetAudioDeviceManager()
    // {
    //     static AudioDeviceManager audioDeviceManager;
    //     return audioDeviceManager;
    // }
    AudioDeviceManager() {};
    ~AudioDeviceManager() {};
    std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> devicePrivacyMaps_ = {};
    // 远程播放设备

    void AddNewDevice(const AudioDeviceDescriptor &devDesc);

 // void RemoveNewDevice (const AudioDeviceDescriptor &devDesc);

    void OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData);

    int32_t GetDeviceUsageFromType(const DeviceType devType)const;

    void ParseDeviceXml();

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetRemoteRenderDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetRemoteCaptureDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCommRenderPrivacyDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCommRenderPublicDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCommCapturePrivacyDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCommCapturePublicDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> GetMediaRenderDevices() const;

    std::vector<unique_ptr<AudioDeviceDescriptor>> GetMediaCaptureDevices() const;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCapturePrivacyDevices() ;

    std::vector<unique_ptr<AudioDeviceDescriptor>> &GetCapturePublicDevices() ;

private:
    // 远程播放设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> remoteRenderDevices_;
    // 远程录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> remoteCaptureDevices_;
    // 隐私通话设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> commRenderPrivacyDevices_;
    // 公共通话设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> commRenderPublicDevices_;
    // 隐私通话录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> commCapturePrivacyDevices_;
    // 公共通话录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> commCapturePublicDevices_;
    // 私有媒体设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaPrivacyDevices_;
    // 公共媒体设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaPublicDevices_;
    // 私有录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> capturePrivacyDevices_;
    // 公共录制设备
    std::vector<unique_ptr<AudioDeviceDescriptor>> capturePublicDevices_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //ST_AUDIO_DEVICE_MANAGER_H