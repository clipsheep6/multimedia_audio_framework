/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "audio_service.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "remote_audio_renderer_sink.h"

namespace OHOS {
namespace AudioStandard {
AudioService *AudioService::GetInstance()
{
    static AudioService AudioService;

    return &AudioService;
}

AudioService::AudioService()
{
    AUDIO_INFO_LOG("AudioService()");
}

AudioService::~AudioService()
{
    AUDIO_INFO_LOG("~AudioService()");
}

inline void DumpProcessConfig(const AudioProcessConfig &config)
{
    AUDIO_INFO_LOG("Dump AudioProcessConfig: sample rate:%{public}d", config.streamInfo.samplingRate);
}

int32_t AudioService::OnProcessRelease(IAudioProcessStream *process)
{
    std::lock_guard<std::mutex> lock(processListMutex_);
    bool isFind = false;
    int32_t ret = ERROR;
    auto paired = linkedPairedList_.begin();
    while (paired != linkedPairedList_.end()) {
        if ((*paired).first == process) {
            ret = UnlinkProcessToEndpoint((*paired).first, (*paired).second);
            linkedPairedList_.erase(paired);
            isFind = true;
            break;
        } else {
            paired++;
        }
    }
    if (isFind) {
        AUDIO_INFO_LOG("OnProcessRelease find and release process result %{public}d", ret);
    } else {
        AUDIO_WARNING_LOG("OnProcessRelease can not find target process");
    }

    return SUCCESS;
}

sptr<AudioProcessInServer> AudioService::GetAudioProcess(const AudioProcessConfig &config)
{
    DumpProcessConfig(config);

    curInRemote_ = true;
    DeviceInfo deviceInfo = GetDeviceInfoForProcess(config, curInRemote_);
    std::shared_ptr<AudioEndpoint> audioEndpoint = GetAudioEndpointForDevice(deviceInfo);
    CHECK_AND_RETURN_RET_LOG(audioEndpoint != nullptr, nullptr, "no endpoint found for the process");

    uint32_t totalSizeInframe = 0;
    uint32_t spanSizeInframe = 0;
    audioEndpoint->GetPreferBufferInfo(totalSizeInframe, spanSizeInframe);

    std::lock_guard<std::mutex> lock(processListMutex_);
    sptr<AudioProcessInServer> process = AudioProcessInServer::Create(config, this);
    CHECK_AND_RETURN_RET_LOG(process != nullptr, nullptr, "AudioProcessInServer create failed.");

    int32_t ret = process->ConfigProcessBuffer(totalSizeInframe, spanSizeInframe);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "ConfigProcessBuffer failed");

    ret = LinkProcessToEndpoint(process, audioEndpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "LinkProcessToEndpoint failed");

    process_ = process;
    linkedPairedList_.push_back(std::make_pair(process, audioEndpoint));
    return process;
}

int32_t AudioService::LinkProcessToEndpoint(sptr<AudioProcessInServer> process,
    std::shared_ptr<AudioEndpoint> endpoint)
{
    AUDIO_INFO_LOG("linkProcessToEndpoint enter");
    int32_t ret = endpoint->LinkProcessStream(process);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "LinkProcessStream failed");

    ret = process->AddProcessStatusListener(endpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "AddProcessStatusListener failed");
    AUDIO_INFO_LOG("linkProcessToEndpoint exit");
    return SUCCESS;
}

int32_t AudioService::UnLinkProcessToEndpoint(sptr<AudioProcessInServer> process,
    std::shared_ptr<AudioEndpoint> endpoint)
{
    AUDIO_INFO_LOG("UnlinkProcessToEndpoint enter");
    int32_t ret = endpoint->UnlinkProcessStream(process);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UnLinkProcessStream failed");

    ret = process->RemoveProcessStatusListener(endpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "RemoveProcessStatusListener failed");
    AUDIO_INFO_LOG("UnlinkProcessToEndpoint exit");
    return SUCCESS;
}

int32_t AudioService::UnlinkProcessToEndpoint(sptr<AudioProcessInServer> process,
    std::shared_ptr<AudioEndpoint> endpoint)
{
    int32_t ret = endpoint->UnlinkProcessStream(process);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UnlinkProcessStream failed");

    ret = process->RemoveProcessStatusListener(endpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "RemoveProcessStatusListener failed");

    return SUCCESS;
}

int32_t AudioService::ChangeProcessToEndpoint(sptr<AudioProcessInServer> process, const AudioProcessConfig &config)
{
    std::shared_ptr<AudioEndpoint> oldEndpoint = nullptr;
    std::shared_ptr<AudioEndpoint> newEndpoint = nullptr;
    if (!curInRemote_) {
        oldEndpoint = GetAudioEndpointForDevice(GetDeviceInfoForProcess(config, false));
        newEndpoint = GetAudioEndpointForDevice(GetDeviceInfoForProcess(config, true));
    } else {
        oldEndpoint = GetAudioEndpointForDevice(GetDeviceInfoForProcess(config, true));
        newEndpoint = GetAudioEndpointForDevice(GetDeviceInfoForProcess(config, false));
    }
    AUDIO_INFO_LOG("Change Unlink oldEndpoint enter");
    int32_t ret = UnLinkProcessToEndpoint(process_, oldEndpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "UnLinkProcessStream oldEndpoint failed.");
    AUDIO_INFO_LOG("Change Unlink oldEndpoint exit");

    ret = LinkProcessToEndpoint(process_, newEndpoint);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "LinkProcessStream newEndpoint failed.");

    AUDIO_INFO_LOG("Change Link newEndpoint exit");
    curInRemote_ = !curInRemote_;
    return SUCCESS;
}

DeviceInfo AudioService::GetDeviceInfoForProcess(const AudioProcessConfig &config, bool isRemote)
{
    // send the config to AudioPolicyServera and get the device info.
    DeviceInfo deviceInfo;
    deviceInfo.deviceId = 6; // 6 for test
    if (isRemote) {
        std::string networkId = RemoteAudioRendererSink::GetDeviceNetworkId();
        AUDIO_INFO_LOG("get device network Id: %{public}s.", networkId.c_str());
        deviceInfo.networkId = networkId;
    } else {
        deviceInfo.networkId = LOCAL_NETWORK_ID;
    }
    deviceInfo.deviceRole = OUTPUT_DEVICE;
    deviceInfo.deviceType = DEVICE_TYPE_SPEAKER;
    deviceInfo.audioStreamInfo = config.streamInfo;
    deviceInfo.deviceName = "mmap_device";
    return deviceInfo;
}

std::shared_ptr<AudioEndpoint> AudioService::GetAudioEndpointForDevice(DeviceInfo deviceInfo)
{
    std::string deviceKey = deviceInfo.networkId + std::to_string(deviceInfo.deviceId);
    if (endpointList_.find(deviceKey) != endpointList_.end()) {
        AUDIO_INFO_LOG("AudioService find endpoint already exist for deviceKey:%{public}s", deviceKey.c_str());
        return endpointList_[deviceKey];
    }
    std::shared_ptr<AudioEndpoint> endpoint = AudioEndpoint::GetInstance(AudioEndpoint::EndpointType::TYPE_MMAP,
        deviceInfo.audioStreamInfo, deviceInfo.networkId);
    if (endpoint == nullptr) {
        AUDIO_ERR_LOG("Find no endpoint for the process");
        return nullptr;
    }
    endpointList_[deviceKey] = endpoint;
    return endpoint;
}

void AudioService::Dump(std::stringstream &dumpStringStream)
{
    AUDIO_INFO_LOG("AudioService dump begin");
    // dump process
    for (auto paired : linkedPairedList_) {
        paired.first->Dump(dumpStringStream);
    }
    // dump endpoint
    for (auto item : endpointList_) {
        dumpStringStream << std::endl << "Endpoint device id:" << item.first << std::endl;
        item.second->Dump(dumpStringStream);
    }
}
} // namespace AudioStandard
} // namespace OHOS
