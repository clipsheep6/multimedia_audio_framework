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

#ifndef AUDIO_USAGE_STRATEGY_PARSER_H
#define AUDIO_USAGE_STRATEGY_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "audio_log.h"
#include "audio_info.h"
#include "iport_observer.h"
#include "parser.h"
#include "audio_router_center.h"

namespace OHOS {
namespace AudioStandard {
static const std::string STREAMUSAGE = "streamUsage";
static const std::string SOURCETYPE = "sourceType";

class AudioUsageStrategyParser : public Parser {
public:
    static constexpr char DEVICE_CONFIG_FILE[] = "system/etc/audio/audio_usage_strategy.xml";

    bool LoadConfiguration() final;
    bool Parse() final;
    void Destroy() final;

    AudioUsageStrategyParser():audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter())
    {
         AUDIO_INFO_LOG("AudioUsageStrategyParser ctor");
    }

    virtual ~AudioUsageStrategyParser()
    {
        AUDIO_INFO_LOG("AudioUsageStrategyParser dtor");
        Destroy();
    }

private:
    bool ParseInternal(xmlNode *node);
    void ParserStreamUsageList(xmlNode *node);
    void ParserSourceTypeList(xmlNode *node);
    void ParserStreamUsageInfo(const std::string &streamUsage);
    void ParserSourceTypeInfo(const std::string &sourceType);
    std::vector<std::string> split(const std::string &line, const std::string &sep);
    AudioRouterCenter &audioRouterCenter_;

    xmlDoc *mDoc_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_USAGE_STRATEGY_PARSER_H
