/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_focus_parser.h"
#include "audio_manager_base.h"
#include "iservice_registry.h"
#include "audio_log.h"
#include "hisysevent.h"
#include "system_ability_definition.h"

#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const uint32_t PCM_8_BIT = 8;
const uint32_t BT_BUFFER_ADJUSTMENT_FACTOR = 50;
static sptr<IStandardAudioService> g_sProxy = nullptr;

AudioPolicyService::~AudioPolicyService()
{
    AUDIO_ERR_LOG("~AudioPolicyService()");
    Deinit();
}

bool AudioPolicyService::Init(void)
{
    serviceFlag_.reset();
    mAudioPolicyManager.Init();
    if (!mConfigParser.LoadConfiguration()) {
        AUDIO_ERR_LOG("Audio Config Load Configuration failed");
        return false;
    }
    if (!mConfigParser.Parse()) {
        AUDIO_ERR_LOG("Audio Config Parse failed");
        return false;
    }
    std::unique_ptr<AudioToneParser> audioToneParser = make_unique<AudioToneParser>();
    CHECK_AND_RETURN_RET_LOG(audioToneParser != nullptr, false, "Failed to create AudioToneParser");
    std::string AUDIO_TONE_CONFIG_FILE = "/etc/audio/audio_tone_dtmf_config.xml";

    if (audioToneParser->LoadConfig(toneDescriptorMap)) {
        AUDIO_ERR_LOG("Audio Tone Load Configuration failed");
        return false;
    }

    std::unique_ptr<AudioFocusParser> audioFocusParser = make_unique<AudioFocusParser>();
    CHECK_AND_RETURN_RET_LOG(audioFocusParser != nullptr, false, "Failed to create AudioFocusParser");
    std::string AUDIO_FOCUS_CONFIG_FILE = "vendor/etc/audio/audio_interrupt_policy_config.xml";

    if (audioFocusParser->LoadConfig(focusTable_[0][0])) {
        AUDIO_ERR_LOG("Audio Interrupt Load Configuration failed");
        return false;
    }

    if (mDeviceStatusListener->RegisterDeviceStatusListener(nullptr)) {
        AUDIO_ERR_LOG("[Policy Service] Register for device status events failed");
        return false;
    }

    return true;
}

void AudioPolicyService::InitKVStore()
{
    mAudioPolicyManager.InitKVStore();
}

