#include "audio_device_manager.h"

#include "ipc_skeleton.h"
#include "hisysevent.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "parameter.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "audio_focus_parser.h"
#include "audio_manager_listener_stub.h"
#include "audio_device_manager.h"
#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "data_share_observer_callback.h"
#include "device_manager.h"
#include "device_init_callback.h"
#include "device_manager_impl.h"
#include "uri.h"
#include <random>

namespace OHOS {
namespace AudioStandard {
using namespace std;

int32_t VOICE_MEDIA = 3; 

// 远程播放设备
std::vector<unique_ptr<AudioDeviceDescriptor>> RemoteRenderDevices = {};

// 远程录制设备
std::vector<unique_ptr<AudioDeviceDescriptor>> RemoteCaptureDevices = {};

// 隐私通话设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CommRenderPrivacyDevices = {};

// 公共通话设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CommRenderPublicDevices = {};

// 隐私通话录制设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CommCapturePrivacyDevices = {};

// 公共通话录制设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CommCapturePublicDevices = {};

// 私有媒体设备
std::vector<unique_ptr<AudioDeviceDescriptor>> MediaPrivacyDevices = {};

// 公共媒体设备
std::vector<unique_ptr<AudioDeviceDescriptor>> MediaPublicDevices = {};

// 私有录制设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CapturePrivacyDevices = {};

// 公共录制设备
std::vector<unique_ptr<AudioDeviceDescriptor>> CapturePublicDevices = {};


// 设备连接时进行添加并分类
void AudioDeviceManager :: AddNewDevice (const AudioDeviceDescriptor &devDesc) 
{
    AUDIO_ERR_LOG("WZX ADD NEW DEVICES.");
    sptr<AudioDeviceDescriptor> newDevice = new(std::nothrow) AudioDeviceDescriptor(devDesc);
    if (newDevice == nullptr) {
        AUDIO_ERR_LOG("Device error: null device.");
        return;
    }

// 远程播放/远程录制
    if (newDevice->networkId_ != LOCAL_NETWORK_ID && newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        RemoteRenderDevices.push_back(newDevice);
    }
    if (newDevice->networkId_ != LOCAL_NETWORK_ID && newDevice->deviceRole_ = DeviceRole::INPUT_DEVICE) {
        RemoteCaptureDevices.push_back(newDevice);
    }

// 私有媒体/公共媒体
   auto mediaPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (mediaPrivacyDevices != devicePrivacyMaps_.end()) {
        auto mediaPrivacyDevList = mediaPrivacyDevices->second;
        for (auto mediaPrivacyDev : mediaPrivacyDevList) {
            int32_t usage = GetDeviceUsageFromType(mediaPrivacyDev->deviceType_);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (mediaPrivacyDev->deviceType_ == newDevice->deviceType_)) {
                MediaPrivacyDevices.push_back(newDevice);
            }
        }
    }
    auto mediaPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (mediaPublicDevices != devicePrivacyMaps_.end()) {
        auto mediaPublicDevList = mediaPublicDevices->second;
        for (auto mediaPublicDev : mediaPublicDevList) {
            int32_t usage = GetDeviceUsageFromType(mediaPublicDev->deviceType_);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (mediaPublicDev->deviceType_ == newDevice->deviceType_)) {
                MediaPublicDevices.push_back(newDevice);
            }
        }
    }

// 隐私通话播放/公共通话播放
   auto comPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (comPrivacyDevices != devicePrivacyMaps_.end()) {
        auto comPrivacyDevList = comPrivacyDevices->second;
        for (auto comPrivacyDev : comPrivacyDevList) {
            int32_t usage = GetDeviceUsageFromType(comPrivacyDev->deviceType_);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (comPrivacyDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE)) {
                CommRenderPrivacyDevices.push_back(newDevice);
            }
        }
    }
    auto comPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (comPublicDevices != devicePrivacyMaps_.end()) {
        auto comPublicDevList = comPublicDevices->second;
        for (auto comPublicDev : comPublicDevList) {
            int32_t usage = GetDeviceUsageFromType(comPublicDev->deviceType_);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (comPublicDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE)) {
                CommRenderPublicDevices.push_back(newDevice);
            }
        }
    }

