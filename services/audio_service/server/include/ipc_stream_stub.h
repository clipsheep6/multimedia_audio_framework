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

#ifndef IPC_STREAM_STUB_H
#define IPC_STREAM_STUB_H

#include "ipc_stream.h"

#include "message_parcel.h"

namespace OHOS {
namespace AudioStandard {
class IpcStreamStub : public IRemoteStub<IpcStream> {
public:
    virtual ~IpcStreamStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static bool CheckInterfaceToken(MessageParcel &data);

    int32_t HandleRegisterStreamListener(MessageParcel &data, MessageParcel &reply);
    int32_t HandleResolveBuffer(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdatePosition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAudioSessionID(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStart(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePause(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStop(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRelease(MessageParcel &data, MessageParcel &reply);
    int32_t HandleFlush(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDrain(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdatePlaybackCaptureConfig(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAudioTime(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAudioPosition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetLatency(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetRate(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetRate(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetLowPowerVolume(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetLowPowerVolume(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAudioEffectMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAudioEffectMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetAudioEnhanceMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAudioEnhanceMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetPrivacyType(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetPrivacyType(MessageParcel &data, MessageParcel &reply);

    int32_t HandleSetOffloadMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnsetOffloadMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetOffloadApproximatelyCacheTime(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOffloadSetVolume(MessageParcel &data, MessageParcel &reply);

    int32_t HandleUpdateSpatializationState(MessageParcel &data, MessageParcel &reply);

    int32_t HandleGetStreamManagerType(MessageParcel &data, MessageParcel &reply);

    int32_t HandleSetSilentModeAndMixWithOthers(MessageParcel &data, MessageParcel &reply);
    
    using HandlerFunc = int32_t(IpcStreamStub::*)(MessageParcel &data, MessageParcel &reply);
    // Using the same order in IpcStreamMsg::Code when add func!
    static inline HandlerFunc funcList_[IpcStreamMsg::IPC_STREAM_MAX_MSG] = {
        &IpcStreamStub::HandleRegisterStreamListener,
        &IpcStreamStub::HandleResolveBuffer,
        &IpcStreamStub::HandleUpdatePosition,
        &IpcStreamStub::HandleGetAudioSessionID,
        &IpcStreamStub::HandleStart,
        &IpcStreamStub::HandlePause,
        &IpcStreamStub::HandleStop,
        &IpcStreamStub::HandleRelease,
        &IpcStreamStub::HandleFlush,
        &IpcStreamStub::HandleDrain,
        &IpcStreamStub::HandleUpdatePlaybackCaptureConfig,
        &IpcStreamStub::HandleGetAudioTime,
        &IpcStreamStub::HandleGetAudioPosition,
        &IpcStreamStub::HandleGetLatency,
        &IpcStreamStub::HandleSetRate,
        &IpcStreamStub::HandleGetRate,
        &IpcStreamStub::HandleSetLowPowerVolume,
        &IpcStreamStub::HandleGetLowPowerVolume,
        &IpcStreamStub::HandleSetAudioEffectMode,
        &IpcStreamStub::HandleGetAudioEffectMode,
        &IpcStreamStub::HandleSetPrivacyType,
        &IpcStreamStub::HandleGetPrivacyType,
        &IpcStreamStub::HandleSetOffloadMode,
        &IpcStreamStub::HandleUnsetOffloadMode,
        &IpcStreamStub::HandleGetOffloadApproximatelyCacheTime,
        &IpcStreamStub::HandleOffloadSetVolume,
        &IpcStreamStub::HandleUpdateSpatializationState,
        &IpcStreamStub::HandleGetStreamManagerType,
        &IpcStreamStub::HandleSetSilentModeAndMixWithOthers,
    };
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_STREAM_STUB_H
