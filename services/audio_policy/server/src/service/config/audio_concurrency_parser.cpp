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
#include <string>
#include "audio_concurrency_parser.h"
#include "audio_info.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

int32_t AudioConcurrencyParser::LoadConfig(std::map<std::pair<StreamCategory, StreamCategory>,
    ConcurrencyAction> &concurrencyMap)
{
    AUDIO_INFO_LOG("lxj load config file");
    doc_ = xmlReadFile(AUDIO_CONCURRENCY_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(doc_ != nullptr, ERR_OPERATION_FAILED, "xmlRead ConcurrencyConfigFile failed!");
    xmlNode *root = xmlDocGetRootElement(doc_);
    CHECK_AND_RETURN_RET_LOG(root != nullptr, ERR_OPERATION_FAILED, "xmlDocGetRootElement failed!");
    if (xmlStrcmp(root->name, reinterpret_cast<const xmlChar*>("audioConcurrencyPolicy"))) {
        AUDIO_ERR_LOG("Missing tag - audioConcurrencyPolicy");
        xmlFreeDoc(doc_);
        xmlCleanupParser();
        return ERR_OPERATION_FAILED;
    }

    int32_t ret = ParseInternal(root, concurrencyMap);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ParseInternal failed");
    return SUCCESS;
}
int32_t AudioConcurrencyParser::ParseInternal(xmlNode * node, std::map<std::pair<StreamCategory, StreamCategory>,
    ConcurrencyAction> &concurrencyMap)
{
        
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS