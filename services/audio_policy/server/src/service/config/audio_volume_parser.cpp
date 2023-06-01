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
#include "audio_volume_parser.h"
#include "config_policy_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioVolumeParser::AudioVolumeParser()
{
    AUDIO_INFO_LOG("AudioVolumeParser ctor");
    audioStreamMap_ = {
        {"VOICE_CALL", STREAM_VOICE_CALL},
        {"MUSIC", STREAM_MUSIC},
        {"RING", STREAM_RING},
        {"VOICE_ASSISTANT", STREAM_VOICE_ASSISTANT},
        {"ALARM", STREAM_ALARM},
        {"ACCESSIBILITY", STREAM_ACCESSIBILITY},
        {"ULTRASONIC", STREAM_ULTRASONIC},
    };

    audioDeviceMap_ = {
        {"earpiece", DEVICE_TYPE_EARPIECE},
        {"speaker", DEVICE_TYPE_SPEAKER},
        {"headset", DEVICE_TYPE_WIRED_HEADSET},
    };
}

AudioVolumeParser::~AudioVolumeParser()
{
    AUDIO_INFO_LOG("AudioVolumeParser dtor");
}

int32_t AudioVolumeParser::ParseVolumeConfig(const char *path, StreamVolumeInfoMap &streamVolumeInfoMap)
{
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    if ((doc = xmlReadFile(path, nullptr, 0)) == nullptr) {
        AUDIO_ERR_LOG("could not parse file %{public}s", path);
        return ERROR;
    }
    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, ERROR, "root element is null");

    if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_volume_config"))) {
        if ((currNode->children) && (currNode->children->next)) {
            currNode = currNode->children->next;
            ParseStreamInfos(currNode, streamVolumeInfoMap);
        } else {
            AUDIO_ERR_LOG("empty volume config in : %s", path);
            return SUCCESS;
        }
    } else {
        AUDIO_ERR_LOG("Missing tag - audio_volume_config in : %s", path);
        return ERROR;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return SUCCESS;
}

int32_t AudioVolumeParser::LoadConfig(StreamVolumeInfoMap &streamVolumeInfoMap)
{
    AUDIO_INFO_LOG("Load Volume Config xml");
    int ret = ERROR;
    CfgFiles *cfgFiles = GetCfgFiles(AUDIO_VOLUME_CONFIG_FILE);
    if (cfgFiles == nullptr) {
        AUDIO_ERR_LOG("Not found audio_volume_config.xml!");
        return ERROR;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            AUDIO_INFO_LOG("volume config file path:%{public}s", cfgFiles->paths[i]);
            ret = ParseVolumeConfig(cfgFiles->paths[i], streamVolumeInfoMap);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
    return ret;
}

void AudioVolumeParser::ParseStreamInfos(xmlNode *node, StreamVolumeInfoMap &streamVolumeInfoMap)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamInfos");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE && (!xmlStrcmp(currNode->name,
            reinterpret_cast<const xmlChar*>("volume_type")))) {
                std::shared_ptr<StreamVolumeInfo> streamVolInfo = std::make_shared<StreamVolumeInfo>();
                ParseStreamVolumeInfoAttr(currNode, streamVolInfo);
                ParseDeviceVolumeInfos(currNode->children, streamVolInfo);
                AUDIO_DEBUG_LOG("Parse streamType:%{public}d ", streamVolInfo->streamType);
                streamVolumeInfoMap[streamVolInfo->streamType] = streamVolInfo;
        }
        currNode = currNode->next;
    }
}

void AudioVolumeParser::ParseStreamVolumeInfoAttr(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamVolumeInfoAttr");
    char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
    streamVolInfo->streamType = audioStreamMap_[pValue];
    AUDIO_DEBUG_LOG("stream type: %{public}s; currNode->name %{public}s;", pValue, currNode->name);
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("minidx"))));
    streamVolInfo->minIndex = atoi(pValue);
    AUDIO_DEBUG_LOG("minidx: %{public}d", atoi(pValue));
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("maxidx"))));
    streamVolInfo->maxIndex = atoi(pValue);
    AUDIO_DEBUG_LOG("minidx: %{public}d", atoi(pValue));
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("defaultidx"))));
    streamVolInfo->defaultIndex = atoi(pValue);
    AUDIO_DEBUG_LOG("defaultidx: %{public}d", atoi(pValue));
    xmlFree(pValue);
}

void AudioVolumeParser::ParseDeviceVolumeInfos(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseDeviceVolumeInfos");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE && (!xmlStrcmp(currNode->name,
            reinterpret_cast<const xmlChar*>("volumecurve")))) {
                char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("deviceClass"))));
                std::shared_ptr<DeviceVolumeInfo> deviceVolInfo = std::make_shared<DeviceVolumeInfo>();
                AUDIO_DEBUG_LOG("deviceClass: %{public}s; currNode->name %{public}s;", pValue, currNode->name);
                deviceVolInfo->deviceName = audioDeviceMap_[pValue];
                AUDIO_DEBUG_LOG("deviceVolInfo->deviceName %{public}d;", deviceVolInfo->deviceName);
                xmlFree(pValue);
                ParseVolumePoints(currNode->children, deviceVolInfo);
                streamVolInfo->deviceVolumeInfos[deviceVolInfo->deviceName] = deviceVolInfo;
        }
        currNode = currNode->next;
    }
}

void AudioVolumeParser::ParseVolumePoints(xmlNode *node, std::shared_ptr<DeviceVolumeInfo> &deviceVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseVolumePoints");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE && (!xmlStrcmp(currNode->name,
            reinterpret_cast<const xmlChar*>("point")))) {
                struct VolumePoint volumePoint;
                char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("idx"))));
                volumePoint.index = atoi(pValue);
                AUDIO_DEBUG_LOG("idx: %{public}d", atoi(pValue));
                xmlFree(pValue);
                pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                    reinterpret_cast<xmlChar*>(const_cast<char*>("decibel"))));
                volumePoint.dbValue = atoi(pValue);
                AUDIO_DEBUG_LOG("decibel: %{public}d", atoi(pValue));
                xmlFree(pValue);
                deviceVolInfo->volumePoints.push_back(volumePoint);
        }
        currNode = currNode->next;
    }
}
} // namespace AudioStandard
} // namespace OHOS