// 私有录制/公共录制
    auto capPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (capPrivacyDevices != devicePrivacyMaps_.end()) {
        auto capPrivacyDevList = capPrivacyDevices->second;
        for (auto capPrivacyDev : capPrivacyDevList) {
            if ((capPrivacyDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)) {
                CapturePrivacyDevices.push_back(newDevice);
            }
        }
    }
    auto capPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (capPublicDevices != devicePrivacyMaps_.end()) {
        auto capPublicDevList = capPublicDevices->second;
        for (auto capPublicDev : capPublicDevList) {
            if ((capPublicDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)) {
                CapturePublicDevices.push_back(newDevice);
            }
        }
    }

// 隐私通话录制/公共通话录制
    auto comCapPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (comCapPrivacyDevices != devicePrivacyMaps_.end()) {
        auto comCapPrivacyDevList = comCapPrivacyDevices->second;
        for (auto comCapPrivacyDev : comCapPrivacyDevList) {
            int32_t usage = GetDeviceUsageFromType(comCapPrivacyDev->deviceType_);
            if ((comCapPrivacyDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)
                && (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA)) {
                CommCapturePrivacyDevices.push_back(newDevice);
            }
        }
    }
    auto comCapPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (comCapPublicDevices != devicePrivacyMaps_.end()) {
        auto comCapPublicDevList = comCapPublicDevices->second;
        for (auto comCapPublicDev : comCapPublicDevList) {
            if ((comCapPublicDev->deviceType_ == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)
                && (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA)) {
                CommCapturePublicDevices.push_back(newDevice);
            }
        }
    }
// auto negativeDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_NEGATIVE);
    // if (negativeDevices != devicePrivacyMaps_.end()) {
    //     auto negativeDevList = privacyDevices->second;
    //     for (auto negativeDev : privacyDevList) {
    //         if (negativeDev.deviceType == devType) {
    //             negativeDevices_.push_back(newDevice);
    //         }
    //     }
    // }
}

// 设备断开时删除对应设备
// void AudioDeviceManager :: RemoveNewDevice (const AudioDeviceDescriptor &devDesc) 
// {
// }

// 解析完成
void AudioDeviceManager::OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData)
{
    AUDIO_INFO_LOG("WZX XmlParsingComplete DevicePrivacyInfo num [%{public}zu]", __func__, xmlData.size());
    if (xmlData.empty()) {
        AUDIO_ERR_LOG("WZX failed to parse xml file. Received data is empty");
        return;
    }

    devicePrivacyMaps_ = xmlData;
}

// 获取usage
int32_t AudioDeviceManager::GetDeviceUsageFromType(const DeviceType devType) const
{
    int32_t devUsage = -1;
    for (const auto &dev : devicePrivacyMaps_) {
        auto devInfolist = dev.second;
        for (auto devInfo : devInfolist) {
            if (devInfo.deviceType == devType) {
                devUsage = devInfo.deviceUsage;
            }
        }
    }
    AUDIO_ERR_LOG("WZX GetDeviceUsageFromType :[%{public}d]", devUsage);
    return devUsage;
}


std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteRenderDevices()
{
    return RemoteRenderDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetRemoteCaptureDevices()
{
    return RemoteCaptureDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPrivacyDevices()
{
    return CommRenderPrivacyDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommRenderPublicDevices()
{
    return CommRenderPublicDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePrivacyDevices()
{
    return CommCapturePrivacyDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetCommCapturePublicDevices()
{
    return GetCommCapturePublicDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::getMediaRenderDevices()
{
    return getMediaRenderDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::getMediaCaptureDevices()
{
    return getMediaCaptureDevices; 
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::getCapturePrivacyDevices()
{
    return getCapturePrivacyDevices;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::getCapturePublicDevices()
{
    return getCapturePublicDevices;
}

// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailRemoteRenderDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailRemoteCaptureDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCommRenderPrivacyDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCommRenderPublicDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCommCapturePrivacyDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCommCapturePublicDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailMediaRenderDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailMediaCaptureDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCapturePrivacyDevices()
// std::vector<unique_ptr<AudioDeviceDescriptor>> getAvailCapturePublicDevices()
}
}
