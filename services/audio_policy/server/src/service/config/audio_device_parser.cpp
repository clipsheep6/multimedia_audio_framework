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

#include "audio_device_parser.h"
#include "string.h"
#include <iostream>
#include <vector>

namespace OHOS {
namespace AudioStandard {
bool AudioDeviceParser::LoadConfiguration()
{
    AUDIO_INFO_LOG("start LoadConfiguration");
    mDoc = xmlReadFile(DEVICE_CONFIG_FILE, nullptr, 0);
    if (mDoc == nullptr) {
        AUDIO_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    return true;
}

bool AudioDeviceParser::Parse()
{
    AUDIO_INFO_LOG("AudioDeviceParser::Parse");
    xmlNode *root = xmlDocGetRootElement(mDoc);
    if (root == nullptr) {
        AUDIO_ERR_LOG("xmlDocGetRootElement Failed");
        return false;
    }

    if (!ParseInternal(root)) {
        return false;
    }
    audioDeviceManager_->OnXmlParsingCompleted(devicePrivacyMaps_);
    return true;
}

void AudioDeviceParser::Destroy()
{
    if (mDoc != nullptr) {
        xmlFreeDoc(mDoc);
    }
}

bool AudioDeviceParser::ParseInternal(xmlNode *node)
{
    AUDIO_INFO_LOG("AudioDeviceParser::ParseInternal");
    xmlNode *currNode = node;
    for (; currNode; currNode = currNode->next) {
        if (XML_ELEMENT_NODE == currNode->type) {
            switch (GetDeviceNodeNameAsInt(currNode)) {
                case ADAPTER:
                    ParseAudioDevicePrivacyType(currNode, devicePrivacyType_);
                    AUDIO_INFO_LOG("AudioDeviceParser: ParseInternal: size:%{public}d, ", devicePrivacyMaps_.size());
                    for (auto devicePrivacy : devicePrivacyMaps_) {
                        AUDIO_INFO_LOG("_____AudioDeviceParser: ParseInternal: devicePrivacyType:%{public}d, ",
                            devicePrivacy.first);
                        for (auto deviceInfo : devicePrivacy.second) {
                            AUDIO_INFO_LOG("deviceInfo: deviceName:%{public}s,  deviceType:%{public}d,"
                                " deviceRole:%{public}d, deviceCategory:%{public}d, deviceUsage:%{public}d_____",
                                (deviceInfo.deviceName).c_str(), deviceInfo.deviceType, deviceInfo.deviceRole,
                                deviceInfo.deviceCategory, deviceInfo.deviceUsage);
                        }
                    } 
                    break;
                default:
                    ParseInternal((currNode->xmlChildrenNode));
                    break;
            }
        }
    }
    return true;
}

void AudioDeviceParser::ParseDevicePrivacyInfo(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists)
{
    AUDIO_INFO_LOG("AudioDeviceParser::ParseDevicePrivacyInfo");
    xmlNode *deviceNode = node;

    while (deviceNode != nullptr) {
        if (deviceNode->type == XML_ELEMENT_NODE) {
            DevicePrivacyInfo deviceInfo = {};
            char *pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            deviceInfo.deviceName = pValue;
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
            deviceInfo.deviceType = deviceTypeMap_[pValue];
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("role"))));
            int32_t intValue = 0;
            ParseDeviceRole(pValue, intValue);
            deviceInfo.deviceRole = static_cast<DeviceRole>(intValue);
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("Category"))));
            intValue = 0;
            ParseDeviceCategory(pValue, intValue);
            deviceInfo.deviceCategory = static_cast<DeviceCategory>(intValue);
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("usage"))));
            intValue = 0;
            ParseDeviceUsage(pValue, intValue);
            deviceInfo.deviceUsage = static_cast<DeviceUsage>(intValue);
            xmlFree(pValue);
            deviceLists.push_back(deviceInfo);
            AUDIO_INFO_LOG("AudioDeviceParser: name:%{public}s, type:%{public}d, role:%{public}d, Category:%{public}d,"
                "Usage:%{public}d", deviceInfo.deviceName.c_str(), deviceInfo.deviceType, deviceInfo.deviceRole,
                deviceInfo.deviceCategory, deviceInfo.deviceUsage);
        }
        deviceNode = deviceNode->next;
    }
}

void AudioDeviceParser::ParserDevicePrivacyInfoList(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists)
{
    AUDIO_INFO_LOG("AudioDeviceParser::ParserDevicePrivacyInfoList");
    xmlNode *currentNode = node;
    while (currentNode != nullptr) {
        if (currentNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currentNode->name, reinterpret_cast<const xmlChar*>("devices")))) {
            ParseDevicePrivacyInfo(currentNode->xmlChildrenNode, deviceLists);
        }
        currentNode = currentNode->next;
    }
}

