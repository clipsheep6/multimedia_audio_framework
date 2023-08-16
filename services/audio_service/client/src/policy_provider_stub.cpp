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

#include "policy_provider_stub.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
bool PolicyProviderStub::CheckInterfaceToken(MessageParcel &data)
{
    static auto locaDescriptor = IPolicyProviderIpc::GetDescriptor();
    auto remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != remoteDescriptor) {
        AUDIO_ERR_LOG("CheckInterFfaceToken failed.");
        return false;
    }
    return true;
}

int PolicyProviderStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (!CheckInterfaceToken(data)) {
        return AUDIO_ERR;
    }
    if (code >= IPolicyProviderMsg::POLICY_PROVIDER_MAX_MSG) {
        AUDIO_WARNING_LOG("OnRemoteRequest unsupported request code:%{public}d.", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*funcList_[code])(data, reply);
}

int32_t PolicyProviderStub::HandleGetProcessDeviceInfo(MessageParcel &data, MessageParcel &reply)
{
    AudioProcessConfig config;
    int32_t ret = ProcessConfig::ReadConfigFromParcel(config, data);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ReadConfigFromParcel failed %{public}d", ret);
    DeviceInfo deviceInfo;
    ret = GetProcessDeviceInfo(config, deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "GetProcessDeviceInfo failed %{public}d", ret);

    reply.WriteInt32(deviceInfo.deviceType);
    reply.WriteInt32(deviceInfo.deviceRole);
    reply.WriteInt32(deviceInfo.deviceId);
    reply.WriteInt32(deviceInfo.channelMasks);
    reply.WriteInt32(deviceInfo.channelIndexMasks);
    reply.WriteString(deviceInfo.deviceName);
    reply.WriteString(deviceInfo.macAddress);

    // AudioStreamInfo
    reply.WriteInt32(deviceInfo.audioStreamInfo.samplingRate);
    reply.WriteInt32(deviceInfo.audioStreamInfo.encoding);
    reply.WriteInt32(deviceInfo.audioStreamInfo.format);
    reply.WriteInt32(deviceInfo.audioStreamInfo.channels);

    reply.WriteString(deviceInfo.networkId);
    reply.WriteString(deviceInfo.displayName);
    reply.WriteInt32(deviceInfo.interruptGroupId);
    reply.WriteInt32(deviceInfo.volumeGroupId);

    return AUDIO_OK;
}

int32_t PolicyProviderStub::HandleInitSharedVolume(MessageParcel &data, MessageParcel &reply)
{
    (void)data;
    std::shared_ptr<AudioSharedMemory> buffer = nullptr;
    int32_t ret = InitSharedVolume(buffer);
    if (ret == SUCCESS && buffer != nullptr) {
        ret = AudioSharedMemory::WriteToParcel(buffer, reply);
    } else {
        AUDIO_ERR_LOG("error: ResolveBuffer failed.");
        return AUDIO_INVALID_PARAM;
    }
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
