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
 
#include "audio_strategy_router_parser.h"
#include <vector>

namespace OHOS {
namespace AudioStandard {
bool AudioStrategyRouterParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(DEVICE_CONFIG_FILE, nullptr, 0);
    if (mDoc_ == nullptr) {
        AUDIO_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    return true;
}

bool AudioStrategyRouterParser::Parse()
{
    xmlNode *root = xmlDocGetRootElement(mDoc_);
    if (root == nullptr) {
        AUDIO_ERR_LOG("xmlDocGetRootElement Failed");
        return false;
    }

    if (!ParseInternal(root)) {
        return false;
    }
    return true;
}

void AudioStrategyRouterParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool AudioStrategyRouterParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;

    for (; currNode; currNode = currNode->next) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: node:%{public}s %{public}d %{public}d", currNode->name, currNode->type, XML_ELEMENT_NODE);
        if (XML_ELEMENT_NODE == currNode->type) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("strategy"))) {
                char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                    AUDIO_INFO_LOG("AudioStrategyRouterParser: node:%{public}s", pValue);
                ParserStrategyInfo(currNode);
            } else {
                ParseInternal((currNode->xmlChildrenNode));
            }
        }
    }
    for (auto& devicePrivacy : audioRouterCenter_.mediaRenderRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: mediaRenderRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    }
    for (auto& devicePrivacy : audioRouterCenter_.callRenderRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: callRenderRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    }
    for (auto& devicePrivacy : audioRouterCenter_.callCaptureRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: callCaptureRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    }
    for (auto& devicePrivacy : audioRouterCenter_.ringRenderRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: ringRenderRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    }
    for (auto& devicePrivacy : audioRouterCenter_.toneRenderRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: toneRenderRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    }
    for (auto& devicePrivacy : audioRouterCenter_.recordCaptureRouters_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: recordCaptureRouters_: devicePrivacy:%{public}s, ",
            devicePrivacy->name_.c_str());
    } 
    return true;
}

void AudioStrategyRouterParser::ParserStrategyInfo(xmlNode *node)
{
    xmlNode *strategyNode = node;

    char *pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
    if (strcmp(pValue, "MEDIA_RENDER") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: MEDIA_RENDER:%{public}s", pValue);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserMediaRenderRouters(pValue);
        xmlFree(pValue);
    } else if (strcmp(pValue, "CALL_RENDER") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: CALL_RENDER:%{public}s", pValue);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserCallRenderRouters(pValue);
        xmlFree(pValue);
    } else if (strcmp(pValue, "RING_RENDER") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: RING_RENDER:%{public}s", pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserRingRenderRouters(pValue);
        xmlFree(pValue);
    } else if (strcmp(pValue, "TONE_RENDER") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: TONE_RENDER:%{public}s", pValue);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserToneRenderRouters(pValue);
        xmlFree(pValue);
    } else if (strcmp(pValue, "RECORD_CAPTURE") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: RECORD_CAPTURE:%{public}s", pValue);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserRecordCaptureRouters(pValue);
    } else if (strcmp(pValue, "CALL_CAPTURE") == 0) {
        AUDIO_INFO_LOG("AudioStrategyRouterParser: CALL_CAPTURE:%{public}s", pValue);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
        AUDIO_INFO_LOG("AudioStrategyRouterParser: routers:%{public}s", pValue);
        ParserCallCaptureRouters(pValue);
    }
}

std::string AudioStrategyRouterParser::ExtractPropertyValue(const std::string &propName, xmlNode *node)
{
    std::string propValue = "";
    xmlChar *tempValue = nullptr;

    if (xmlHasProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()))) {
        tempValue = xmlGetProp(node, reinterpret_cast<const xmlChar*>(propName.c_str()));
    }

    if (tempValue != nullptr) {
        propValue = reinterpret_cast<const char*>(tempValue);
        xmlFree(tempValue);
    }

    return propValue;
}

std::vector<std::string> AudioStrategyRouterParser::split(const std::string &line, const std::string &sep)
{
    std::vector<std::string> buf;
    int temp = 0;
    std::string::size_type pos = 0;
    while (true) {
        pos = line.find(sep, temp);
        if (pos == std::string::npos) {
            break;
        }
        buf.push_back(line.substr(temp, pos-temp));
        temp = pos + sep.length();
    }
    buf.push_back(line.substr(temp, line.length()));
    return buf;
}

void AudioStrategyRouterParser::ParserMediaRenderRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.mediaRenderRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserMediaRenderRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.mediaRenderRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserMediaRenderRouters PrivacyPriorityRouter");
        } else if (*i == "PublicPriorityRouter") {
            audioRouterCenter_.mediaRenderRouters_.push_back(std::make_unique<PublicPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserMediaRenderRouters PublicPriorityRouter");
        } else if (*i == "StreamFilterRouter") {
            audioRouterCenter_.mediaRenderRouters_.push_back(std::make_unique<StreamFilterRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserMediaRenderRouters StreamFilterRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.mediaRenderRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserMediaRenderRouters DefaultRouter");
        }
    }
}

void AudioStrategyRouterParser::ParserRecordCaptureRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.recordCaptureRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRecordCaptureRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.recordCaptureRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRecordCaptureRouters PrivacyPriorityRouter");
        } else if (*i == "PublicPriorityRouter") {
            audioRouterCenter_.recordCaptureRouters_.push_back(std::make_unique<PublicPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRecordCaptureRouters PublicPriorityRouter");
        } else if (*i == "StreamFilterRouter") {
            audioRouterCenter_.recordCaptureRouters_.push_back(std::make_unique<StreamFilterRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRecordCaptureRouters StreamFilterRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.recordCaptureRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRecordCaptureRouters DefaultRouter");
        }
    }
}

void AudioStrategyRouterParser::ParserCallRenderRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.callRenderRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallRenderRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.callRenderRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallRenderRouters PrivacyPriorityRouter");
        } else if (*i == "CockpitPhoneRouter") {
            audioRouterCenter_.callRenderRouters_.push_back(std::make_unique<CockpitPhoneRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallRenderRouters CockpitPhoneRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.callRenderRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallRenderRouters DefaultRouter");
        }
    }
}

void AudioStrategyRouterParser::ParserCallCaptureRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.callCaptureRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallCaptureRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.callCaptureRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallCaptureRouters PrivacyPriorityRouter");
        } else if (*i == "CockpitPhoneRouter") {
            audioRouterCenter_.callCaptureRouters_.push_back(std::make_unique<CockpitPhoneRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallCaptureRouters CockpitPhoneRouter");
        } else if (*i == "PairDeviceRouter") {
            audioRouterCenter_.callCaptureRouters_.push_back(std::make_unique<PairDeviceRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallCaptureRouters PairDeviceRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.callCaptureRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserCallCaptureRouters DefaultRouter");
        }
    }
}

void AudioStrategyRouterParser::ParserRingRenderRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.ringRenderRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRingRenderRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.ringRenderRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRingRenderRouters PrivacyPriorityRouter");
        } else if (*i == "PublicPriorityRouter") {
            audioRouterCenter_.ringRenderRouters_.push_back(std::make_unique<PublicPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRingRenderRouters PublicPriorityRouter");
        } else if (*i == "StreamFilterRouter") {
            audioRouterCenter_.ringRenderRouters_.push_back(std::make_unique<StreamFilterRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRingRenderRouters StreamFilterRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.ringRenderRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserRingRenderRouters DefaultRouter");
        }
    }
}

void AudioStrategyRouterParser::ParserToneRenderRouters(const std::string &routers)
{
    std::vector<std::string> buf = split(routers, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "UserSelectRouter") {
            audioRouterCenter_.toneRenderRouters_.push_back(std::make_unique<UserSelectRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserToneRenderRouters UserSelectRouter");
        } else if (*i == "PrivacyPriorityRouter") {
            audioRouterCenter_.toneRenderRouters_.push_back(std::make_unique<PrivacyPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserToneRenderRouters PrivacyPriorityRouter");
        } else if (*i == "PublicPriorityRouter") {
            audioRouterCenter_.toneRenderRouters_.push_back(std::make_unique<PublicPriorityRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserToneRenderRouters PublicPriorityRouter");
        } else if (*i == "StreamFilterRouter") {
            audioRouterCenter_.toneRenderRouters_.push_back(std::make_unique<StreamFilterRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserToneRenderRouters StreamFilterRouter");
        } else if (*i == "DefaultRouter") {
            audioRouterCenter_.toneRenderRouters_.push_back(std::make_unique<DefaultRouter>());
            AUDIO_INFO_LOG("AudioStrategyRouterParser: ParserToneRenderRouters DefaultRouter");
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS