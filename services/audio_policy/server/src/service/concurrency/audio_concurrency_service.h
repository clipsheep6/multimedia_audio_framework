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
#ifndef ST_AUDIO_CONCURRENCY_SERVICE_H
#define ST_AUDIO_CONCURRENCY_SERVICE_H
#include<mutex>

#include "iremote_object.h"

#include "audio_info.h"
#include "audio_log.h"
#include "audio_concurrency_callback.h"
#include "i_audio_concurrency_event_dispatcher.h"
#include "audio_concurrency_parser.h"
#include "audio_policy_server_handler.h"

namespace OHOS {
namespace AudioStandard {

class AudioConcurrencyService : public std::enable_shared_from_this<AudioConcurrencyService>,
                                public IAudioConcurrencyEventDispatcher {
public:
    AudioConcurrencyService()
    {
        AUDIO_INFO_LOG("lxj AudioConcurrencyService ctor");
    }
    virtual ~AudioConcurrencyService()
    {
        AUDIO_INFO_LOG("lxj AudioConcurrencyService dtor");
    }
    void Init();
    void DispatchConcurrencyEventWithSessionId(uint32_t sessionId) override;
    int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object);
    int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID);
    void SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler);
    int32_t ActivateAudioConcurrency(const uint32_t sessionID);
    int32_t ActivateAudioConcurrency(AudioPipeType incomingPipeType,
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
private:
    class AudioConcurrencyClient : public IRemoteObject::DeathRecipient {
    public:
        explicit AudioConcurrencyClient(
            const std::shared_ptr<AudioConcurrencyService> &service,
            const std::shared_ptr<AudioConcurrencyCallback> &callback, uint32_t sessionID);
        virtual ~AudioConcurrencyClient();

        DISALLOW_COPY_AND_MOVE(AudioConcurrencyClient);

        // DeathRecipient
        void OnRemoteDied(const wptr<IRemoteObject> &remote);

        void OnConcedeStream();

    private:
        const std::weak_ptr<AudioConcurrencyService> service_;
        const std::shared_ptr<AudioConcurrencyCallback> callback_;
        const uint32_t sessionID_;
    };
    std::map<int32_t /*sessionId*/, sptr<AudioConcurrencyClient>> concurrencyClients_ = {};
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyCfgMap_ = {};
    std::shared_ptr<AudioPolicyServerHandler> handler_;
    std::mutex cbMapMutex_;
    
};
} // namespace AudioStandard
} // namespace OHOS
#endif