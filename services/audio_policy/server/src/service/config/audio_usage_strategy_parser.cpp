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
 
#include "audio_usage_strategy_parser.h"
#include <string>
#include <iostream>
#include <vector>

namespace OHOS {
namespace AudioStandard {
const string MEDIA_RENDER_ROUTERS = "MediaRenderRouters";
const string CALL_RENDER_ROUTERS = "CallRenderRouters";
const string RECORD_CAPTURE_ROUTERS = "RecordCaptureRouters";
const string CALL_CAPTURE_ROUTERS = "CallCaptureRouters";
const string RING_RENDER_ROUTERS = "RingRenderRouters";
const string TONE_RENDER_ROUTERS = "ToneRenderRouters";

bool AudioUsageStrategyParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(DEVICE_CONFIG_FILE, nullptr, 0);
    if (mDoc_ == nullptr) {
        AUDIO_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    return true;
}

bool AudioUsageStrategyParser::Parse()
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

void AudioUsageStrategyParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool AudioUsageStrategyParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    for (; currNode; currNode = currNode->next) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: node:%{public}s %{public}d %{public}d", currNode->name, currNode->type, XML_ELEMENT_NODE);
        if (XML_ELEMENT_NODE == currNode->type) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("adapter"))) {
                char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                    AUDIO_INFO_LOG("AudioUsageStrategyParser: node:%{public}s", pValue);
                if (strcmp(pValue, "streamUsage") == 0) {
                    ParserStreamUsageList(currNode->xmlChildrenNode);
                } else if (strcmp(pValue, "sourceType") == 0) {
                    ParserSourceTypeList(currNode->xmlChildrenNode);
                }
            } else {
                ParseInternal((currNode->xmlChildrenNode));
            }
        }
    }
    for (auto& devicePrivacy : audioRouterCenter_.renderConfigMap_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: renderConfigMap_: streamUsage:%{public}d, ",
            devicePrivacy.first);
    }
    for (auto& devicePrivacy : audioRouterCenter_.capturerConfigMap_) {
        AUDIO_INFO_LOG("AudioUsageStrategyParser: capturerConfigMap_: sourceType:%{public}d, ",
            devicePrivacy.first);
    } 
    return true;
}

void AudioUsageStrategyParser::ParserStreamUsageList(xmlNode *node)
{
    xmlNode *strategyNode = node;
    while (strategyNode != nullptr) {
        if (strategyNode->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(strategyNode->name, reinterpret_cast<const xmlChar*>("strategy"))) {
                char *pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                if (strategyNode != nullptr) {
                    AUDIO_ERR_LOG("AudioUsageStrategyParser: No name provided for the adapter %{public}s", node->name);
                    return;
                } else {
                    pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("streamUsage"))));
                    AUDIO_INFO_LOG("AudioStrategyRouterParser: usage:%{public}s", pValue);
                    ParserStreamUsageInfo(pValue);
                    xmlFree(pValue);
                }
            }
        }
        strategyNode = strategyNode->next;
    }
}

void AudioUsageStrategyParser::ParserSourceTypeList(xmlNode *node)
{
    xmlNode *strategyNode = node;
    while (strategyNode != nullptr) {
        if (strategyNode->type == XML_ELEMENT_NODE) {
            char *pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            if (strategyNode != nullptr) {
                AUDIO_ERR_LOG("AudioUsageStrategyParser: No name provided for the adapter %{public}s", node->name);
                return;
            } else {
               pValue = reinterpret_cast<char*>(xmlGetProp(strategyNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("sourceType"))));
                AUDIO_INFO_LOG("AudioStrategyRouterParser:sourceType:%{public}s", pValue);
                ParserSourceTypeInfo(pValue);
                xmlFree(pValue);
            }
        }
        strategyNode = strategyNode->next;
    }
}

std::vector<std::string> AudioUsageStrategyParser::split(const std::string &line, const std::string &sep)
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

