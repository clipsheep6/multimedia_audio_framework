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

#ifndef ST_AUDIO_STATE_MANAGER_H
#define ST_AUDIO_STATE_MANAGER_H

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioStateManager {
public:
    static AudioStateManager& GetAudioStateManager()
    {
        static AudioStateManager audioStateManager;
        return audioStateManager;
    }

    // Set media render device selected by the user
    void SetPreferredMediaRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Set call render device selected by the user
    void SetPreferredCallRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Set call capture device selected by the user
    void SetPreferredCallCaptureDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Set ring render device selected by the user
    void SetPreferredRingRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Set record capture device selected by the user
    void SetPreferredRecordCaptureDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Set tone render device selected by the user
    void SetPreferredToneRenderDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor, int32_t clientId = -1);

    // Get media render device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredMediaRenderDevice(int32_t clientId = -1);

    // Get call render device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredCallRenderDevice(int32_t clientId = -1);

    // Get call capture device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredCallCaptureDevice(int32_t clientId = -1);

    // Get ring render device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredRingRenderDevice(int32_t clientId = -1);

    // Get record capture device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredRecordCaptureDevice(int32_t clientId = -1);

    // Get tone render device selected by the user
    unique_ptr<AudioDeviceDescriptor> GetPreferredToneRenderDevice(int32_t clientId = -1);

    void UpdatePreferredMediaRenderDeviceConnectState(ConnectState state);
    void UpdatePreferredCallRenderDeviceConnectState(ConnectState state);
    void UpdatePreferredCallCaptureDeviceConnectState(ConnectState state);
    void UpdatePreferredRecordCaptureDeviceConnectState(ConnectState state);

private:
    struct UserPreferredDeviceWithClientId {
        sptr<AudioDeviceDescriptor> preferredMediaRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> preferredCallRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> preferredCallCaptureDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> preferredRingRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> preferredRecordCaptureDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> preferredToneRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
        int32_t deviceFlag = 0;
    }
    AudioStateManager() {};
    ~AudioStateManager() {};
    sptr<AudioDeviceDescriptor> preferredMediaRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    sptr<AudioDeviceDescriptor> preferredCallRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    sptr<AudioDeviceDescriptor> preferredCallCaptureDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    sptr<AudioDeviceDescriptor> preferredRingRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    sptr<AudioDeviceDescriptor> preferredRecordCaptureDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    sptr<AudioDeviceDescriptor> preferredToneRenderDevice_ = new(std::nothrow) AudioDeviceDescriptor();
    std::mutex mutex_;
    map<int32_t, UserPreferredDeviceWithClientId> userPreferredDeviceWithUidMap_;

};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_STATE_MANAGER_H