bool AudioPolicyService::ConnectServiceAdapter()
{
    if (!mAudioPolicyManager.ConnectServiceAdapter()) {
        AUDIO_ERR_LOG("AudioPolicyService::ConnectServiceAdapter  Error in connecting to audio service adapter");
        return false;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        AUDIO_ERR_LOG("[Policy Service] Get samgr failed");
        return false;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    if (object == nullptr) {
        AUDIO_DEBUG_LOG("[Policy Service] audio service remote object is NULL.");
        return false;
    }

    g_sProxy = iface_cast<IStandardAudioService>(object);
    if (g_sProxy == nullptr) {
        AUDIO_DEBUG_LOG("[Policy Service] init g_sProxy is NULL.");
        return false;
    }

    if (serviceFlag_.count() != MIN_SERVICE_COUNT) {
        OnServiceConnected(AudioServiceIndex::AUDIO_SERVICE_INDEX);
    }

    return true;
}

void AudioPolicyService::Deinit(void)
{
    AUDIO_ERR_LOG("Policy service died. closing active ports");
    std::for_each(mIOHandles.begin(), mIOHandles.end(), [&](std::pair<std::string, AudioIOHandle> handle) {
        mAudioPolicyManager.CloseAudioPort(handle.second);
    });

    mIOHandles.clear();
    mDeviceStatusListener->UnRegisterDeviceStatusListener();
    return;
}

int32_t AudioPolicyService::SetAudioSessionCallback(AudioSessionCallback *callback)
{
    return mAudioPolicyManager.SetAudioSessionCallback(callback);
}

int32_t AudioPolicyService::SetStreamVolume(AudioStreamType streamType, float volume) const
{
    return mAudioPolicyManager.SetStreamVolume(streamType, volume);
}

float AudioPolicyService::GetStreamVolume(AudioStreamType streamType) const
{
    return mAudioPolicyManager.GetStreamVolume(streamType);
}

int32_t AudioPolicyService::SetStreamMute(AudioStreamType streamType, bool mute) const
{
    return mAudioPolicyManager.SetStreamMute(streamType, mute);
}

bool AudioPolicyService::GetStreamMute(AudioStreamType streamType) const
{
    return mAudioPolicyManager.GetStreamMute(streamType);
}

bool AudioPolicyService::IsStreamActive(AudioStreamType streamType) const
{
    return mAudioPolicyManager.IsStreamActive(streamType);
}

std::string AudioPolicyService::GetPortName(InternalDeviceType deviceType)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            portName = PIPE_SINK;
            break;
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            portName = BLUETOOTH_SPEAKER;
            break;
        case InternalDeviceType::DEVICE_TYPE_SPEAKER:
        case InternalDeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case InternalDeviceType::DEVICE_TYPE_USB_HEADSET:
            portName = PRIMARY_SPEAKER;
            break;
        case InternalDeviceType::DEVICE_TYPE_MIC:
            portName = PRIMARY_MIC;
            break;
        case InternalDeviceType::DEVICE_TYPE_FILE_SINK:
            portName = FILE_SINK;
            break;
        case InternalDeviceType::DEVICE_TYPE_FILE_SOURCE:
            portName = FILE_SOURCE;
            break;
        default:
            portName = PORT_NONE;
            break;
    }
    return portName;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioPolicyService::GetDevices(DeviceFlag deviceFlag)
{
    AUDIO_INFO_LOG("Entered AudioPolicyService::%{public}s", __func__);
    std::vector<sptr<AudioDeviceDescriptor>> deviceList = {};

    if (deviceFlag < DeviceFlag::OUTPUT_DEVICES_FLAG || deviceFlag > DeviceFlag::ALL_DEVICES_FLAG) {
        AUDIO_ERR_LOG("Invalid flag provided %{public}d", deviceFlag);
        return deviceList;
    }

    if (deviceFlag == DeviceFlag::ALL_DEVICES_FLAG) {
        return mConnectedDevices;
    }

    DeviceRole role = DeviceRole::OUTPUT_DEVICE;
    role = (deviceFlag == DeviceFlag::OUTPUT_DEVICES_FLAG) ? DeviceRole::OUTPUT_DEVICE : DeviceRole::INPUT_DEVICE;

    AUDIO_INFO_LOG("GetDevices mConnectedDevices size = [%{public}zu]", mConnectedDevices.size());
    for (const auto &device : mConnectedDevices) {
        if (device != nullptr && device->deviceRole_ == role) {
            sptr<AudioDeviceDescriptor> devDesc = new(std::nothrow) AudioDeviceDescriptor(device->deviceType_,
                device->deviceRole_);
            deviceList.push_back(devDesc);
        }
    }

    AUDIO_INFO_LOG("GetDevices list size = [%{public}zu]", deviceList.size());
    return deviceList;
}

DeviceType AudioPolicyService::FetchHighPriorityDevice()
{
    AUDIO_DEBUG_LOG("Entered AudioPolicyService::%{public}s", __func__);
    DeviceType priorityDevice = DEVICE_TYPE_SPEAKER;

    for (const auto &device : priorityList) {
        auto isPresent = [&device] (const sptr<AudioDeviceDescriptor> &desc) {
            CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "FetchHighPriorityDevice device is nullptr");
            return desc->deviceType_ == device;
        };

        auto itr = std::find_if(mConnectedDevices.begin(), mConnectedDevices.end(), isPresent);
        if (itr != mConnectedDevices.end()) {
            priorityDevice = (*itr)->deviceType_;
            break;
        }
    }

    return priorityDevice;
}

void UpdateActiveDeviceRoute(InternalDeviceType deviceType)
{
    auto ret = SUCCESS;

    switch (deviceType) {
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_WIRED_HEADSET: {
            ret = g_sProxy->UpdateActiveDeviceRoute(deviceType, DeviceFlag::ALL_DEVICES_FLAG);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the route for %{public}d", deviceType);
            break;
        }
        case DEVICE_TYPE_SPEAKER: {
            ret = g_sProxy->UpdateActiveDeviceRoute(deviceType, DeviceFlag::OUTPUT_DEVICES_FLAG);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the route for %{public}d", deviceType);
            break;
        }
        case DEVICE_TYPE_MIC: {
            ret = g_sProxy->UpdateActiveDeviceRoute(deviceType, DeviceFlag::INPUT_DEVICES_FLAG);
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to update the route for %{public}d", deviceType);
            break;
        }
        default:
            break;
    }
}

static string ConvertToHDIAudioFormat(AudioSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case SAMPLE_U8:
            return "u8";
        case SAMPLE_S16LE:
            return "s16le";
        case SAMPLE_S24LE:
            return "s24le";
        case SAMPLE_S32LE:
            return "s32le";
        default:
            return "";
    }
}

