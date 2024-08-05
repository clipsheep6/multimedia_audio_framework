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

#ifndef AUDIO_DEVICE_AFFINITY_PARSER_H
#define AUDIO_DEVICE_AFFINITY_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_policy_log.h"
#include "audio_info.h"
#include "iport_observer.h"
#include "parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class audioDeviceAffinityParser : public Parser {
public:
    static constexpr char AFFINITY_CONFIG_FILE[] = "system/etc/audio/audio_device_affinity_config.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    audioDeviceAffinityParser()
    {
        AUDIO_DEBUG_LOG("audioDeviceAffinityParser ctor");
    }

    virtual ~audioDeviceAffinityParser()
    {
        AUDIO_DEBUG_LOG("audioDeviceAffinityParser dtor");
        Destroy();
    }


private:
    bool ParseInternal(xmlNode *node);
    void ParserOutputDevicesList(xmlNode *node);
    void ParserInputDevicesList(xmlNode *node);
    void ParserAffinityGroup(const std::string &strategyName, const std::string &streamUsage);
    void ParserAffinityGroupDevice(const std::string &strategyName, const std::string &streamUsage);

    std::vector<std::string> split(const std::string &line, const std::string &sep);

    static std::map<std::string, DeviceType> deviceTypeMap_ = {
    {"DEVICE_TYPE_EARPIECE", DEVICE_TYPE_EARPIECE},
    {"DEVICE_TYPE_SPEAKER", DEVICE_TYPE_SPEAKER},
    {"DEVICE_TYPE_WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"DEVICE_TYPE_WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"DEVICE_TYPE_BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"DEVICE_TYPE_BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"DEVICE_TYPE_USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DEVICE_TYPE_USB_ARM_HEADSET", DEVICE_TYPE_USB_ARM_HEADSET},
    {"DEVICE_TYPE_DP", DEVICE_TYPE_DP},
    {"DEVICE_TYPE_REMOTE_CAST", DEVICE_TYPE_REMOTE_CAST},
    };

    xmlDoc *doc_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_AFFINITY_PARSER_H
