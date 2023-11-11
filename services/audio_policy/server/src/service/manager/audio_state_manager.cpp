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

void AudioStateManager::SetPerferredMediaRenderDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredMediaRenderDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
}

void AudioStateManager::SetPerferredCallRenderDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredCallRenderDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
}

void AudioStateManager::SetPerferredCallCaptureDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredCallCaptureDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
}

void AudioStateManager::SetPerferredRingRenderDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredRingRenderDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
}

void AudioStateManager::SetPerferredRecordCaptureDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredRecordCaptureDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
}

void AudioStateManager::SetPerferredToneRenderDevice(sptr<AudioDeviceDescriptor> deviceDescriptor)
{
	perferredToneRenderDevice_ = make_unique<AudioDeviceDescriptor>(deviceDescriptor);
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