int32_t AudioPolicyService::ActivateNewDevice(DeviceType deviceType)
{
    int32_t result = SUCCESS;

    if (mCurrentActiveDevice == deviceType) {
        return result;
    }

    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        auto primaryModulesPos = deviceClassInfo_.find(ClassType::TYPE_A2DP);
        if (primaryModulesPos != deviceClassInfo_.end()) {
            auto moduleInfoList = primaryModulesPos->second;
            for (auto &moduleInfo : moduleInfoList) {
                if (mIOHandles.find(moduleInfo.name) == mIOHandles.end()) {
                    AUDIO_INFO_LOG("Load a2dp module [%{public}s]", moduleInfo.name.c_str());
                    AudioStreamInfo audioStreamInfo = {};
                    GetActiveDeviceStreamInfo(deviceType, audioStreamInfo);
                    uint32_t bufferSize
                        = (audioStreamInfo.samplingRate * audioStreamInfo.format * audioStreamInfo.channels)
                            / (PCM_8_BIT * BT_BUFFER_ADJUSTMENT_FACTOR);
                    AUDIO_INFO_LOG("a2dp rate: %{public}d, format: %{public}d, channel: %{public}d",
                        audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
                    moduleInfo.channels = to_string(audioStreamInfo.channels);
                    moduleInfo.rate = to_string(audioStreamInfo.samplingRate);
                    moduleInfo.format = ConvertToHDIAudioFormat(audioStreamInfo.format);
                    moduleInfo.bufferSize = to_string(bufferSize);

                    AudioIOHandle ioHandle = mAudioPolicyManager.OpenAudioPort(moduleInfo);
                    CHECK_AND_RETURN_RET_LOG(ioHandle != ERR_OPERATION_FAILED && ioHandle != ERR_INVALID_HANDLE,
                                             ERR_OPERATION_FAILED, "OpenAudioPort failed %{public}d", ioHandle);
                    mIOHandles[moduleInfo.name] = ioHandle;
                }

                std::string activePort = GetPortName(mCurrentActiveDevice);
                AUDIO_INFO_LOG("port %{public}s, active device %{public}d", activePort.c_str(), mCurrentActiveDevice);
                mAudioPolicyManager.SuspendAudioDevice(activePort, true);
            }
        }
    } else if (mCurrentActiveDevice == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string activePort = GetPortName(mCurrentActiveDevice);
        AUDIO_INFO_LOG("suspend a2dp first %{public}s", activePort.c_str());
        mAudioPolicyManager.SuspendAudioDevice(activePort, true);
    }

    AudioIOHandle ioHandle = GetAudioIOHandle(deviceType);
    std::string portName = GetPortName(deviceType);

    AUDIO_INFO_LOG("port name is %{public}s", portName.c_str());
    if (portName != PORT_NONE) {
        result = mAudioPolicyManager.SetDeviceActive(ioHandle, deviceType, portName, true);
        mAudioPolicyManager.SuspendAudioDevice(portName, false);
    } else {
        result = ERR_DEVICE_NOT_SUPPORTED;
    }

    if (result) {
        AUDIO_ERR_LOG("SetDeviceActive failed %{public}d", result);
        return result;
    }

    if (!isUpdateRouteSupported_) {
        AUDIO_INFO_LOG("Update route not supported for this device");
        return SUCCESS;
    }

    UpdateActiveDeviceRoute(deviceType);

    return SUCCESS;
}

int32_t AudioPolicyService::SetDeviceActive(InternalDeviceType deviceType, bool active)
{
    AUDIO_DEBUG_LOG("[Policy Service] Device type[%{public}d] status[%{public}d]", deviceType, active);
    CHECK_AND_RETURN_RET_LOG(deviceType != DEVICE_TYPE_NONE, ERR_DEVICE_NOT_SUPPORTED, "Invalid device");

    int32_t result = SUCCESS;

    if (!active) {
        CHECK_AND_RETURN_RET_LOG(deviceType == mCurrentActiveDevice, SUCCESS, "This device is not active");
        if (mConnectedDevices.size() == mDefaultDeviceCount && deviceType == DEVICE_TYPE_SPEAKER) {
            deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
        } else {
            deviceType = FetchHighPriorityDevice();
        }
    }

    if (deviceType == mCurrentActiveDevice) {
        AUDIO_ERR_LOG("Device already activated %{public}d. No need to activate again", mCurrentActiveDevice);
        return SUCCESS;
    }

    if (deviceType == DEVICE_TYPE_SPEAKER) {
        result = ActivateNewDevice(DEVICE_TYPE_SPEAKER);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "Failed for speaker %{public}d", result);

        result = ActivateNewDevice(DEVICE_TYPE_MIC);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "Failed for MIC %{public}d", result);
    } else if (deviceType == DEVICE_TYPE_FILE_SINK) {
        result = ActivateNewDevice(DEVICE_TYPE_FILE_SINK);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "Failed for FILE_SINK %{public}d", result);

        result = ActivateNewDevice(DEVICE_TYPE_FILE_SOURCE);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "Failed for FILE_SOURCE %{public}d", result);
    } else {
        result = ActivateNewDevice(deviceType);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "Activate failed %{public}d", result);
    }

    mCurrentActiveDevice = deviceType;
    return result;
}

