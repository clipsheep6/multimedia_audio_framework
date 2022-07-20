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

#include <atomic>
#include <limits>

#include "audio_log.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief The AudioDeviceDescriptor provides
 *         different sets of audio devices and their roles
 */
AudioDeviceDescriptor::AudioDeviceDescriptor(DeviceType type, DeviceRole role) : deviceType_(type), deviceRole_(role)
{
    deviceId_ = GenerateNextId();
    audioStreamInfo_ = {};
    channelMasks_ = 0;
    deviceName_ = "";
    macAddress_ = "";

    volumeGroupId_ = 0;
    interruptGroupId_ = 0;
    networkId_ = LOCAL_NETWORK_ID;
}

AudioDeviceDescriptor::AudioDeviceDescriptor()
    : AudioDeviceDescriptor(DeviceType::DEVICE_TYPE_NONE, DeviceRole::DEVICE_ROLE_NONE)
{}

AudioDeviceDescriptor::AudioDeviceDescriptor(const AudioDeviceDescriptor &deviceDescriptor)
{
    deviceId_ = deviceDescriptor.deviceId_;
    deviceName_ = deviceDescriptor.deviceName_;
    macAddress_ = deviceDescriptor.macAddress_;
    deviceType_ = deviceDescriptor.deviceType_;
    deviceRole_ = deviceDescriptor.deviceRole_;

    supportedRates_ = deviceDescriptor.supportedRates_;
    supportedEncodings_ = deviceDescriptor.supportedEncodings_;
    supportedFormats_ = deviceDescriptor.supportedFormats_;
    supportedChannels_ = deviceDescriptor.supportedChannels_;
    supportedChannelMasks_ = deviceDescriptor.supportedChannelMasks_;

    audioStreamInfo_.channels = deviceDescriptor.audioStreamInfo_.channels;
    audioStreamInfo_.encoding = deviceDescriptor.audioStreamInfo_.encoding;
    audioStreamInfo_.format = deviceDescriptor.audioStreamInfo_.format;
    audioStreamInfo_.samplingRate = deviceDescriptor.audioStreamInfo_.samplingRate;
    channelMasks_ = deviceDescriptor.channelMasks_;
    volumeGroupId_ = deviceDescriptor.volumeGroupId_;
    interruptGroupId_ = deviceDescriptor.interruptGroupId_;
    networkId_ = deviceDescriptor.networkId_;
}

AudioDeviceDescriptor::~AudioDeviceDescriptor()
{}

int32_t AudioDeviceDescriptor::GenerateNextId() {
    static std::atomic<uint32_t> sNextUniqueId(1);
    int32_t id = sNextUniqueId++;
    while (id > std::numeric_limits<int32_t>::max()) {
        id -= std::numeric_limits<int32_t>::max();
    }

    return id;
}

bool AudioDeviceDescriptor::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(deviceType_);
    parcel.WriteInt32(deviceRole_);
    parcel.WriteInt32(deviceId_);

    parcel.WriteInt32(audioStreamInfo_.channels);
    parcel.WriteInt32(audioStreamInfo_.encoding);
    parcel.WriteInt32(audioStreamInfo_.format);
    parcel.WriteInt32(audioStreamInfo_.samplingRate);
    parcel.WriteInt32(channelMasks_);

    size_t samplingRateSize = supportedRates_.size();
    parcel.WriteInt32(samplingRateSize);
    for (const AudioSamplingRate &rate : supportedRates_) {
        parcel.WriteInt32(rate);
    }

    size_t encodingsSize = supportedEncodings_.size();
    parcel.WriteInt32(encodingsSize);
    for (const AudioEncodingType &encoding : supportedEncodings_) {
        parcel.WriteInt32(encoding);
    }

    size_t formatsSize = supportedFormats_.size();
    parcel.WriteInt32(formatsSize);
    for (const AudioSampleFormat &format : supportedFormats_) {
        parcel.WriteInt32(format);
    }

    size_t channelsSize = supportedChannels_.size();
    parcel.WriteInt32(channelsSize);
    for (const AudioChannel &channels : supportedChannels_) {
        parcel.WriteInt32(channels);
    }

    size_t channelMasksSize = supportedChannelMasks_.size();
    parcel.WriteInt32(channelMasksSize);
    for (const int32_t &channelMask : supportedChannelMasks_) {
        parcel.WriteInt32(channelMask);
    }

    parcel.WriteString(deviceName_);
    parcel.WriteString(macAddress_);

    parcel.WriteInt32(interruptGroupId_);
    parcel.WriteInt32(volumeGroupId_);
    parcel.WriteString(networkId_);
    return true;
}

sptr<AudioDeviceDescriptor> AudioDeviceDescriptor::Unmarshalling(Parcel &in)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (audioDeviceDescriptor == nullptr) {
        return nullptr;
    }

    audioDeviceDescriptor->deviceType_ = static_cast<DeviceType>(in.ReadInt32());
    audioDeviceDescriptor->deviceRole_ = static_cast<DeviceRole>(in.ReadInt32());
    audioDeviceDescriptor->deviceId_ = in.ReadInt32();

    audioDeviceDescriptor->audioStreamInfo_.channels = static_cast<AudioChannel>(in.ReadInt32());
    audioDeviceDescriptor->audioStreamInfo_.encoding = static_cast<AudioEncodingType>(in.ReadInt32());
    audioDeviceDescriptor->audioStreamInfo_.format = static_cast<AudioSampleFormat>(in.ReadInt32());
    audioDeviceDescriptor->audioStreamInfo_.samplingRate = static_cast<AudioSamplingRate>(in.ReadInt32());
    audioDeviceDescriptor->channelMasks_ = in.ReadInt32();

    int32_t i = 0;
    int32_t samplingRateSize = in.ReadInt32();
    for (i = 0; i < samplingRateSize; i++) {
        audioDeviceDescriptor->supportedRates_.push_back(static_cast<AudioSamplingRate>(in.ReadInt32()));
    }

    int32_t encodingsSize = in.ReadInt32();
    for (i = 0; i < encodingsSize; i++) {
        audioDeviceDescriptor->supportedEncodings_.push_back(static_cast<AudioEncodingType>(in.ReadInt32()));
    }

    int32_t formatsSize = in.ReadInt32();
    for (i = 0; i < formatsSize; i++) {
        audioDeviceDescriptor->supportedFormats_.push_back(static_cast<AudioSampleFormat>(in.ReadInt32()));
    }

    int32_t channelsSize = in.ReadInt32();
    for (i = 0; i < channelsSize; i++) {
        audioDeviceDescriptor->supportedChannels_.push_back(static_cast<AudioChannel>(in.ReadInt32()));
    }

    int32_t channelMasksSize = in.ReadInt32();
    for (i = 0; i < channelMasksSize; i++) {
        audioDeviceDescriptor->supportedChannelMasks_.push_back(static_cast<int32_t>(in.ReadInt32()));
    }

    audioDeviceDescriptor->deviceName_ = in.ReadString();
    audioDeviceDescriptor->macAddress_ = in.ReadString();

    audioDeviceDescriptor->interruptGroupId_ = in.ReadInt32();
    audioDeviceDescriptor->volumeGroupId_ = in.ReadInt32();
    audioDeviceDescriptor->networkId_ = in.ReadString();

    return audioDeviceDescriptor;
}

void AudioDeviceDescriptor::SetDeviceInfo(std::string deviceName, std::string macAddress)
{
    deviceName_ = deviceName;
    macAddress_ = macAddress;
}

void AudioDeviceDescriptor::SetDeviceCapabilities(const std::vector<AudioSamplingRate> &sampleRates,
    const std::vector<AudioEncodingType> &encodings, const std::vector<AudioSampleFormat> &formats,
    const std::vector<AudioChannel> &channels, const std::vector<int32_t> &channelMasks)
{
    if (sampleRates.empty() || encodings.empty() || formats.empty() || channels.empty() || channelMasks.empty()) {
        AUDIO_ERR_LOG("Set device capabilities failed");
        return;
    }

    supportedRates_.clear();
    supportedEncodings_.clear();
    supportedFormats_.clear();
    supportedChannels_.clear();
    supportedChannelMasks_.clear();

    audioStreamInfo_.samplingRate = sampleRates[0];
    audioStreamInfo_.encoding = encodings[0];
    audioStreamInfo_.format = formats[0];
    audioStreamInfo_.channels = channels[0];
    channelMasks_ = channelMasks[0];

    supportedRates_.insert(supportedRates_.begin(), sampleRates.begin(), sampleRates.end());
    supportedEncodings_.insert(supportedEncodings_.begin(), encodings.begin(), encodings.end());
    supportedFormats_.insert(supportedFormats_.begin(), formats.begin(), formats.end());
    supportedChannels_.insert(supportedChannels_.begin(), channels.begin(), channels.end());
    supportedChannelMasks_.insert(supportedChannelMasks_.begin(), channelMasks.begin(), channelMasks.end());
}
} // namespace AudioStandard
} // namespace OHOS
