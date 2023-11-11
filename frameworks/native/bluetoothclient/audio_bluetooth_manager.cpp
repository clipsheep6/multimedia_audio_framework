/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "audio_bluetooth_manager.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "bluetooth_def.h"
#include "bluetooth_device_manager.h"
#include "bluetooth_device_utils.h"

namespace OHOS {
namespace Bluetooth {
using namespace AudioStandard;

A2dpSource *AudioA2dpManager::a2dpInstance_ = nullptr;
AudioA2dpListener AudioA2dpManager::a2dpListener_;
HandsFreeAudioGateway *AudioHfpManager::hfpInstance_ = nullptr;
AudioHfpListener AudioHfpManager::hfpListener_;
int AudioA2dpManager::connectionState_ = static_cast<int>(BTConnectState::DISCONNECTED);
BluetoothRemoteDevice AudioA2dpManager::activeA2dpDevice_;
std::mutex g_a2dpInstanceLock;

static bool GetAudioStreamInfo(A2dpCodecInfo codecInfo, AudioStreamInfo &audioStreamInfo)
{
    switch (codecInfo.sampleRate) {
        case A2DP_SBC_SAMPLE_RATE_48000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
            break;
        case A2DP_SBC_SAMPLE_RATE_44100_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_44100;
            break;
        case A2DP_SBC_SAMPLE_RATE_32000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_32000;
            break;
        case A2DP_SBC_SAMPLE_RATE_16000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_16000;
            break;
        case A2DP_L2HCV2_SAMPLE_RATE_96000_USER:
            audioStreamInfo.samplingRate = SAMPLE_RATE_96000;
            break;
        default:
            return false;
    }
    switch (codecInfo.bitsPerSample) {
        case A2DP_SAMPLE_BITS_16_USER:
            audioStreamInfo.format = SAMPLE_S16LE;
            break;
        case A2DP_SAMPLE_BITS_24_USER:
            audioStreamInfo.format = SAMPLE_S24LE;
            break;
        case A2DP_SAMPLE_BITS_32_USER:
            audioStreamInfo.format = SAMPLE_S32LE;
            break;
        default:
            return false;
    }
    switch (codecInfo.channelMode) {
        case A2DP_SBC_CHANNEL_MODE_STEREO_USER:
            audioStreamInfo.channels = STEREO;
            break;
        case A2DP_SBC_CHANNEL_MODE_MONO_USER:
            audioStreamInfo.channels = MONO;
            break;
        default:
            return false;
    }
    audioStreamInfo.encoding = ENCODING_PCM;
    return true;
}

void AudioA2dpManager::RegisterBluetoothA2dpListener()
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "Failed to obtain A2DP profile instance");
    a2dpInstance_->RegisterObserver(&a2dpListener_);
}

void AudioA2dpManager::UnregisterBluetoothA2dpListener()
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    CHECK_AND_RETURN_LOG(a2dpInstance_ != nullptr, "A2DP profile instance unavailable");
    a2dpInstance_->DeregisterObserver(&a2dpListener_);
    a2dpInstance_ = nullptr;
}

void AudioA2dpManager::DisconnectBluetoothA2dpSink()
{
    int connectionState = static_cast<int>(BTConnectState::DISCONNECTED);
    a2dpListener_.OnConnectionStateChanged(activeA2dpDevice_, connectionState);
    MediaBluetoothDeviceManager::ClearAllA2dpBluetoothDevice();
}

int32_t AudioA2dpManager::SetActiveA2dpDevice(const std::string& macAddress)
{
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "Failed to obtain A2DP profile instance");
    BluetoothRemoteDevice device;
    if (MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("SetActiveA2dpDevice: the configuring A2DP device doesn't exist.");
        return ERROR;
    }
    int32_t ret = a2dpInstance_->SetActiveSinkDevice(device);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "SetActiveA2dpDevice failed. result: %{public}d", ret);
    activeA2dpDevice_ = device;
    return SUCCESS;
}

int32_t AudioA2dpManager::SetDeviceAbsVolume(const std::string& macAddress, int32_t volume)
{
    BluetoothRemoteDevice device;
    if (MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("SetDeviceAbsVolume: the configuring A2DP device doesn't exist.");
        return ERROR;
    }
    return AvrcpTarget::GetProfile()->SetDeviceAbsoluteVolume(device, volume);
}