bool AudioPolicyService::IsDeviceActive(InternalDeviceType deviceType) const
{
    AUDIO_INFO_LOG("Entered AudioPolicyService::%{public}s", __func__);
    return mCurrentActiveDevice == deviceType;
}

int32_t AudioPolicyService::SetRingerMode(AudioRingerMode ringMode)
{
    return mAudioPolicyManager.SetRingerMode(ringMode);
}

AudioRingerMode AudioPolicyService::GetRingerMode() const
{
    return mAudioPolicyManager.GetRingerMode();
}

int32_t AudioPolicyService::SetAudioScene(AudioScene audioScene)
{
    if (g_sProxy == nullptr) {
        AUDIO_DEBUG_LOG("AudioPolicyService::SetAudioScene g_sProxy is nullptr");
        return ERR_OPERATION_FAILED;
    }

    int32_t result = g_sProxy->SetAudioScene(audioScene);
    AUDIO_INFO_LOG("SetAudioScene return value from audio HAL: %{public}d", result);
    // As Audio HAL is stubbed now, we set and return
    mAudioScene = audioScene;
    AUDIO_INFO_LOG("AudioScene is set as: %{public}d", audioScene);

    return SUCCESS;
}

AudioScene AudioPolicyService::GetAudioScene() const
{
    AUDIO_INFO_LOG("GetAudioScene return value: %{public}d", mAudioScene);
    return mAudioScene;
}

bool AudioPolicyService::IsAudioInterruptEnabled() const
{
    return interruptEnabled_;
}

void AudioPolicyService::OnAudioInterruptEnable(bool enable)
{
    interruptEnabled_ = enable;
}

void AudioPolicyService::OnUpdateRouteSupport(bool isSupported)
{
    isUpdateRouteSupported_ = isSupported;
}

bool AudioPolicyService::GetActiveDeviceStreamInfo(DeviceType deviceType, AudioStreamInfo &streamInfo)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        auto streamInfoPos = connectedBTDeviceMap_.find(activeBTDevice_);
        if (streamInfoPos != connectedBTDeviceMap_.end()) {
            streamInfo.samplingRate = streamInfoPos->second.samplingRate;
            streamInfo.format = streamInfoPos->second.format;
            streamInfo.channels = streamInfoPos->second.channels;
            return true;
        }
    }

    return false;
}

bool AudioPolicyService::IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AudioStreamInfo audioStreamInfo = {};
        if (GetActiveDeviceStreamInfo(deviceType, audioStreamInfo)) {
            AUDIO_DEBUG_LOG("Device configurations current rate: %{public}d, format: %{public}d, channel: %{public}d",
                audioStreamInfo.samplingRate, audioStreamInfo.format, audioStreamInfo.channels);
            AUDIO_DEBUG_LOG("Device configurations updated rate: %{public}d, format: %{public}d, channel: %{public}d",
                streamInfo.samplingRate, streamInfo.format, streamInfo.channels);
            if ((audioStreamInfo.samplingRate != streamInfo.samplingRate)
                || (audioStreamInfo.channels != streamInfo.channels)
                || (audioStreamInfo.format != streamInfo.format)) {
                return true;
            }
        }
    }

    return false;
}

void AudioPolicyService::UpdateConnectedDevices(DeviceType devType, vector<sptr<AudioDeviceDescriptor>> &desc,
                                                bool isConnected, const std::string &macAddress,
                                                const AudioStreamInfo &streamInfo)
{
    sptr<AudioDeviceDescriptor> audioDescriptor = nullptr;

    if (std::find(ioDeviceList.begin(), ioDeviceList.end(), devType) != ioDeviceList.end()) {
        AUDIO_INFO_LOG("Filling io device list for %{public}d", devType);
        audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(devType, INPUT_DEVICE);
        desc.push_back(audioDescriptor);
        if (isConnected) {
            mConnectedDevices.insert(mConnectedDevices.begin(), audioDescriptor);
        }

        audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(devType, OUTPUT_DEVICE);
        desc.push_back(audioDescriptor);
        if (isConnected) {
            mConnectedDevices.insert(mConnectedDevices.begin(), audioDescriptor);
        }
    } else {
        AUDIO_INFO_LOG("Filling non-io device list for %{public}d", devType);
        audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(devType, GetDeviceRole(devType));
        desc.push_back(audioDescriptor);
        if (isConnected) {
            mConnectedDevices.insert(mConnectedDevices.begin(), audioDescriptor);
        }
    }
}

