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
#undef LOG_TAG
#define LOG_TAG "PrivacyPriorityRouter"

#include "privacy_priority_router.h"
#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

unique_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPrivacyDevices();
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

unique_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

unique_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommCapturePrivacyDevices();
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

vector<unique_ptr<AudioDeviceDescriptor>> PrivacyPriorityRouter::GetRingRenderDevice(
    StreamUsage streamUsage, int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> devices;
    unique_ptr<AudioDeviceDescriptor> desc;
    AudioRingerMode mode = audioPolicyManager_.GetRingerMode();
    if (streamUsage == STREAM_USAGE_NOTIFICATION || streamUsage == STREAM_USAGE_RINGTONE) {
        desc = GetMediaRenderDevice(streamUsage, clientUID);
        AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage, clientUID,
        desc->deviceType_);
    } else if (streamUsage == STREAM_USAGE_ALARM) {
        desc = GetCallRenderDevice(streamUsage, clientUID);
        AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage, clientUID,
        desc->deviceType_);
    }
    if ((desc->getType() != DEVICE_TYPE_SPEAKER) || desc->getType() != DEVICE_TYPE_NONE) {
        devices.push_back(move(desc));
        unique_ptr<AudioDeviceDescriptor> speaker =
            AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice();
        switch (streamUsage) {
        case STREAM_USAGE_NOTIFICATION:
        case STREAM_USAGE_RINGTONE:
            if (mode == RINGER_MODE_NORMAL) {
                devices.push_back(move(speaker));
            }
            break;
        case STREAM_USAGE_ALARM:
            devices.push_back(move(speaker));
            break;
        default:
            break;
        }
    } else if (desc->getType() == DEVICE_TYPE_SPEAKER){
        devices.push_back(move(desc));
    }
    return devices;
}

unique_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaCapturePrivacyDevices();
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

unique_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS