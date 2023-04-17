/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
*/
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
#ifndef AUDIO_EFFECT_CONFIG_PARSER_H
#define AUDIO_EFFECT_CONFIG_PARSER_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
constexpr int COUNT_UPPER_LIMIT = 20;
constexpr int COUNT_FIRST_NODE_UPPER_LIMIT = 1;

typedef struct {
    std::string name;
    std::string path;
}Library;

typedef struct {
    std::string name;
    std::string libraryName;
    std::string effectId;
}Effect;

typedef struct {
    std::string name;
    std::vector<std::string> apply;
}effectChain;

typedef struct {
    std::string type;
    std::string address;
    std::string chain;
}Device;

typedef struct {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
}Preprocess;

typedef struct {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
}Postprocess;

typedef struct {
    float version;
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<effectChain> effectChains;
    std::vector<Preprocess> preprocess;
    std::vector<Postprocess> postprocess;
}OriginalEffectConfig;

class AudioEffectConfigParser {
public:
    explicit AudioEffectConfigParser();
    ~AudioEffectConfigParser();
    int32_t LoadEffectConfig(OriginalEffectConfig &result);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_EFFECT_CONFIG_PARSER_H