void AudioPolicyService::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, void *privData,
    const std::string &macAddress, const AudioStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("Device connection state updated | TYPE[%{public}d] STATUS[%{public}d]", devType, isConnected);
    int32_t result = ERROR;

    // fill device change action for callback
    std::vector<sptr<AudioDeviceDescriptor>> deviceChangeDescriptor = {};

    auto isPresent = [&devType] (const sptr<AudioDeviceDescriptor> &descriptor) {
        return descriptor->deviceType_ == devType;
    };

    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        if (isConnected) {
            connectedBTDeviceMap_.insert(make_pair(macAddress, streamInfo));
            activeBTDevice_ = macAddress;
        } else {
            connectedBTDeviceMap_.erase(macAddress);
            activeBTDevice_ = "";
        }
    }

    // If device already in list, remove it else do not modify the list
    mConnectedDevices.erase(std::remove_if(mConnectedDevices.begin(), mConnectedDevices.end(), isPresent),
                            mConnectedDevices.end());

    // new device found. If connected, add into active device list
    if (isConnected) {
        result = ActivateNewDevice(devType);
        CHECK_AND_RETURN_LOG(result == SUCCESS, "Failed to activate new device %{public}d", devType);
        mCurrentActiveDevice = devType;
        UpdateConnectedDevices(devType, deviceChangeDescriptor, isConnected, macAddress, streamInfo);
    } else {
        UpdateConnectedDevices(devType, deviceChangeDescriptor, isConnected, macAddress, streamInfo);

        auto priorityDev = FetchHighPriorityDevice();
        AUDIO_INFO_LOG("Priority device is [%{public}d]", priorityDev);

        if (priorityDev == DEVICE_TYPE_SPEAKER) {
            result = ActivateNewDevice(DEVICE_TYPE_SPEAKER);
            CHECK_AND_RETURN_LOG(result == SUCCESS, "Failed to activate new device [%{public}d]", result);

            result = ActivateNewDevice(DEVICE_TYPE_MIC);
            CHECK_AND_RETURN_LOG(result == SUCCESS, "Failed to activate new device [%{public}d]", result);
        } else {
            result = ActivateNewDevice(priorityDev);
            CHECK_AND_RETURN_LOG(result == SUCCESS, "Failed to activate new device [%{public}d]", result);
        }

        if (devType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            if (mIOHandles.find(BLUETOOTH_SPEAKER) != mIOHandles.end()) {
                mAudioPolicyManager.CloseAudioPort(mIOHandles[BLUETOOTH_SPEAKER]);
                mIOHandles.erase(BLUETOOTH_SPEAKER);
            }
        }

        mCurrentActiveDevice = priorityDev;
    }

    TriggerDeviceChangedCallback(deviceChangeDescriptor, isConnected);
}

std::vector<int32_t> AudioPolicyService::GetSupportedTones()
{
    std::vector<int> supportedToneList = {};
    for (auto i = toneDescriptorMap.begin(); i != toneDescriptorMap.end(); i++) {
        supportedToneList.push_back(i->first);
    }
    return supportedToneList;
}
std::shared_ptr<ToneInfo> AudioPolicyService::GetToneConfig(int32_t ltonetype)
{
    if (toneDescriptorMap.find(ltonetype) == toneDescriptorMap.end()) {
        return nullptr;
    }
    AUDIO_DEBUG_LOG("AudioPolicyService GetToneConfig %{public}d",ltonetype);
    return toneDescriptorMap[ltonetype];
}

