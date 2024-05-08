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
#include<memory>

#include "audio_info.h"
#include "audio_log.h"
#include "audio_concurrency_parser.h"

namespace OHOS {
namespace AudioStandard {

class AudioConcurrencyService : public std::enable_shared_from_this<AudioConcurrencyService> {
public:
    AudioConcurrencyService()
    {
        AUDIO_INFO_LOG("lxj AudioConcurrencyService ctor");
    }
    ~AudioConcurrencyService()
    {
        AUDIO_INFO_LOG("lxj AudioConcurrencyService dtor");
    }
    void Init();
private:
    std::map<std::pair<StreamCategory, StreamCategory>, ConcurrencyAction> concurrencyCfgMap_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
