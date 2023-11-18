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
 
#include "audio_router_parser.h"
#include <string>
#include <iostream>
#include <vector>

namespace OHOS {
namespace AudioStandard {
bool AudioRouterParser::LoadConfiguration()
{
    mDoc_ = xmlReadFile(DEVICE_CONFIG_FILE, nullptr, 0);
    if (mDoc_ == nullptr) {
        AUDIO_ERR_LOG("xmlReadFile Failed");
        return false;
    }

    return true;
}

bool AudioRouterParser::Parse()
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

void AudioRouterParser::Destroy()
{
    if (mDoc_ != nullptr) {
        xmlFreeDoc(mDoc_);
    }
}

bool AudioRouterParser::ParseInternal(xmlNode *node)
{
    xmlNode *currNode = node;
    while (currNode != nullptr) {
        if ((currNode->type == XML_ELEMENT_NODE) &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("strategy")))) {
            char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            AUDIO_DEBUG_LOG("Found node strategy name: %{public}s", pValue);
            ParseAudioStrategyRouter(currNode);
            break;
        } else {
            currNode = currNode->next;
        }
    }
    return true;
}

void AudioRouterParser::AddRouters(std::vector<std::unique_ptr<RouterBase>> &routers, string &routeName)
{
    if (routeName == "UserSelectRouter") {
        routers.push_back(std::make_unique<UserSelectRouter>());
    } else if (routeName == "PrivacyPriorityRouter") {
        routers.push_back(std::make_unique<PrivacyPriorityRouter>());
    } else if (routeName == "PublicPriorityRouter") {
        routers.push_back(std::make_unique<PublicPriorityRouter>());
    } else if (routeName == "StreamFilterRouter") {
        routers.push_back(std::make_unique<StreamFilterRouter>());
    } else if (routeName == "DefaultRouter") {
        routers.push_back(std::make_unique<DefaultRouter>());
    }
}

void AudioRouterParser::ParseAudioStrategyRouter(xmlNode *node)
{
    xmlNode *deviceNode = node;

    while (deviceNode != nullptr) {
        if (deviceNode->type == XML_ELEMENT_NODE) {
            char *pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
            xmlFree(pValue);

            pValue = reinterpret_cast<char*>(xmlGetProp(deviceNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("routers"))));
            
            xmlFree(pValue);
            AUDIO_DEBUG_LOG("AudioRouterParser: name:%{public}s, type:%{public}d, role:%{public}d, Category:%{public}d,"
                "Usage:%{public}d", deviceInfo.deviceName.c_str(), deviceInfo.deviceType, deviceInfo.deviceRole,
                deviceInfo.deviceCategory, deviceInfo.deviceUsage);
        }
        deviceNode = deviceNode->next;
    }
}

#if 0
void AudioRouterParser::ParserDevicePrivacyInfoList(xmlNode *node, std::list<DevicePrivacyInfo> &deviceLists)
{
    xmlNode *currentNode = node;
    while (currentNode != nullptr) {
        if (currentNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currentNode->name, reinterpret_cast<const xmlChar*>("devices")))) {
            ParseDevicePrivacyInfo(currentNode->xmlChildrenNode, deviceLists);
        }
        currentNode = currentNode->next;
    }
}

AudioDevicePrivacyType AudioRouterParser::GetDevicePrivacyType(const std::string &devicePrivacyType)
{
    if (devicePrivacyType == PRIVACY_TYPE) {
        return AudioDevicePrivacyType::TYPE_PRIVACY;
    } else if (devicePrivacyType == PUBLIC_TYPE) {
        return AudioDevicePrivacyType::TYPE_PUBLIC;
    } else {
        return AudioDevicePrivacyType::TYPE_NEGATIVE;
    }
}

std::string AudioRouterParser::ExtractPropertyValue(const std::string &propName, xmlNode *node)
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

std::vector<std::string> split(const std::string &line, const std::string &sep)
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

void AudioRouterParser::ParseDeviceRole(const std::string &deviceRole, int32_t &deviceRoleFlag)
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

void AudioRouterParser::ParseDeviceCategory(const std::string &deviceCategory, int32_t &deviceCategoryFlag)
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

void AudioRouterParser::ParseDeviceUsage(const std::string &deviceUsage, int32_t &deviceUsageFlag)
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
#endif
} // namespace AudioStandard
} // namespace OHOS