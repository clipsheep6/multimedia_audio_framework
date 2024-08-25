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
#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static int32_t PREFERRED_MEDIA_RENDER = 1 << 0;
static int32_t PREFERRED_CALL_RENDER = 1 << 1;
static int32_t PREFERRED_CALL_CAPTURE = 1 << 2;
static int32_t PREFERRED_RING_RENDER = 1 << 3;
static int32_t PREFERRED_RECORD_CAPTURE = 1 << 4;
static int32_t PREFERRED_TONE_RENDER = 1 << 4;

void AudioStateManager::SetPreferredMediaRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    preferredMediaRenderDevice_ = deviceDescriptor;
    if (clientId == -1) {
        preferredMediaRenderDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_MEDIA_RENDER;
        preferredWithClientId.preferredMediaRenderDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_MEDIA_RENDER;
    preferredWithClientId.preferredMediaRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId == -1) {
        preferredCallRenderDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_CALL_RENDER;
        preferredWithClientId.preferredCallRenderDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_CALL_RENDER;
    preferredWithClientId.preferredCallRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredCallCaptureDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId == -1) {
        preferredCallCaptureDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_CALL_CAPTURE;
        preferredWithClientId.preferredCallCaptureDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_CALL_CAPTURE;
    preferredWithClientId.preferredCallCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRingRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId == -1) {
        preferredRingRenderDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_RING_RENDER;
        preferredWithClientId.preferredRingRenderDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_RING_RENDER;
    preferredWithClientId.preferredRingRenderDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredRecordCaptureDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId == -1) {
        preferredRecordCaptureDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_RECORD_CAPTURE;
        preferredWithClientId.preferredRecordCaptureDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_RECORD_CAPTURE;
    preferredWithClientId.preferredRecordCaptureDevice_ = deviceDescriptor;
}

void AudioStateManager::SetPreferredToneRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId == -1) {
        preferredToneRenderDevice_ = deviceDescriptor;
        return;
    }
    if (userPreferredDeviceWithUidMap_.count(clientId) == 0) {
        UserPreferredDeviceWithClientId preferredWithClientId;
        userPreferredDeviceWithUidMap_[clientId] = preferredWithClientId;
        preferredWithClientId.deviceFlag &= PREFERRED_TONE_RENDER;
        preferredWithClientId.preferredToneRenderDevice_ = deviceDescriptor;
    }
    UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
    preferredWithClientId.deviceFlag &= PREFERRED_TONE_RENDER;
    preferredWithClientId.preferredToneRenderDevice_ = deviceDescriptor;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredMediaRenderDevice(int32_t clientId)
{
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_MEDIA_RENDER == PREFERRED_MEDIA_RENDER) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredMediaRenderDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredMediaRenderDevice_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallRenderDevice(int32_t clientId)
{
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_CALL_RENDER == PREFERRED_CALL_RENDER) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredCallRenderDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredCallRenderDevice_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredCallCaptureDevice(int32_t clientId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_CALL_CAPTURE == PREFERRED_CALL_CAPTURE) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredCallCaptureDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredCallCaptureDevice_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRingRenderDevice(int32_t clientId)
{
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_RING_RENDER == PREFERRED_RING_RENDER) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredRingRenderDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredRingRenderDevice_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredRecordCaptureDevice(int32_t clientId)
{
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_RECORD_CAPTURE == PREFERRED_RECORD_CAPTURE) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredRecordCaptureDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredRecordCaptureDevice_);
    return devDesc;
}

unique_ptr<AudioDeviceDescriptor> AudioStateManager::GetPreferredToneRenderDevice(int32_t clientId)
{
    if (clientId != -1 && userPreferredDeviceWithUidMap_.count(clientId) > 0) {
        UserPreferredDeviceWithClientId preferredWithClientId = userPreferredDeviceWithUidMap_[clientId];
        if (preferredWithClientId.deviceFlag & PREFERRED_TONE_RENDER == PREFERRED_TONE_RENDER) {
            return make_unique<AudioDeviceDescriptor>(preferredWithClientId.preferredToneRenderDevice_);
        }
    }
    unique_ptr<AudioDeviceDescriptor> devDesc = make_unique<AudioDeviceDescriptor>(preferredToneRenderDevice_);
    return devDesc;
}

void AudioStateManager::UpdatePreferredMediaRenderDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredMediaRenderDevice_ != nullptr, "preferredMediaRenderDevice_ is nullptr");
    preferredMediaRenderDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredCallRenderDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredCallRenderDevice_ != nullptr, "preferredCallRenderDevice_ is nullptr");
    preferredCallRenderDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredCallCaptureDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredCallCaptureDevice_ != nullptr, "preferredCallCaptureDevice_ is nullptr");
    preferredCallCaptureDevice_->connectState_ = state;
}

void AudioStateManager::UpdatePreferredRecordCaptureDeviceConnectState(ConnectState state)
{
    CHECK_AND_RETURN_LOG(preferredRecordCaptureDevice_ != nullptr, "preferredRecordCaptureDevice_ is nullptr");
    preferredRecordCaptureDevice_->connectState_ = state;
}
} // namespace AudioStandard
} // namespace OHOS
