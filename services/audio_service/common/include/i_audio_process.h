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

#ifndef I_AUDIO_PROCESS_H
#define I_AUDIO_PROCESS_H

#include <memory>

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "audio_info.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class AudioProcess {
public:
    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) = 0;

    virtual int32_t Start() = 0;

    virtual int32_t Pause(bool isFlush) = 0;

    virtual int32_t Resume() = 0;

    virtual int32_t Stop() = 0;

    virtual int32_t RequestHandleInfo() = 0;

    virtual int32_t Release() = 0;

    virtual ~AudioProcess() = default;
};

class IAudioProcess : public AudioProcess, public IRemoteBroker {
public:
    static int32_t WriteConfigToParcel(const AudioProcessConfig &config, MessageParcel &parcel);

    static int32_t ReadConfigFromParcel(AudioProcessConfig &config, MessageParcel &parcel);

    virtual ~IAudioProcess() = default;

    // IPC code.
    enum IAudioProcessMsg : uint32_t {
        ON_RESOLVE_BUFFER = 0,
        ON_START,
        ON_PAUSE,
        ON_RESUME,
        ON_STOP,
        ON_REQUEST_HANDLE_INFO,
        ON_RELEASE,
        PROCESS_MAX_MSG
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioProcess");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_PROCESS_H
