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

int32_t AudioConcurrencyParser::LoadConfig(std::map<std::pair<AudioPipeType, AudioPipeType>,
    ConcurrencyAction> &concurrencyMap)
{
    AUDIO_INFO_LOG("lxj load config file");
    doc_ = xmlReadFile(AUDIO_CONCURRENCY_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(doc_ != nullptr, ERR_OPERATION_FAILED, "lxj xmlRead concurrencyConfigFile failed!");
    AUDIO_INFO_LOG("lxj doc: %{public}s", doc_->name);
    xmlNode *root = xmlDocGetRootElement(doc_);
    CHECK_AND_RETURN_RET_LOG(root != nullptr, ERR_OPERATION_FAILED, "lxj xmlDocGetRootElement failed!");
    if (xmlStrcmp(root->name, reinterpret_cast<const xmlChar*>("audioConcurrencyPolicy"))) {
        AUDIO_ERR_LOG("lxj Missing tag - audioConcurrencyPolicy");
        xmlFreeDoc(doc_);
        xmlCleanupParser();
        return ERR_OPERATION_FAILED;
    }
    AUDIO_INFO_LOG("lxj head: %{public}s, head type: %{public}d", root->name, root->type);
    ParseInternal(root, concurrencyMap);

    return SUCCESS;
}
void AudioConcurrencyParser::ParseInternal(xmlNode *node, std::map<std::pair<AudioPipeType, AudioPipeType>,
    ConcurrencyAction> &concurrencyMap)
{
    xmlNode *curNode = node;
    for (; curNode; curNode = curNode->next) {
        if (curNode->type == XML_ELEMENT_NODE &&
            !xmlStrcmp(curNode->name, reinterpret_cast<const xmlChar*>("existingStream"))) {
            char *nodeName = reinterpret_cast<char*>(xmlGetProp(curNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            std::string existingStream = nodeName;
            AUDIO_INFO_LOG("lxj existingStream: %{public}s", existingStream.c_str());
            ParseIncoming(existingStream, curNode->children, concurrencyMap);
        } else {
            ParseInternal((curNode->children), concurrencyMap);
        }
    }
    return;
}

void AudioConcurrencyParser::ParseIncoming(const std::string &existing, xmlNode *node,
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> &concurrencyMap)
{
    xmlNode *incomingNode = node;
    while (incomingNode != nullptr) {
        if (incomingNode->type == XML_ELEMENT_NODE &&
            !xmlStrcmp(incomingNode->name, reinterpret_cast<const xmlChar*>("incomingStream"))) {
            char *incomingName = reinterpret_cast<char*>(xmlGetProp(incomingNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            char *actionName = reinterpret_cast<char*>(xmlGetProp(incomingNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("action"))));
            //AUDIO_INFO_LOG("lxj existing: %{public}s, incoming: %{public}s, action: %{public}s",
            //    existing.c_str(), incomingName, actionName);
            std::string incoming = incomingName;
            std::string action = actionName;
            AUDIO_INFO_LOG("lxj existing: %{public}s %{public}d, incoming: %{public}s %{public}d, action: %{public}s",
                existing.c_str(), audioPipeTypeMap[existing], incoming.c_str(), audioPipeTypeMap[incoming], action.c_str());
            std::pair<AudioPipeType, AudioPipeType> concurrencyPair =
                std::make_pair(audioPipeTypeMap[existing], audioPipeTypeMap[incoming]);
            ConcurrencyAction concurrencyAction = (action == "play both" || action == "mix") ? PLAY_BOTH :
                (action == "concede existing" ? CONCEDE_EXISTING : CONCEDE_INCOMING);
            concurrencyMap.emplace(concurrencyPair, concurrencyAction);
            xmlFree(incomingName);
            xmlFree(actionName);
        }
        incomingNode = incomingNode->next;
    }
}
} // namespace AudioStandard
} // namespace OHOS