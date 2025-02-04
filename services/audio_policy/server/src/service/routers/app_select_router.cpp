/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AppSelectRouter"
#endif

#include "app_select_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
unique_ptr<AudioDeviceDescriptor> AppSelectRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

unique_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

unique_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetCapturerDevice(clientUID);
    return device;
}

vector<std::unique_ptr<AudioDeviceDescriptor>> AppSelectRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

unique_ptr<AudioDeviceDescriptor> AppSelectRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetCapturerDevice(clientUID);
    return device;
}

unique_ptr<AudioDeviceDescriptor> AppSelectRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> device =
        AudioAffinityManager::GetAudioAffinityManager().GetRendererDevice(clientUID);
    return device;
}

} // namespace AudioStandard
} // namespace OHOS
