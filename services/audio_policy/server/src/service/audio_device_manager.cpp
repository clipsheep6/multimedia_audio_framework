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
void AudioDeviceManager::AddNewDevice(const AudioDeviceDescriptor &devDesc)
{
    std::list<DevicePrivacyInfo> privacyDeviceList;
    std::list<DevicePrivacyInfo> publicDeviceList;

    auto privacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (privacyDevices != devicePrivacyMaps_.end()) {
       privacyDeviceList = privacyDevices->second;
    }  

    auto publicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (publicDevices != devicePrivacyMaps_.end()) {
        publicDeviceList = publicDevices->second;
    }

// 远程播放/远程录制
    if (devDesc.networkId_ != LOCAL_NETWORK_ID && devDesc.deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
        if (!newDevice) {
            AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
            return;
        }
        AUDIO_ERR_LOG("WZX ADD remote DEVICES_1. networkId :[%{public}s], devicerole :[%{public}d]", newDevice->networkId_.c_str(), newDevice->deviceRole_);
        remoteRenderDevices_.push_back(move(newDevice));
    }

    if (devDesc.networkId_ != LOCAL_NETWORK_ID && devDesc.deviceRole_ == DeviceRole::INPUT_DEVICE) {
        unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
        if (!newDevice) {
            AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
            return;
        }
        AUDIO_ERR_LOG("WZX ADD remote DEVICES_2. networkId :[%{public}s], devicerole :[%{public}d]", newDevice->networkId_.c_str(), newDevice->deviceRole_);
        remoteCaptureDevices_.push_back(move(newDevice));
    }

// 隐私媒体/公共媒体
//    AUDIO_ERR_LOG("****************** Find media privacy DEVICES ******************");

    for (auto mediaPrivacyDev : privacyDeviceList) {
        if (mediaPrivacyDev.deviceType == devDesc.deviceType_) {
            int32_t usage = mediaPrivacyDev.deviceUsage;
            if(usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) {
                unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
                if (!newDevice) {
                    AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                    return;
                }
                AUDIO_ERR_LOG("===================== WZX ADD media privacy DEVICES.========================");
                mediaPrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }


//    AUDIO_ERR_LOG("****************** Find media public DEVICES ******************");

    for (auto mediaPublicDev : publicDeviceList) {
        if (mediaPublicDev.deviceType == devDesc.deviceType_) {
            unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
            if (!newDevice) {
                AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                return;
            }
            int32_t usage = mediaPublicDev.deviceUsage;
            if(usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD media public DEVICES.========================");
                mediaPublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }

//    AUDIO_ERR_LOG("****************** Find voice render privacy DEVICES ******************");
// 隐私通话播放/公共通话播放

    for (auto comRenderPrivacyDev : privacyDeviceList) {
        if ((comRenderPrivacyDev.deviceType == devDesc.deviceType_)
            && ((comRenderPrivacyDev.deviceRole & 0x2) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = comRenderPrivacyDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
                if (!newDevice) {
                    AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                    return;
                }
                AUDIO_ERR_LOG("===================== WZX ADD comm privacy DEVICES.========================");      
                commRenderPrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }

//    AUDIO_ERR_LOG("****************** Find voice render public DEVICES ******************");
    for (auto comRenderPublicDev : publicDeviceList) {
        if ((comRenderPublicDev.deviceType == devDesc.deviceType_)
            && ((comRenderPublicDev.deviceRole & 0x2) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = comRenderPublicDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comm public DEVICES.========================");
                unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
                if (!newDevice) {
                    AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                    return;
                }
                commRenderPublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }

// 隐私通话录制/公共通话录制
//    AUDIO_ERR_LOG("****************** Find voice capturer privacy DEVICES ******************");
    for (auto comCapPrivacyDev : privacyDeviceList) {
        if ((comCapPrivacyDev.deviceType == devDesc.deviceType_)
            && ((comCapPrivacyDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = comCapPrivacyDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comCapPrivacyDevices DEVICES.========================");
                unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
                if (!newDevice) {
                    AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                    return;
                }
                commCapturePrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }

//    AUDIO_ERR_LOG("****************** Find voice capturer public DEVICES ******************");

    for (auto comCapPublicDev : publicDeviceList) {
        if ((comCapPublicDev.deviceType == devDesc.deviceType_)
            && ((comCapPublicDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = comCapPublicDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comCapPublicDevices DEVICES.========================");
                unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
                if (!newDevice) {
                    AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                    return;
                }
                commCapturePublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }

// 私有录制/公共录制
//    AUDIO_ERR_LOG("****************** Find capturer privacy DEVICES ******************");
    for (auto capPrivacyDev : privacyDeviceList) {
        if ((capPrivacyDev.deviceType == devDesc.deviceType_) 
            && ((capPrivacyDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            AUDIO_ERR_LOG("===================== WZX ADD capPrivacyDev DEVICES.========================");
            unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
            if (!newDevice) {
                AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                return;
            }
            capturePrivacyDevices_.push_back(move(newDevice));
        }
    }

//    AUDIO_ERR_LOG("****************** Find capturer public DEVICES ******************");

    for (auto capPublicDev : publicDeviceList) {
        if ((capPublicDev.deviceType == devDesc.deviceType_) 
            && ((capPublicDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            AUDIO_ERR_LOG("===================== WZX ADD capPublicDev DEVICES.========================");
            unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
            if (!newDevice) {
                AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
                return;
            }                
            capturePublicDevices_.push_back(move(newDevice));
        }
    }
    AUDIO_ERR_LOG("WZX ADD DEVICE END.");
}

void AudioDeviceManager::ParseDeviceXml()
{
    std::unique_ptr<AudioDeviceParser> audioDeviceParser = make_unique<AudioDeviceParser>(this);
    if (audioDeviceParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("WZX AudioAdapterManager: Audio device Config Load Configuration successfully");
        audioDeviceParser->Parse();
    }
}

// 解析完成
 void AudioDeviceManager::OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData)
 {
    AUDIO_INFO_LOG("WZX XmlParsingComplete DevicePrivacyInfo num %{public}s [%{public}zu]", __func__, xmlData.size());
    if (xmlData.empty()) {
        AUDIO_ERR_LOG("WZX failed to parse xml file. Received data is empty");
        return;
    }

    devicePrivacyMaps_ = xmlData;
    AUDIO_INFO_LOG("WZX XmlParsingComplete devicePrivacyMaps_ num %{public}s [%{public}zu]", __func__, devicePrivacyMaps_.size());
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
    return mediaRenderDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetMediaCaptureDevices() const
{
    std::vector<unique_ptr<AudioDeviceDescriptor>> mediaCaptureDevices_;
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
}
}
