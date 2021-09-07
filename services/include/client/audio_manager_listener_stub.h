/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef AUDIO_MANAGER_LISTENER_STUB_H
#define AUDIO_MANAGER_LISTENER_STUB_H

#include "audio_system_manager.h"
#include "i_standard_audio_manager_listener.h"

namespace OHOS {
namespace AudioStandard {
class AudioManagerListenerStub : public IRemoteStub<IStandardAudioManagerListener> {
public:
    AudioManagerListenerStub();
    virtual ~AudioManagerListenerStub();

    // IStandardAudioManagerListener override
    int OnRemoteRequest(uint32_t code, MessageParcel &data,
                                MessageParcel &reply, MessageOption &option) override;
    void OnError(AudioManagerErrorType errorType, int32_t errorCode) override;
    void OnInfo(AudioManagerInfoType type, int32_t extra) override;
    
    // AudioManagerListenerStub
    void SetCallback(const std::weak_ptr<AudioManagerCallback> &callback);
private:
    std::weak_ptr<AudioManagerCallback> callback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_MANAGER_LISTENER_STUB_H