void AudioPolicyService::OnDeviceConfigurationChanged(DeviceType deviceType,
    const std::string &macAddress, const AudioStreamInfo &streamInfo)
{
    AUDIO_DEBUG_LOG("OnDeviceConfigurationChanged in");
    if ((deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) && !macAddress.compare(activeBTDevice_)
        && IsDeviceActive(deviceType)) {
        if (!IsConfigurationUpdated(deviceType, streamInfo)) {
            AUDIO_DEBUG_LOG("Audio configuration same");
            return;
        }

        uint32_t bufferSize
            = (streamInfo.samplingRate * streamInfo.format * streamInfo.channels)
                / (PCM_8_BIT * BT_BUFFER_ADJUSTMENT_FACTOR);
        AUDIO_DEBUG_LOG("Updated buffer size: %{public}d", bufferSize);
        connectedBTDeviceMap_[macAddress] = streamInfo;

        auto a2dpModulesPos = deviceClassInfo_.find(ClassType::TYPE_A2DP);
        if (a2dpModulesPos != deviceClassInfo_.end()) {
            auto moduleInfoList = a2dpModulesPos->second;
            for (auto &moduleInfo : moduleInfoList) {
                if (mIOHandles.find(moduleInfo.name) != mIOHandles.end()) {
                    moduleInfo.channels = to_string(streamInfo.channels);
                    moduleInfo.rate = to_string(streamInfo.samplingRate);
                    moduleInfo.format = ConvertToHDIAudioFormat(streamInfo.format);
                    moduleInfo.bufferSize = to_string(bufferSize);

                    // First unload the existing bt sink
                    AUDIO_DEBUG_LOG("UnLoad existing a2dp module");
                    std::string currentActivePort = GetPortName(mCurrentActiveDevice);
                    AudioIOHandle activateDeviceIOHandle = mIOHandles[BLUETOOTH_SPEAKER];
                    mAudioPolicyManager.SuspendAudioDevice(currentActivePort, true);
                    mAudioPolicyManager.CloseAudioPort(activateDeviceIOHandle);

                    // Load bt sink module again with new configuration
                    AUDIO_DEBUG_LOG("Reload a2dp module [%{public}s]", moduleInfo.name.c_str());
                    AudioIOHandle ioHandle = mAudioPolicyManager.OpenAudioPort(moduleInfo);
                    CHECK_AND_RETURN_LOG(ioHandle != ERR_OPERATION_FAILED && ioHandle != ERR_INVALID_HANDLE,
                        "OpenAudioPort failed %{public}d", ioHandle);
                    mIOHandles[moduleInfo.name] = ioHandle;
                    std::string portName = GetPortName(deviceType);
                    mAudioPolicyManager.SetDeviceActive(ioHandle, deviceType, portName, true);
                    mAudioPolicyManager.SuspendAudioDevice(portName, false);

                    break;
                }
            }
        }
    }
}

void AudioPolicyService::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    AUDIO_INFO_LOG("[module_load]::OnServiceConnected for [%{public}d]", serviceIndex);
    CHECK_AND_RETURN_LOG(serviceIndex >= HDI_SERVICE_INDEX && serviceIndex <= AUDIO_SERVICE_INDEX, "invalid index");

    // If audio service or hdi service is not ready, donot load default modules
    serviceFlag_.set(serviceIndex, true);
    if (serviceFlag_.count() != MIN_SERVICE_COUNT) {
        AUDIO_INFO_LOG("[module_load]::hdi service or audio service not up. Cannot load default module now");
        return;
    }

    int32_t result = ERROR;
    AUDIO_INFO_LOG("[module_load]::HDI and AUDIO SERVICE is READY. Loading default modules");
    for (const auto &device : deviceClassInfo_) {
        if (device.first == ClassType::TYPE_PRIMARY || device.first == ClassType::TYPE_FILE_IO) {
            auto moduleInfoList = device.second;
            for (auto &moduleInfo : moduleInfoList) {
                AUDIO_INFO_LOG("[module_load]::Load module[%{public}s]", moduleInfo.name.c_str());
                AudioIOHandle ioHandle = mAudioPolicyManager.OpenAudioPort(moduleInfo);
                if (ioHandle == ERR_OPERATION_FAILED || ioHandle == ERR_INVALID_HANDLE) {
                    AUDIO_INFO_LOG("[module_load]::Open port failed");
                    continue;
                }
                mIOHandles[moduleInfo.name] = ioHandle;

                auto devType = GetDeviceType(moduleInfo.name);
                if (devType == DEVICE_TYPE_SPEAKER || devType == DEVICE_TYPE_MIC) {
                    result = mAudioPolicyManager.SetDeviceActive(ioHandle, devType, moduleInfo.name, true);
                    if (result != SUCCESS) {
                        AUDIO_ERR_LOG("[module_load]::Device failed %{public}d", devType);
                        continue;
                    }
                } else if (devType == DEVICE_TYPE_FILE_SOURCE || devType == DEVICE_TYPE_FILE_SINK) {
                    // in case of file sink and source, no need to fill connected device list
                    continue;
                }

                // add new device into active device list
                sptr<AudioDeviceDescriptor> audioDescriptor = new(std::nothrow) AudioDeviceDescriptor(devType,
                    GetDeviceRole(moduleInfo.role));
                mConnectedDevices.insert(mConnectedDevices.begin(), audioDescriptor);
                mDefaultDeviceCount++;
            }
        }
    }

    if (result == SUCCESS) {
        AUDIO_INFO_LOG("[module_load]::Setting speaker as active device on bootup");
        mCurrentActiveDevice = DEVICE_TYPE_SPEAKER;
    }
}

// Parser callbacks
void AudioPolicyService::OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmlData)
{
    AUDIO_INFO_LOG("AudioPolicyService::%{public}s, device class num [%{public}zu]", __func__, xmlData.size());
    if (xmlData.empty()) {
        AUDIO_ERR_LOG("failed to parse xml file. Received data is empty");
        return;
    }

    deviceClassInfo_ = xmlData;
}

int32_t AudioPolicyService::SetDeviceChangeCallback(const int32_t clientId, const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("Entered AudioPolicyService::%{public}s", __func__);

    sptr<IStandardAudioPolicyManagerListener> callback = iface_cast<IStandardAudioPolicyManagerListener>(object);
    if (callback != nullptr) {
        deviceChangeCallbackMap_[clientId] = callback;
    }

    return SUCCESS;
}

int32_t AudioPolicyService::UnsetDeviceChangeCallback(const int32_t clientId)
{
    AUDIO_INFO_LOG("Entered AudioPolicyService::%{public}s", __func__);

    if (deviceChangeCallbackMap_.erase(clientId)) {
        AUDIO_DEBUG_LOG("AudioPolicyServer:UnsetDeviceChangeCallback for clientID %{public}d done", clientId);
    } else {
        AUDIO_DEBUG_LOG("AudioPolicyServer:UnsetDeviceChangeCallback clientID %{public}d not present/unset already",
                        clientId);
    }

    return SUCCESS;
}

int32_t AudioPolicyService::ReconfigureAudioChannel(const uint32_t &channelCount, DeviceType deviceType)
{
    if (mCurrentActiveDevice != DEVICE_TYPE_FILE_SINK) {
        AUDIO_INFO_LOG("FILE_SINK_DEVICE is not active. Cannot reconfigure now");
        return ERROR;
    }

    std::string module = FILE_SINK;

    if (deviceType == DeviceType::DEVICE_TYPE_FILE_SINK) {
        CHECK_AND_RETURN_RET_LOG(channelCount <= CHANNEL_8 && channelCount >= MONO, ERROR, "Invalid sink channel");
        module = FILE_SINK;
    } else if (deviceType == DeviceType::DEVICE_TYPE_FILE_SOURCE) {
        CHECK_AND_RETURN_RET_LOG(channelCount <= CHANNEL_6 && channelCount >= MONO, ERROR, "Invalid src channel");
        module = FILE_SOURCE;
    } else {
        AUDIO_INFO_LOG("Invalid DeviceType");
        return ERROR;
    }

    if (mIOHandles.find(module) != mIOHandles.end()) {
        mAudioPolicyManager.CloseAudioPort(mIOHandles[module]);
        mIOHandles.erase(module);
    }

    auto fileClass = deviceClassInfo_.find(ClassType::TYPE_FILE_IO);
    if (fileClass != deviceClassInfo_.end()) {
        auto moduleInfoList = fileClass->second;
        for (auto &moduleInfo : moduleInfoList) {
            if (module == moduleInfo.name) {
                moduleInfo.channels = to_string(channelCount);
                AudioIOHandle ioHandle = mAudioPolicyManager.OpenAudioPort(moduleInfo);
                mIOHandles[moduleInfo.name] = ioHandle;
                mAudioPolicyManager.SetDeviceActive(ioHandle, deviceType, module, true);
            }
        }
    }

    return SUCCESS;
}

// private methods
AudioIOHandle AudioPolicyService::GetAudioIOHandle(InternalDeviceType deviceType)
{
    AudioIOHandle ioHandle;
    switch (deviceType) {
        case InternalDeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case InternalDeviceType::DEVICE_TYPE_USB_HEADSET:
        case InternalDeviceType::DEVICE_TYPE_SPEAKER:
            ioHandle = mIOHandles[PRIMARY_SPEAKER];
            break;
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            ioHandle = mIOHandles[PIPE_SINK];
            break;
        case InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            ioHandle = mIOHandles[BLUETOOTH_SPEAKER];
            break;
        case InternalDeviceType::DEVICE_TYPE_MIC:
            ioHandle = mIOHandles[PRIMARY_MIC];
            break;
        case InternalDeviceType::DEVICE_TYPE_FILE_SINK:
            ioHandle = mIOHandles[FILE_SINK];
            break;
        case InternalDeviceType::DEVICE_TYPE_FILE_SOURCE:
            ioHandle = mIOHandles[FILE_SOURCE];
            break;
        default:
            ioHandle = mIOHandles[PRIMARY_MIC];
            break;
    }
    return ioHandle;
}

