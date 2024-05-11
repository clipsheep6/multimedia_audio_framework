/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include<map>
#include<string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

static std::map<std::string, StreamCategory> streamCategoryMap = {
    {"primary out", PRIMARY_OUT},
    {"primary in", PRIMARY_IN},
    {"lowlatency out", LOWLATENCY_OUT},
    {"lowlatency in", LOWLATENCY_IN},
    {"offload", OFFLOAD},
    {"multichannel", MULTICHANNEL},
    {"direct out", DIRECT_OUT},
    {"direct in", DIRECT_IN},
    {"call out", CALL_OUT},
    {"call in", CALL_IN},
};

enum ConcurrencyAction {
    PLAY_BOTH,
    CONCEDE_INCOMING,
    CONCEDE_EXISTING,
};

class AudioConcurrencyParser {
public:
    static constexpr char AUDIO_CONCURRENCY_CONFIG_FILE[] = "system/etc/audio/audio_concurrency_config.xml";
    AudioConcurrencyParser()
    {
        AUDIO_INFO_LOG("lxj AudioConcurrencyParser ctor");
    }
    virtual ~AudioConcurrencyParser()
    {
        if (doc_ != nullptr) {
            xmlFreeDoc(doc_);
            AUDIO_INFO_LOG("lxj xml doc freed in dtor");
        }
        AUDIO_INFO_LOG("lxj AudioConcurrencyParser dtor");
    }
    int32_t LoadConfig(std::map<std::pair<StreamCategory, StreamCategory>, ConcurrencyAction> &concurrencyMap);

private:
    int32_t ParseInternal(xmlNode * node, std::map<std::pair<StreamCategory, StreamCategory>,
        ConcurrencyAction> &concurrencyMap);
    xmlDoc *doc_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS