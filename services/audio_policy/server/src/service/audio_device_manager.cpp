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
constexpr int32_t MS_PER_S = 1000;
constexpr int32_t NS_PER_MS = 1000000;

static int64_t GetCurrentTimeMS()
{
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

void AudioDeviceManager::AddRemoteRenderDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddRemoteRenderDevice memory allocation failed");
        return;
    }
    if (devDesc.networkId_ != LOCAL_NETWORK_ID && devDesc.deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        AUDIO_ERR_LOG("WZX Add Remote RenderDevice. networkId :[%{public}s], devicerole :[%{public}d]", newDevice->networkId_.c_str(), newDevice->deviceRole_);
        remoteRenderDevices_.push_back(move(newDevice));
    }
}

void AudioDeviceManager::AddRemoteCaptureDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddRemoteCaptureDevice memory allocation failed");
        return;
    }
    if (devDesc.networkId_ != LOCAL_NETWORK_ID && devDesc.deviceRole_ == DeviceRole::INPUT_DEVICE) {
        AUDIO_ERR_LOG("WZX Add Remote CaptureDevice. networkId :[%{public}s], devicerole :[%{public}d]", newDevice->networkId_.c_str(), newDevice->deviceRole_);
        remoteCaptureDevices_.push_back(move(newDevice));
    }
}

void AudioDeviceManager::AddComRenderPrivacyDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto comRenderPrivacyDev : privacyDeviceList) {
        if ((comRenderPrivacyDev.deviceType == devDesc.deviceType_)
            && ((comRenderPrivacyDev.deviceRole & 0x2) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = comRenderPrivacyDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comm privacy DEVICES.========================");
                commRenderPrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddComRenderPublicDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto comRenderPublicDev : publicDeviceList) {
        if ((comRenderPublicDev.deviceType == devDesc.deviceType_)
            && ((comRenderPublicDev.deviceRole & 0x2) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = comRenderPublicDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comm public DEVICES.========================");
                commRenderPublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddComCapturePrivacyDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("Add ComCapturePrivacyDevices memory allocation failed");
        return;
    }
    for (auto comCapPrivacyDev : privacyDeviceList) {
        if ((comCapPrivacyDev.deviceType == devDesc.deviceType_)
            && ((comCapPrivacyDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = comCapPrivacyDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comCapPrivacyDevices DEVICES.========================");
                commCapturePrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddComCapturePublicDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto comCapPublicDev : publicDeviceList) {
        if ((comCapPublicDev.deviceType == devDesc.deviceType_)
            && ((comCapPublicDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = comCapPublicDev.deviceUsage;
            if (usage == DeviceUsage::VOICE || usage == VOICE_MEDIA) {
                AUDIO_ERR_LOG("===================== WZX ADD comCapPublicDevices DEVICES.========================");
                commCapturePublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddCapPrivacyDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto capPrivacyDev : privacyDeviceList) {
        if ((capPrivacyDev.deviceType == devDesc.deviceType_) 
            && ((capPrivacyDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            AUDIO_ERR_LOG("===================== WZX ADD capPrivacyDev DEVICES.========================");
            capturePrivacyDevices_.push_back(move(newDevice));
        }
    }
}

void AudioDeviceManager::AddCapPublicDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto capPublicDev : publicDeviceList) {
        if ((capPublicDev.deviceType == devDesc.deviceType_) 
            && ((capPublicDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            AUDIO_ERR_LOG("===================== WZX ADD capPublicDev DEVICES.========================");              
            capturePublicDevices_.push_back(move(newDevice));
        }
    }
}

void AudioDeviceManager::AddMediaRenderPrivcayDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }

    for (auto mediaRenderPrivacyDev : privacyDeviceList) {
        if ((mediaRenderPrivacyDev.deviceType == devDesc.deviceType_)
            && ((mediaRenderPrivacyDev.deviceRole & 0x1) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = mediaRenderPrivacyDev.deviceUsage;
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA)) {
                // int32_t category = devDesc.deviceCategory;
                // if (category == DeviceCategory::SOUNDBOX || category == DeviceCategory::CAR) {
                //     mediaRenderPublicDevices_.push_back(move(newDevice));
                // }
                AUDIO_ERR_LOG("===================== WZX ADD mediaRenderPublicDev DEVICES.========================");
                mediaRenderPrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddMediaCapturePrivacyDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto mediaCapturePrivacyDev : privacyDeviceList) {
        if ((mediaCapturePrivacyDev.deviceType == devDesc.deviceType_)
            && ((mediaCapturePrivacyDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = mediaCapturePrivacyDev.deviceUsage;
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA)) {
                // int32_t category = devDesc.deviceCategory;
                // if (category == DeviceCategory::SOUNDBOX || category == DeviceCategory::CAR) {
                //     mediaCapturePublicDevices_.push_back(move(newDevice));
                // }
                AUDIO_ERR_LOG("===================== WZX ADD mediaRenderPublicDev DEVICES.========================");
                mediaCapturePrivacyDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddMediaRenderPublicDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto mediaRenderPublicDev : publicDeviceList) {
        if ((mediaRenderPublicDev.deviceType == devDesc.deviceType_)
            && ((mediaRenderPublicDev.deviceRole & 0x1) == DeviceRole::OUTPUT_DEVICE)) {
            int32_t usage = mediaRenderPublicDev.deviceUsage;
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA)) {
                // int32_t category = devDesc.deviceCategory;
                // if (category == DeviceCategory::HEADPHONE || category == DeviceCategory::GLASSES) {
                //     mediaRenderPrivacyDevices_.push_back(move(newDevice));
                // }
                AUDIO_ERR_LOG("===================== WZX ADD mediaRenderPublicDev DEVICES.========================");
                mediaRenderPublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

void AudioDeviceManager::AddMediaCapturePublicDev(const AudioDeviceDescriptor &devDesc)
{
    unique_ptr<AudioDeviceDescriptor> newDevice = make_unique<AudioDeviceDescriptor>(devDesc);
    if (!newDevice) {
        AUDIO_ERR_LOG("AddNewDevice memory allocation failed");
        return;
    }
    for (auto mediaCapturePublicDev : publicDeviceList) {
        if ((mediaCapturePublicDev.deviceType == devDesc.deviceType_)
            && ((mediaCapturePublicDev.deviceRole & 0x1) == DeviceRole::INPUT_DEVICE)) {
            int32_t usage = mediaCapturePublicDev.deviceUsage;
            if ((usage == DeviceUsage::MEDIA || usage == VOICE_MEDIA)) {
                // int32_t category = devDesc.deviceCategory;
                // if (category == DeviceCategory::HEADPHONE || category == DeviceCategory::GLASSES) {
                //     mediaCapturePrivacyDevices_.push_back(move(newDevice));
                // }
                AUDIO_ERR_LOG("===================== WZX ADD mediaRenderPublicDev DEVICES.========================");
                mediaCapturePublicDevices_.push_back(move(newDevice));
                break;
            }
        }
    }
}

// 设备连接时进行添加并分类
void AudioDeviceManager::AddNewDevice(AudioDeviceDescriptor &devDesc)
{
    devDesc.connectState_ = CONNECTED;
    devDesc.connectTimeStamp_ = GetCurrentTimeMS();

    AddRemoteRenderDev(devDesc);
    AddRemoteCaptureDev(devDesc);

    AddMediaRenderPrivcayDev(devDesc);
    AddMediaCapturePrivacyDev(devDesc);
    AddComRenderPrivacyDev(devDesc);
    AddComCapturePrivacyDev(devDesc);
    AddCapPrivacyDev(devDesc);

    AddMediaRenderPublicDev(devDesc);
    AddMediaCapturePublicDev(devDesc);
    AddComRenderPublicDev(devDesc);
    AddComCapturePublicDev(devDesc);
    AddCapPublicDev(devDesc);

    AUDIO_ERR_LOG("WZX ADD DEVICE END.");
}

void AudioDeviceManager::RemoveNewDevice(const AudioDeviceDescriptor &devDesc)
{
    auto isPresent = [&devDesc] (const std::unique_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return devDesc.deviceType_ == desc->deviceType_ && devDesc.macAddress_ == desc->macAddress_;
    };

    auto itr = std::find_if(remoteRenderDevices_.begin(), remoteRenderDevices_.end(), isPresent);
    if (itr != remoteRenderDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove remoteRenderDevices_ ========================");
        remoteRenderDevices_.erase(itr);
    }

    auto itr1 = std::find_if(remoteCaptureDevices_.begin(), remoteCaptureDevices_.end(), isPresent);
    if (itr1 != remoteCaptureDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove remoteCaptureDevices_ ========================");
        remoteCaptureDevices_.erase(itr1);
    }

    auto itr2 = std::find_if(commRenderPrivacyDevices_.begin(), commRenderPrivacyDevices_.end(), isPresent);
    if (itr2 != commRenderPrivacyDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove commRenderPrivacyDevices_ ========================");
        commRenderPrivacyDevices_.erase(itr2);
    }

    auto itr3 = std::find_if(commRenderPublicDevices_.begin(), commRenderPublicDevices_.end(), isPresent);
    if (itr3 != commRenderPublicDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove commRenderPublicDevices_ ========================");
        commRenderPublicDevices_.erase(itr3);
    }

    auto itr4 = std::find_if(commCapturePrivacyDevices_.begin(), commCapturePrivacyDevices_.end(), isPresent);
    if (itr4 != commCapturePrivacyDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove commCapturePrivacyDevices_ ========================");
        commCapturePrivacyDevices_.erase(itr4);
    }

    auto itr5 = std::find_if(commCapturePublicDevices_.begin(), commCapturePublicDevices_.end(), isPresent);
    if (itr5 != commCapturePublicDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove commCapturePublicDevices_ ========================");
        commCapturePublicDevices_.erase(itr5);
    }

    auto itr6 = std::find_if(mediaRenderPrivacyDevices_.begin(), mediaRenderPrivacyDevices_.end(), isPresent);
    if (itr6 != mediaRenderPrivacyDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove mediaPrivacyDevices_ ========================");
        mediaRenderPrivacyDevices_.erase(itr6);
    }

    auto itr7 = std::find_if(mediaRenderPublicDevices_.begin(), mediaRenderPublicDevices_.end(), isPresent);
    if (itr7 != mediaRenderPublicDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove mediaPublicDevices_ ========================");
        mediaRenderPublicDevices_.erase(itr7);
    }

    auto itr8 = std::find_if(mediaCapturePrivacyDevices_.begin(), mediaCapturePrivacyDevices_.end(), isPresent);
    if (itr8 != mediaCapturePrivacyDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove mediaPrivacyDevices_ ========================");
        mediaCapturePrivacyDevices_.erase(itr8);
    }

    auto itr9 = std::find_if(mediaCapturePublicDevices_.begin(), mediaCapturePublicDevices_.end(), isPresent);
    if (itr9 != mediaCapturePublicDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove mediaPublicDevices_ ========================");
        mediaCapturePublicDevices_.erase(itr9);
    }

    auto itr10 = std::find_if(capturePrivacyDevices_.begin(), capturePrivacyDevices_.end(), isPresent);
    if (itr10 != capturePrivacyDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove capturePrivacyDevices_ ========================");
        capturePrivacyDevices_.erase(itr10);
    }

    auto itr11 = std::find_if(capturePublicDevices_.begin(), capturePublicDevices_.end(), isPresent);
    if (itr11 != capturePublicDevices_.end()) {
        AUDIO_ERR_LOG("=====================remove capturePublicDevices_ ========================");
        capturePublicDevices_.erase(itr11);
    }
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

    auto privacyDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PRIVACY);
    if (privacyDevices != devicePrivacyMaps_.end()) {
       privacyDeviceList = privacyDevices->second;
    }
    auto publicDevices = devicePrivacyMaps_.find(AudioDevicePrivacyType::TYPE_PUBLIC);
    if (publicDevices != devicePrivacyMaps_.end()) {
        publicDeviceList = publicDevices->second;
    }

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

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetMediaRenderPrivacyDevices() 
{
    return mediaRenderPrivacyDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetMediaRenderPublicDevices()
{

    return mediaRenderPublicDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetMediaCapturePrivacyDevices() 
{
    return mediaCapturePrivacyDevices_;
}

std::vector<unique_ptr<AudioDeviceDescriptor>> &AudioDeviceManager::GetMediaCapturePublicDevices()
{

    return mediaCapturePublicDevices_;
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
