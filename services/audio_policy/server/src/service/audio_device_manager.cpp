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

// 设备连接时进行添加并分类
void AudioDeviceManager::AddNewDevice (const AudioDeviceDescriptor &devDesc) 
{
    AUDIO_ERR_LOG("WZX ADD NEW DEVICES.");
    int32_t usage = -1;

    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }

// 远程播放/远程录制
    if (newDevice->networkId_ != LOCAL_NETWORK_ID && newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        remoteRenderDevices_.push_back(move(newDevice));
    }
    if (newDevice->networkId_ != LOCAL_NETWORK_ID && newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE) {
        remoteCaptureDevices_.push_back(move(newDevice));
    }

// 私有媒体/公共媒体
   auto mediaPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (mediaPrivacyDevices != devicePrivacyMaps_.end()) {
        auto mediaPrivacyDevList = mediaPrivacyDevices->second;
        for (auto mediaPrivacyDev : mediaPrivacyDevList) {
            usage = GetDeviceUsageFromType(mediaPrivacyDev.deviceType);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (mediaPrivacyDev.deviceType == newDevice->deviceType_)) {
                mediaPrivacyDevices_.push_back(move(newDevice));
            }
        }
    }
    auto mediaPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (mediaPublicDevices != devicePrivacyMaps_.end()) {
        auto mediaPublicDevList = mediaPublicDevices->second;
        for (auto mediaPublicDev : mediaPublicDevList) {
            usage = GetDeviceUsageFromType(mediaPublicDev.deviceType);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA)
                && (mediaPublicDev.deviceType == newDevice->deviceType_)) {
                mediaPublicDevices_.push_back(move(newDevice));
            }
        }
    }

// 隐私通话播放/公共通话播放
   auto comPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (comPrivacyDevices != devicePrivacyMaps_.end()) {
        auto comPrivacyDevList = comPrivacyDevices->second;
        for (auto comPrivacyDev : comPrivacyDevList) {
            usage = GetDeviceUsageFromType(comPrivacyDev.deviceType);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (comPrivacyDev.deviceType == newDevice->deviceType_)
                && (newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE)) {
                commRenderPrivacyDevices_.push_back(move(newDevice));
            }
        }
    }
    auto comPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (comPublicDevices != devicePrivacyMaps_.end()) {
        auto comPublicDevList = comPublicDevices->second;
        for (auto comPublicDev : comPublicDevList) {
            usage = GetDeviceUsageFromType(comPublicDev.deviceType);
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) 
                && (comPublicDev.deviceType == newDevice->deviceType_)
                && (newDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE)) {
                commRenderPublicDevices_.push_back(move(newDevice));
            }
        }
    }

// 私有录制/公共录制
    auto capPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (capPrivacyDevices != devicePrivacyMaps_.end()) {
        auto capPrivacyDevList = capPrivacyDevices->second;
        for (auto capPrivacyDev : capPrivacyDevList) {
            if ((capPrivacyDev.deviceType == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)) {
                capturePrivacyDevices_.push_back(move(newDevice));
            }
        }
    }
    auto capPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (capPublicDevices != devicePrivacyMaps_.end()) {
        auto capPublicDevList = capPublicDevices->second;
        for (auto capPublicDev : capPublicDevList) {
            if ((capPublicDev.deviceType == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)) {
                capturePublicDevices_.push_back(move(newDevice));
            }
        }
    }

