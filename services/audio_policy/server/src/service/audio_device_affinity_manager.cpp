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

#include "audio_state_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

void DeviceAffinityManager::ParseAffinityDeviceXml()
{
    unique_ptr<audioAffinityDeviceParser> audioAffinityDeviceParser = make_unique<audioAffinityDeviceParser>(this);
    if (audioAffinityDeviceParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("Audio Affinity device manager load configuration successfully.");
        audioAffinityDeviceParser->Parse();
    }
}

void DeviceAffinityManager::OnAffinityXmlParsingCompleted(list<AffinityDeviceInfo>> &xmlData)
{
    CHECK_AND_RETURN_LOG(!xmlData.empty(), "Failed to parse xml file.");
    for (AffinityDeviceInfo deviceInfo : xmlData) {
        if (deviceInfo.isOutputDevice) {
            rendererDeviceList_.add(deviceInfo);
        } else {
            captureDeviceList_.add(deviceInfo);
        }
    }
}

unique_ptr<AudioDeviceDescriptor>  DeviceAffinityManager::GetRendererDevice(int clientUID)
{
    return activeRendererDeviceMap_[clientUID];
}

unique_ptr<AudioDeviceDescriptor>  DeviceAffinityManager::GetCaptureDevice(int clientUID)
{
    return activeCaptureDeviceMap_[clientUID];
}

void DeviceAffinityManager::addSelectRendererDevice(unique_ptr<AudioDeviceDescriptor> &desc, int clientUID)
{
    activeRendererDeviceMap_[clientUID] = desc;
}

void DeviceAffinityManager::addSelectCaptureDevice(unique_ptr<AudioDeviceDescriptor> &desc, int clientUID)
{
    activeCaptureDeviceMap_[clientUID] = desc;
}

} // namespace AudioStandard
} // namespace OHOS