void AudioDeviceParser::ParseAudioDevicePrivacyType(xmlNode *node, AudioDevicePrivacyType &deviceType)
{
    AUDIO_INFO_LOG("AudioDeviceParser::ParseAudioDevicePrivacyType");

    xmlNode *currNode = node;
    std::string adapterName = ExtractPropertyValue("name", currNode);

    while (currNode != nullptr) {
        //read deviceType
        if (currNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("adapter")))) {
            if (adapterName.empty()) {
                AUDIO_ERR_LOG("AudioDeviceParser: No name provided for the adapter %{public}s", node->name);
                return;
            } else {
                AUDIO_INFO_LOG("AudioDeviceParser: adapter name: %{public}s", adapterName.c_str());
                devicePrivacyType_ = GetDevicePrivacyType(adapterName);
                std::list<DevicePrivacyInfo> deviceLists = {};
                ParserDevicePrivacyInfoList(currNode->xmlChildrenNode, deviceLists);
                devicePrivacyMaps_[devicePrivacyType_] = deviceLists;
            }
        } else {
            return;
        }
        currNode = currNode->next;
    }
}

AudioDevicePrivacyType AudioDeviceParser::GetDevicePrivacyType(const std::string &devicePrivacyType)
{
    if (devicePrivacyType == PRIVACY_TYPE) {
        return AudioDevicePrivacyType::TYPE_PRIVACY;
    } else if (devicePrivacyType == PUBLIC_TYPE) {
        return AudioDevicePrivacyType::TYPE_PUBLIC;
    } else {
        return AudioDevicePrivacyType::TYPE_NEGATIVE;
    }
}

std::string AudioDeviceParser::ExtractPropertyValue(const std::string &propName, xmlNode *node)
{
    AUDIO_INFO_LOG("AudioDeviceParser::ExtractPropertyValue");
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

DeviceNodeName AudioDeviceParser::GetDeviceNodeNameAsInt(xmlNode *node)
{
    if (!xmlStrcmp(node->name, reinterpret_cast<const xmlChar*>("adapter"))) {
        return DeviceNodeName::ADAPTER;
    } else {
        return DeviceNodeName::UNKNOWN_NODE;
    }
}

std::vector<std::string> split(const std::string line, const std::string sep)
{
	std::vector<std::string> buf;
	int temp=0;
	std::string::size_type pos=0;
	while (true)
	{
		pos = line.find(sep, temp);
		if (pos == std::string::npos) break;
		buf.push_back(line.substr(temp, pos-temp));
		temp = pos + sep.length();
	}
	buf.push_back(line.substr(temp, line.length()));
	return buf;
}

void AudioDeviceParser::ParseDeviceRole(const std::string deviceRole, int32_t &deviceRoleFlag)
{
    std::vector<std::string> buf = split(deviceRole, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "output") {
            deviceRoleFlag |= DeviceRole::OUTPUT_DEVICE;
        } else if (*i == "input") {
            deviceRoleFlag |= DeviceRole::INPUT_DEVICE;
        }
    }
}

void AudioDeviceParser::ParseDeviceCategory(const std::string deviceCategory, int32_t &deviceCategoryFlag)
{
    std::vector<std::string> buf = split(deviceCategory, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "HEADPHONE") {
            deviceCategoryFlag |= DeviceCategory::BT_HEADPHONE;
        } else if (*i == "GLASSES") {
            deviceCategoryFlag |= DeviceCategory::BT_GLASSES;
        } else if (*i == "SOUNDBOX") {
            deviceCategoryFlag |= DeviceCategory::BT_SOUNDBOX;
        } else if (*i == "CAR") {
            deviceCategoryFlag |= DeviceCategory::BT_CAR;
        } else if (*i == "HEADPHONE_UNWEAR") {
            deviceCategoryFlag |= DeviceCategory::BT_UNWEAR_HEADPHONE;
        } else if (*i == "WATCH") {
            deviceCategoryFlag |= DeviceCategory::BT_WATCH;
        }
    }
}

void AudioDeviceParser::ParseDeviceUsage(const std::string deviceUsage, int32_t &deviceUsageFlag)
{
    std::vector<std::string> buf = split(deviceUsage, ",");
    typedef std::vector<std::string>::iterator itr;
    for (itr i = buf.begin(); i != buf.end(); i++) {
        if (*i == "media") {
            deviceUsageFlag |= DeviceUsage::MEDIA;
        } else if (*i == "voice") {
            deviceUsageFlag |= DeviceUsage::VOICE;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS