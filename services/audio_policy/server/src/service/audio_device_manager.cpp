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
// 蓝牙
std::vector<unique_ptr<AudioDeviceDescriptor>> bluetoothDevices_ = {};
// 远程设备
std::vector<unique_ptr<AudioDeviceDescriptor>> remoteDevices_ = {};
// 通话设备
std::vector<unique_ptr<AudioDeviceDescriptor>> communicationDevices_ = {};
// 媒体设备
std::vector<unique_ptr<AudioDeviceDescriptor>> mediaDevices_ = {};
// 录音设备
std::vector<unique_ptr<AudioDeviceDescriptor>> captureDevices_ = {};
// 隐私设备
std::vector<unique_ptr<AudioDeviceDescriptor>> privacyDevices_ = {};
// 公共属性设备
std::vector<unique_ptr<AudioDeviceDescriptor>> publicDevices_ = {};
// 默认设备
std::vector<unique_ptr<AudioDeviceDescriptor>> defaultDevices_ = {};

// 设备连接时进行添加并分类
void AudioDeviceManager :: AddNewDevice (const AudioDeviceDescriptor &devDesc) 
{
    sptr<AudioDeviceDescriptor> newDevice = new(std::nothrow) AudioDeviceDescriptor(devDesc);
    if (newDevice == nullptr) {
        AUDIO_ERR_LOG("Device error: null device.");
        return;
    }
// 通过descriptor属性分类
    if (newDevice->deviceType_ == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO
        || newDevice.deviceType_ == DeviceType :: DEVICE_TYPE_BLUETOOTH_A2DP) {
        bluetoothDevices_.push_back(newDevice);
    }
    if (newDevice->networkId_ != LOCAL_NETWORK_ID) {
        remoteDevices_.push_back(newDevice);
    }
    if (newDevice->deviceRole_ = DeviceRole::INPUT_DEVICE) {
        captureDevices_.push_back(newDevice);
    }
// 通过usage进行分类
    int32_t usage = GetDeviceUsageFromType(newDevice->deviceType_);
    if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
        communicationDevices_.push_back(newDevice);
    }
    if (usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA) {
        mediaDevices_.push_back(newDevice);
    }
// 通过获取privacy进行分类
   auto privacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (privacyDevices != devicePrivacyMaps_.end()) {
        auto privacyDevList = privacyDevices->second;
        for (auto privacyDev : privacyDevList) {
            if (privacyDev.deviceType == devType) {
                privacyDevices_.push_back(newDevice);
            }
        }
    }
    auto publicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (publicDevices != devicePrivacyMaps_.end()) {
        auto publicDevList = privacyDevices->second;
        for (auto publicDev : privacyDevList) {
            if (publicDev.deviceType == devType) {
                publicDevices_.push_back(newDevice);
            }
        }
    }
    auto negativeDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_NEGATIVE);
    if (negativeDevices != devicePrivacyMaps_.end()) {
        auto negativeDevList = privacyDevices->second;
        for (auto negativeDev : privacyDevList) {
            if (negativeDev.deviceType == devType) {
                negativeDevices_.push_back(newDevice);
            }
        }
    }
}

// 设备断开时进行对应删除
void AudioDeviceManager :: RemoveNewDevice (const AudioDeviceDescriptor &devDesc) 
{

}

// 解析完成
void AudioDeviceManager::OnXmlParsingCompleted(const std::unordered_map<AudioDevicePrivacyType, std::list<DevicePrivacyInfo>> &xmlData)
{
    AUDIO_INFO_LOG("%{public}s, DevicePrivacyInfo num [%{public}zu]", __func__, xmlData.size());
    if (xmlData.empty()) {
        AUDIO_ERR_LOG("failed to parse xml file. Received data is empty");
        return;
    }

    devicePrivacyMaps_ = xmlData;
}

// 获取usage
int32_t AudioDeviceManager::GetDeviceUsageFromType(const DeviceType devType) const
{
    int32_t devUsage = -1;
    for (const auto &dev : devicePrivacyMaps_) {
        auto devPrivacyInfolist = dev.second;
        for (auto devPrivacyInfo : devPrivacyInfolist) {
            if (devPrivacyInfo.deviceType == devType) {
                devUsage = devPrivacyInfo.deviceUsage;
                AUDIO_ERR_LOG("WZX GetDeviceUsageFromType :[%{public}d]", devUsage);
            }
        }
    }
    return devUsage;
}

// 获取privacy
void AudioDeviceManager::GetDeviceForPrivacyWithType(const DeviceType devType) const
{

}

// 非通话的隐私设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetPrivacyDevices()
{
    if (privacyDevices_ == {}) {
        AUDIO_ERR_LOG("No privacy devices. devicelist is empty");
        return;
    }

    std::vector<unique_ptr<AudioDeviceDescriptor>> priDevices_ = {};
    for (auto privacyDevice : privacyDevices_) {
        sptr<AudioDeviceDescriptor> priDevice = new(std::nothrow) AudioDeviceDescriptor(privacyDevice);
        int32_t priUsage = GetDeviceUsageFromType(priDevice->deviceType_);
        if (priUsage == DeviceUsage::MEDIA || priUsage == VOICE_MEDIA) {
            priDevices_.push_back(priDevice);
        }
    }
    return priDevices_;
}

// 非通话的公共属性设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetPublicDevices()
{
    return publicDevices_;
}

// 非通话的默认设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetDefautlDevices()
{
    return defaultDevices_;
}

// 非通话的所有设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableDevices()
{
    return communicationDevices_;
}

// 通话的隐私设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetPrivacyComDevices()
{
    return communicationDevices_;
}

// 通话的公共属性设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetPublicComDevices()
{
    return communicationDevices_;
}

// 通话的默认设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetDefautlComDevices()
{
    return communicationDevices_;
}

// 通话的所有设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableComDevices()
{
    return communicationDevices_;
}

// 录音的所有设备列表
std::vector<unique_ptr<AudioDeviceDescriptor>> AudioDeviceManager::GetAvailableRecordDevices()
{
    return captureDevices_;
}

}
}
