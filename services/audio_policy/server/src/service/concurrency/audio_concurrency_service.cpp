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

#include "audio_concurrency_service.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
void AudioConcurrencyService::Init()
{
    AUDIO_INFO_LOG("lxj AudioConcurrencyService Init");
    std::unique_ptr<AudioConcurrencyParser> parser = std::make_unique<AudioConcurrencyParser>();
    CHECK_AND_RETURN_LOG(parser != nullptr, "Create parser failed!");
    CHECK_AND_RETURN_LOG(!parser->LoadConfig(concurrencyCfgMap_), "Load cfgMap failed!");
}
} // namespace AudioStandard
} // namespace OHOS