int32_t AudioA2dpManager::GetA2dpDeviceStreamInfo(const std::string& macAddress,
    AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::mutex> a2dpLock(g_a2dpInstanceLock);
    a2dpInstance_ = A2dpSource::GetProfile();
    CHECK_AND_RETURN_RET_LOG(a2dpInstance_ != nullptr, ERROR, "Failed to obtain A2DP profile instance");
    BluetoothRemoteDevice device;
    if (MediaBluetoothDeviceManager::GetConnectedA2dpBluetoothDevice(macAddress, device) != SUCCESS) {
        AUDIO_ERR_LOG("GetA2dpDeviceStreamInfo: the configuring A2DP device doesn't exist.");
        return ERROR;
    }
    A2dpCodecStatus codecStatus = a2dpInstance_->GetCodecStatus(device);
    if (!GetAudioStreamInfo(codecStatus.codecInfo, streamInfo)) {
        AUDIO_ERR_LOG("GetA2dpDeviceStreamInfo: Unsupported a2dp codec info");
        return ERROR;
    }
    return SUCCESS;
}

bool AudioA2dpManager::HasA2dpDeviceConnected()
{
    a2dpInstance_ = A2dpSource::GetProfile();
    if (!a2dpInstance_) {
        return false;
    }
    std::vector<int32_t> states {static_cast<int32_t>(BTConnectState::CONNECTED)};
    std::vector<BluetoothRemoteDevice> devices;
    a2dpInstance_->GetDevicesByStates(states, devices);

    return !devices.empty();
}

void AudioA2dpListener::OnConnectionStateChanged(const BluetoothRemoteDevice &device, int state)
{
    AUDIO_INFO_LOG("OnConnectionStateChanged: state: %{public}d", state);
    // Record connection state and device for hdi start time to check
    AudioA2dpManager::SetConnectionState(state);
    if (state == static_cast<int>(BTConnectState::CONNECTED)) {
        MediaBluetoothDeviceManager::SetMediaStack(device, CONNECT);
    }
    if (state == static_cast<int>(BTConnectState::DISCONNECTED)) {
        MediaBluetoothDeviceManager::SetMediaStack(device, DISCONNECT);
    }
}

void AudioA2dpListener::OnConfigurationChanged(const BluetoothRemoteDevice &device, const A2dpCodecInfo &codecInfo,
    int error)
{
    AUDIO_INFO_LOG("OnConfigurationChanged: sampleRate: %{public}d, channels: %{public}d, format: %{public}d",
        codecInfo.sampleRate, codecInfo.channelMode, codecInfo.bitsPerSample);
    AudioStreamInfo streamInfo = {};
    if (!GetAudioStreamInfo(codecInfo, streamInfo)) {
        AUDIO_ERR_LOG("OnConfigurationChanged: Unsupported a2dp codec info");
        return;
    }
    MediaBluetoothDeviceManager::UpdateA2dpDeviceConfiguration(device, streamInfo);
}

void AudioA2dpListener::OnPlayingStatusChanged(const BluetoothRemoteDevice &device, int playingState, int error)
{
    AUDIO_INFO_LOG("OnPlayingStatusChanged, state: %{public}d, error: %{public}d", playingState, error);
}

void AudioA2dpListener::OnMediaStackChanged(const BluetoothRemoteDevice &device, int action)
{
    AUDIO_INFO_LOG("OnMediaStackChanged, action: %{public}d", action);
    MediaBluetoothDeviceManager::SetMediaStack(device, action);
}

void AudioHfpManager::RegisterBluetoothScoListener()
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    hfpInstance_ = HandsFreeAudioGateway::GetProfile();
    CHECK_AND_RETURN_LOG(hfpInstance_ != nullptr, "Failed to obtain HFP AG profile instance");

    hfpInstance_->RegisterObserver(&hfpListener_);
}

void AudioHfpManager::UnregisterBluetoothScoListener()
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    CHECK_AND_RETURN_LOG(hfpInstance_ != nullptr, "HFP AG profile instance unavailable");

    hfpInstance_->DeregisterObserver(&hfpListener_);
    hfpInstance_ = nullptr;
}

void AudioHfpListener::OnScoStateChanged(const BluetoothRemoteDevice &device, int state)
{
    AUDIO_INFO_LOG("Entered %{public}s [%{public}d]", __func__, state);
    HfpScoConnectState scoState = static_cast<HfpScoConnectState>(state);
    if (scoState == HfpScoConnectState::SCO_CONNECTED || scoState == HfpScoConnectState::SCO_DISCONNECTED) {
        bool isConnected = (scoState == HfpScoConnectState::SCO_CONNECTED) ? true : false;
        HfpBluetoothDeviceManager::OnHfpStackChanged(device, isConnected);
    }
}
} // namespace Bluetooth
} // namespace OHOS