// 隐私通话录制/公共通话录制
    auto comCapPrivacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (comCapPrivacyDevices != devicePrivacyMaps_.end()) {
        auto comCapPrivacyDevList = comCapPrivacyDevices->second;
        for (auto comCapPrivacyDev : comCapPrivacyDevList) {
            usage = GetDeviceUsageFromType(comCapPrivacyDev.deviceType);
            if ((comCapPrivacyDev.deviceType == newDevice->deviceType_) 
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)
                && (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA)) {
                commCapturePrivacyDevices_.push_back(move(newDevice));
            }
        }
    }
    auto comCapPublicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (comCapPublicDevices != devicePrivacyMaps_.end()) {
        auto comCapPublicDevList = comCapPublicDevices->second;
        for (auto comCapPublicDev : comCapPublicDevList) {
            usage = GetDeviceUsageFromType(comCapPublicDev.deviceType);
            if ((comCapPublicDev.deviceType == newDevice->deviceType_)
                && (newDevice->deviceRole_ == DeviceRole::INPUT_DEVICE)
                && (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA)) {
                commCapturePublicDevices_.push_back(move(newDevice));
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
    AUDIO_INFO_LOG("WZX XmlParsingComplete DevicePrivacyInfo num %{public}s [%{public}zu]", __func__, xmlData.size());
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


std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetRemoteRenderDevices() 
{
    return remoteRenderDevices_;
}


std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetRemoteCaptureDevices() 
{
    return remoteCaptureDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCommRenderPrivacyDevices() 
{
    return commRenderPrivacyDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCommRenderPublicDevices() 
{
    return commRenderPublicDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCommCapturePrivacyDevices() 
{
    return commCapturePrivacyDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCommCapturePublicDevices() 
{
    return commCapturePublicDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaRenderDevices() const
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaRenderDevices_;
    for (auto &renPriDevice : mediaPrivacyDevices_) {
        unique_ptr<AudioDeviceDescriptor> renderPriDevice = make_unique<AudioDeviceDescriptor>();
        if (renPriDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
            renderPriDevice->deviceType_ = renPriDevice->deviceType_;
            renderPriDevice->deviceRole_ = renPriDevice->deviceRole_;
            renderPriDevice->deviceId_ = renPriDevice->deviceId_;
            renderPriDevice->channelMasks_ = renPriDevice->channelMasks_;
            renderPriDevice->channelIndexMasks_ = renPriDevice->channelIndexMasks_;
            renderPriDevice->deviceName_ = renPriDevice->deviceName_;
            renderPriDevice->macAddress_ = renPriDevice->macAddress_;
            renderPriDevice->interruptGroupId_ = renPriDevice->interruptGroupId_;
            renderPriDevice->volumeGroupId_ = renPriDevice->volumeGroupId_;
            renderPriDevice->networkId_ = renPriDevice->networkId_;
            renderPriDevice->displayName_ = renPriDevice->displayName_;
            renderPriDevice->audioStreamInfo_ = renPriDevice->audioStreamInfo_;
            //继续赋值
            mediaRenderDevices_.push_back(move(renderPriDevice));
        }
    }
    for (auto &renPubDevice : mediaPublicDevices_) {
        unique_ptr<AudioDeviceDescriptor> renderPubDevice = make_unique<AudioDeviceDescriptor>();
        if (renPubDevice->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
            renderPubDevice->deviceType_ = renPubDevice->deviceType_;
            renderPubDevice->deviceRole_ = renPubDevice->deviceRole_;
            renderPubDevice->deviceId_ = renPubDevice->deviceId_;
            renderPubDevice->channelMasks_ = renPubDevice->channelMasks_;
            renderPubDevice->channelIndexMasks_ = renPubDevice->channelIndexMasks_;
            renderPubDevice->deviceName_ = renPubDevice->deviceName_;
            renderPubDevice->macAddress_ = renPubDevice->macAddress_;
            renderPubDevice->interruptGroupId_ = renPubDevice->interruptGroupId_;
            renderPubDevice->volumeGroupId_ = renPubDevice->volumeGroupId_;
            renderPubDevice->networkId_ = renPubDevice->networkId_;
            renderPubDevice->displayName_ = renPubDevice->displayName_;
            renderPubDevice->audioStreamInfo_ = renPubDevice->audioStreamInfo_;
            mediaRenderDevices_.push_back(move(renderPubDevice));
        }
    }
    return mediaRenderDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCaptureDevices() const
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaCaptureDevices_;
    for (auto &capPriDevice : mediaPrivacyDevices_) {
        unique_ptr<AudioDeviceDescriptor> capturePriDevice = make_unique<AudioDeviceDescriptor>();
        if (capPriDevice->deviceRole_ == DeviceRole::INPUT_DEVICE) {
            capturePriDevice->deviceType_ = capPriDevice->deviceType_;
            capturePriDevice->deviceRole_ = capPriDevice->deviceRole_;
            capturePriDevice->deviceId_ = capPriDevice->deviceId_;
            capturePriDevice->channelMasks_ = capPriDevice->channelMasks_;
            capturePriDevice->channelIndexMasks_ = capPriDevice->channelIndexMasks_;
            capturePriDevice->deviceName_ = capPriDevice->deviceName_;
            capturePriDevice->macAddress_ = capPriDevice->macAddress_;
            capturePriDevice->interruptGroupId_ = capPriDevice->interruptGroupId_;
            capturePriDevice->volumeGroupId_ = capPriDevice->volumeGroupId_;
            capturePriDevice->networkId_ = capPriDevice->networkId_;
            capturePriDevice->displayName_ = capPriDevice->displayName_;
            capturePriDevice->audioStreamInfo_ = capPriDevice->audioStreamInfo_;
            mediaCaptureDevices_.push_back(move(capturePriDevice));
        }
    }
    for (auto &capPubDevice : mediaPublicDevices_) {
        unique_ptr<AudioDeviceDescriptor> capturePubDevice = make_unique<AudioDeviceDescriptor>();
        if (capPubDevice->deviceRole_ == DeviceRole::INPUT_DEVICE) {
            capturePubDevice->deviceType_ = capPubDevice->deviceType_;
            capturePubDevice->deviceRole_ = capPubDevice->deviceRole_;
            capturePubDevice->deviceId_ = capPubDevice->deviceId_;
            capturePubDevice->channelMasks_ = capPubDevice->channelMasks_;
            capturePubDevice->channelIndexMasks_ = capPubDevice->channelIndexMasks_;
            capturePubDevice->deviceName_ = capPubDevice->deviceName_;
            capturePubDevice->macAddress_ = capPubDevice->macAddress_;
            capturePubDevice->interruptGroupId_ = capPubDevice->interruptGroupId_;
            capturePubDevice->volumeGroupId_ = capPubDevice->volumeGroupId_;
            capturePubDevice->networkId_ = capPubDevice->networkId_;
            capturePubDevice->displayName_ = capPubDevice->displayName_;
            capturePubDevice->audioStreamInfo_ = capPubDevice->audioStreamInfo_;
            mediaCaptureDevices_.push_back(move(capturePubDevice));
        }
    }
    return mediaCaptureDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCapturePrivacyDevices() 
{
    return capturePrivacyDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetCapturePublicDevices() 
{
    return capturePublicDevices_;
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