void AudioUsageStrategyParser::ParserStreamUsageInfo(const std::string &streamUsage)
{
    std::vector<std::string> buf = split(streamUsage, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "STREAM_USAGE_MEDIA") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_MEDIA] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: UserSelectRouter");
        } else if (*i == "STREAM_USAGE_MUSIC") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_MUSIC] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_VOICE_COMMUNICATION");
        } else if (*i == "STREAM_USAGE_VOICE_COMMUNICATION") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION] = CALL_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_VOICE_COMMUNICATION");
        } else if (*i == "STREAM_USAGE_VOICE_ASSISTANT") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_VOICE_ASSISTANT] = CALL_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_VOICE_ASSISTANT");
        } else if (*i == "STREAM_USAGE_ALARM") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_ALARM] = RING_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_ALARM");
        } else if (*i == "STREAM_USAGE_VOICE_MESSAGE") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_VOICE_MESSAGE] = CALL_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_VOICE_MESSAGE");
        } else if (*i == "STREAM_USAGE_NOTIFICATION_RINGTONE") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_NOTIFICATION_RINGTONE");
        } else if (*i == "STREAM_USAGE_RINGTONE") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_RINGTONE] = RING_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_RINGTONE");
        } else if (*i == "STREAM_USAGE_NOTIFICATION") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_NOTIFICATION] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_NOTIFICATION");
        } else if (*i == "STREAM_USAGE_ACCESSIBILITY") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_ACCESSIBILITY] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_ACCESSIBILITY");
        } else if (*i == "STREAM_USAGE_SYSTEM") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_SYSTEM] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_SYSTEM");
        } else if (*i == "STREAM_USAGE_MOVIE") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_MOVIE] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_MOVIE");
        } else if (*i == "STREAM_USAGE_GAME") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_GAME] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_GAME");
        } else if (*i == "STREAM_USAGE_AUDIOBOOK") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_AUDIOBOOK] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_AUDIOBOOK");
        } else if (*i == "STREAM_USAGE_NAVIGATION") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_NAVIGATION] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_NAVIGATION");
        } else if (*i == "STREAM_USAGE_DTMF") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_DTMF] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_DTMF");
        } else if (*i == "STREAM_USAGE_ENFORCED_TONE") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_ENFORCED_TONE] = TONE_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_ENFORCED_TONE");
        } else if (*i == "STREAM_USAGE_ULTRASONIC") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_ULTRASONIC] = MEDIA_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_ULTRASONIC");
        } else if (*i == "STREAM_USAGE_RANGING") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_RANGING] = RING_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_RANGING");
        } else if (*i == "STREAM_USAGE_VOICE_MODEM_COMMUNICATION") {
            audioRouterCenter_.renderConfigMap_[StreamUsage::STREAM_USAGE_VOICE_MODEM_COMMUNICATION] = CALL_RENDER_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: STREAM_USAGE_VOICE_MODEM_COMMUNICATION");
        }
    }
}

void AudioUsageStrategyParser::ParserSourceTypeInfo(const std::string &sourceType)
{
    std::vector<std::string> buf = split(sourceType, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "SOURCE_TYPE_MIC") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_MIC] = RECORD_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_MIC");
        } else if (*i == "SOURCE_TYPE_VOICE_RECOGNITION") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_VOICE_RECOGNITION] = RECORD_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_VOICE_RECOGNITION");
        } else if (*i == "SOURCE_TYPE_PLAYBACK_CAPTURE") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_PLAYBACK_CAPTURE] = RECORD_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_PLAYBACK_CAPTURE");
        } else if (*i == "SOURCE_TYPE_WAKEUP") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_WAKEUP] = RECORD_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_WAKEUP");
        } else if (*i == "SOURCE_TYPE_VOICE_COMMUNICATION") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_VOICE_COMMUNICATION] = CALL_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_VOICE_COMMUNICATION");
        } else if (*i == "SOURCE_TYPE_VOICE_MODEM_COMMUNICATION") {
            audioRouterCenter_.capturerConfigMap_[SOURCE_TYPE_VOICE_MODEM_COMMUNICATION] = RECORD_CAPTURE_ROUTERS;
            AUDIO_INFO_LOG("AudioUsageStrategyParser: SOURCE_TYPE_VOICE_MODEM_COMMUNICATION");
        }
    }
}

} // namespace AudioStandard
} // namespace OHOS