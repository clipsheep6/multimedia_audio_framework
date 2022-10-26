/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <iostream>
#include "audio_server.h"
#include <cstddef>
#include <cstdint>
#include "message_parcel.h"
#include "audio_service_client.h"
#include "audio_manager_proxy.h"

using namespace std;

namespace OHOS {
    constexpr int32_t OFFSET = 4;
    const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
    const int32_t SYSTEM_ABILITY_ID = 3001;
    const bool RUN_ON_CREATE = false;
    namespace AudioStandard {
        uint32_t Convert2Uint32(const uint8_t *ptr)
        {
            if (ptr == nullptr)
            {
                return 0;
            }
            // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
            return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
        }
        void AudioServerFuzzTestOne(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            uint32_t code = Convert2Uint32(rawData);
            rawData = rawData + OFFSET;
            size = size - OFFSET;

            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            MessageParcel reply;
            MessageOption option;
            std::shared_ptr<AudioServer> AudioServerPtr =
                std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
            AudioServerPtr->OnRemoteRequest(code, data, reply, option);
        }

        void AudioServerFuzzTestTwo(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            std::shared_ptr<AudioServiceClient> audioServiceClient = std::make_shared<AudioServiceClient>();
            ASClientType eClientType = *reinterpret_cast<const ASClientType *>(rawData);
            audioServiceClient->Initialize(eClientType);

            AudioStreamParams audioStreamParams;
            audioStreamParams.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
            audioStreamParams.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
            audioStreamParams.channels = *reinterpret_cast<const AudioChannel *>(rawData);
            audioStreamParams.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);

            AudioStreamType audioType = *reinterpret_cast<const AudioStreamType *>(rawData);
            audioServiceClient->CreateStream(audioStreamParams, audioType);

            uint32_t sessionID = *reinterpret_cast<const uint32_t *>(rawData);
            audioServiceClient->GetSessionID(sessionID);
            audioServiceClient->SetStreamType(audioType);

            uint32_t volume = *reinterpret_cast<const uint32_t *>(rawData);
            audioServiceClient->SetStreamVolume(sessionID, volume);
            audioServiceClient->GetStreamVolume(sessionID);

            uint32_t writeLen = *reinterpret_cast<const uint32_t *>(rawData);
            audioServiceClient->RenderPrebuf(writeLen);

            uint32_t bufferLen = *reinterpret_cast<const uint32_t *>(rawData);
            auto buffer = std::make_unique<uint8_t[]>(bufferLen);

            uint64_t timeStamp = *reinterpret_cast<const int32_t *>(rawData);
            audioServiceClient->GetCurrentTimeStamp(timeStamp);

            uint64_t latency = *reinterpret_cast<const int32_t *>(rawData);
            audioServiceClient->GetAudioLatency(latency);

            audioServiceClient->GetAudioStreamParams(audioStreamParams);

            size_t minBufferSize = *reinterpret_cast<const size_t *>(rawData);
            audioServiceClient->GetMinimumBufferSize(minBufferSize);

            uint32_t frameCount = *reinterpret_cast<const uint32_t *>(rawData);
            audioServiceClient->GetMinimumFrameCount(frameCount);

            audioServiceClient->SetStreamVolume(volume);
            AudioRendererRate renderRate = *reinterpret_cast<const AudioRendererRate *>(rawData);
            audioServiceClient->SetStreamRenderRate(renderRate);
            int32_t bufferSizeInMsec = *reinterpret_cast<const int32_t *>(rawData);
            audioServiceClient->SetBufferSizeInMsec(bufferSizeInMsec);

            AudioRenderMode renderMode = *reinterpret_cast<const AudioRenderMode *>(rawData);
            audioServiceClient->SetAudioRenderMode(renderMode);

            AudioCaptureMode captureMode = *reinterpret_cast<const AudioCaptureMode *>(rawData);
            audioServiceClient->SetAudioCaptureMode(captureMode);

            std::string cachePath = *reinterpret_cast<const std::string *>(rawData);
            audioServiceClient->SetApplicationCachePath(cachePath);

            std::string permissionName;
            uint32_t appTokenId = *reinterpret_cast<const uint32_t *>(rawData);
            int32_t appUid = *reinterpret_cast<const int32_t *>(rawData);
            bool privacyFlag = *reinterpret_cast<const bool *>(rawData);
            AudioPermissionState state = *reinterpret_cast<const AudioPermissionState *>(rawData);
            audioServiceClient->VerifyClientPermission(permissionName, appTokenId, appUid, privacyFlag, state);

            float powerVolumeFactor = *reinterpret_cast<const float *>(rawData);
            audioServiceClient->SetStreamLowPowerVolume(powerVolumeFactor);

            int32_t clientPid = *reinterpret_cast<const int32_t *>(rawData);
            int32_t clientUid = *reinterpret_cast<const int32_t *>(rawData);
            audioServiceClient->SetClientID(clientPid, clientUid);
        }

        void AudioServerFuzzTestThree(const uint8_t *rawData, size_t size)
        {
            if (rawData == nullptr)
            {
                std::cout << "Invalid data" << std::endl;
                return;
            }
            MessageParcel data;
            data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
            data.WriteBuffer(rawData, size);
            data.RewindRead(0);
            sptr<IRemoteObject> object = data.ReadRemoteObject();
            std::shared_ptr<AudioManagerProxy> audioManagerProxy = std::make_shared<AudioManagerProxy>(object);
            AudioVolumeType volumeType = *reinterpret_cast<const AudioVolumeType *>(rawData);
            audioManagerProxy->GetMaxVolume(volumeType);
            audioManagerProxy->GetMinVolume(volumeType);

            bool isMute = *reinterpret_cast<const bool *>(rawData);
            audioManagerProxy->SetMicrophoneMute(isMute);

            float volume = *reinterpret_cast<const float *>(rawData);
            audioManagerProxy->SetVoiceVolume(volume);

            AudioScene audioScene = *reinterpret_cast<const AudioScene *>(rawData);
            DeviceType activeDevice = *reinterpret_cast<const DeviceType *>(rawData);
            audioManagerProxy->SetAudioScene(audioScene, activeDevice);

            DeviceFlag deviceFlag = *reinterpret_cast<const DeviceFlag *>(rawData);
            audioManagerProxy->GetDevices(deviceFlag);

            std::string key = *reinterpret_cast<const std::string *>(rawData);
            std::string networkId = *reinterpret_cast<const std::string *>(rawData);
            AudioParamKey networkKey = *reinterpret_cast<const AudioParamKey *>(rawData);
            std::string condition = *reinterpret_cast<const std::string *>(rawData);
            audioManagerProxy->GetAudioParameter(key);
            audioManagerProxy->GetAudioParameter(networkId, networkKey, condition);

            std::string value;
            audioManagerProxy->SetAudioParameter(key, value);
            audioManagerProxy->SetAudioParameter(networkId, networkKey, condition, value);

            DeviceType type = *reinterpret_cast<const DeviceType *>(rawData);
            DeviceFlag flag = *reinterpret_cast<const DeviceFlag *>(rawData);
            audioManagerProxy->UpdateActiveDeviceRoute(type, flag);

            int32_t sizeOne = *reinterpret_cast<const int32_t *>(rawData);
            audioManagerProxy->RetrieveCookie(sizeOne);

            DeviceType deviceType = *reinterpret_cast<const DeviceType *>(rawData);
            DeviceRole deviceRole = *reinterpret_cast<const DeviceRole *>(rawData);
            audioManagerProxy->GetTransactionId(deviceType, deviceRole);

            bool isStartDevice = *reinterpret_cast<const bool *>(rawData);
            audioManagerProxy->CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);

            audioManagerProxy->SetParameterCallback(object);

            bool audioMono = *reinterpret_cast<const bool *>(rawData);
            audioManagerProxy->SetAudioMonoState(audioMono);

            float audioBalance = *reinterpret_cast<const float *>(rawData);
            audioManagerProxy->SetAudioBalanceValue(audioBalance);
        }
    } // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTestOne(data, size);
    return 0;
}
extern "C" int LLVMFuzzerTestTwoInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTestTwo(data, size);
    return 0;
}
extern "C" int LLVMFuzzerTestThreeInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTestThree(data, size);
    return 0;
}