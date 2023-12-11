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

void AudioStateManager::SetPerferredMediaRenderDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredMediaRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPerferredCallRenderDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredCallRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPerferredCallCaptureDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredCallCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPerferredRingRenderDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredRingRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPerferredRecordCaptureDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredRecordCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPerferredToneRenderDevice(const unique_ptr<AudioDeviceDescriptor> &deviceDescriptor)
{
    perferredToneRenderDevice_ = deviceDescriptor;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredMediaRenderDevice()
{
    return perferredMediaRenderDevice_;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredCallRenderDevice()
{
    return perferredCallRenderDevice_;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredCallCaptureDevice()
{
    return perferredCallCaptureDevice_;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredRingRenderDevice()
{
    return perferredRingRenderDevice_;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredRecordCaptureDevice()
{
    return perferredRecordCaptureDevice_;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPerferredToneRenderDevice()
{
    return perferredToneRenderDevice_;
}

} // namespace AudioStandard
} // namespace OHOS