InternalDeviceType AudioPolicyService::GetDeviceType(const std::string &deviceName)
{
    InternalDeviceType devType = InternalDeviceType::DEVICE_TYPE_NONE;
    if (deviceName == "Speaker") {
        devType = InternalDeviceType::DEVICE_TYPE_SPEAKER;
    } else if (deviceName == "Built_in_mic") {
        devType = InternalDeviceType::DEVICE_TYPE_MIC;
    } else if (deviceName == "fifo_output" || deviceName == "fifo_input") {
        devType = DEVICE_TYPE_BLUETOOTH_SCO;
    } else if (deviceName == "file_sink") {
        devType = DEVICE_TYPE_FILE_SINK;
    } else if (deviceName == "file_source") {
        devType = DEVICE_TYPE_FILE_SOURCE;
    }

    return devType;
}

void AudioPolicyService::WriteDeviceChangedSysEvents(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    for (auto deviceDescriptor : desc) {
        if (deviceDescriptor != nullptr) {
            if (deviceDescriptor->deviceType_ == DEVICE_TYPE_WIRED_HEADSET) {
                HiviewDFX::HiSysEvent::Write("AUDIO", "AUDIO_HEADSET_CHANGE",
                    HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                    "ISCONNECT", isConnected ? 1 : 0,
                    "HASMIC", 1,
                    "DEVICETYPE", DEVICE_TYPE_WIRED_HEADSET);
            }

            if (!isConnected) {
                continue;
            }

            if (deviceDescriptor->deviceRole_ == OUTPUT_DEVICE) {
                vector<SinkInput> sinkInputs = mAudioPolicyManager.GetAllSinkInputs();
                for (SinkInput sinkInput : sinkInputs) {
                    HiviewDFX::HiSysEvent::Write("AUDIO", "AUDIO_DEVICE_CHANGE",
                        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                        "ISOUTPUT", 1,
                        "STREAMID", sinkInput.streamId,
                        "STREAMTYPE", sinkInput.streamType,
                        "DEVICETYPE", deviceDescriptor->deviceType_);
                }
            } else if (deviceDescriptor->deviceRole_ == INPUT_DEVICE) {
                vector<SourceOutput> sourceOutputs = mAudioPolicyManager.GetAllSourceOutputs();
                for (SourceOutput sourceOutput : sourceOutputs) {
                    HiviewDFX::HiSysEvent::Write("AUDIO", "AUDIO_DEVICE_CHANGE",
                        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                        "ISOUTPUT", 0,
                        "STREAMID", sourceOutput.streamId,
                        "STREAMTYPE", sourceOutput.streamType,
                        "DEVICETYPE", deviceDescriptor->deviceType_);
                }
            }
        }
    }
}

void AudioPolicyService::TriggerDeviceChangedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected)
{
    DeviceChangeAction deviceChangeAction;
    deviceChangeAction.deviceDescriptors = desc;
    deviceChangeAction.type = isConnected ? DeviceChangeType::CONNECT : DeviceChangeType::DISCONNECT;

    WriteDeviceChangedSysEvents(desc, isConnected);

    for (auto it = deviceChangeCallbackMap_.begin(); it != deviceChangeCallbackMap_.end(); ++it) {
        if (it->second) {
            it->second->OnDeviceChange(deviceChangeAction);
        }
    }
}

DeviceRole AudioPolicyService::GetDeviceRole(DeviceType deviceType) const
{
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
            return DeviceRole::OUTPUT_DEVICE;
        case DeviceType::DEVICE_TYPE_MIC:
            return DeviceRole::INPUT_DEVICE;
        default:
            return DeviceRole::DEVICE_ROLE_NONE;
    }
}

DeviceRole AudioPolicyService::GetDeviceRole(const std::string &role)
{
    if (role == ROLE_SINK) {
        return DeviceRole::OUTPUT_DEVICE;
    } else if (role == ROLE_SOURCE) {
        return DeviceRole::INPUT_DEVICE;
    } else {
        return DeviceRole::DEVICE_ROLE_NONE;
    }
}

void AudioPolicyService::OnAudioLatencyParsed(uint64_t latency)
{
    audioLatencyInMsec_ = latency;
}

int32_t AudioPolicyService::GetAudioLatencyFromXml() const
{
    return audioLatencyInMsec_;
}
} // namespace AudioStandard
} // namespace OHOS
