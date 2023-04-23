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
#include "audio_effect_config_parser.h"
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace OHOS {
namespace AudioStandard {
static constexpr char AUDIO_EFFECT_CONFIG_FILE[] = "system/etc/audio/audio_effect_config.xml";
static const std::string EFFECT_CONFIG_NAME[5] = {"libraries", "effects", "effectChains", "preprocess", "postprocess"};
static constexpr int32_t FILE_CONTENT_ERROR = -2;
static constexpr int32_t FILE_PARSE_ERROR = -3;
AudioEffectConfigParser::AudioEffectConfigParser()
{
    AUDIO_INFO_LOG("AudioEffectConfigParser created");
}

AudioEffectConfigParser::~AudioEffectConfigParser()
{
}

static int32_t LoadConfigCheck(xmlDoc* doc, xmlNode* currNode)
{
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_effects_conf"))) {
        AUDIO_ERR_LOG("Missing tag - audio_effects_conf: %{public}s", AUDIO_EFFECT_CONFIG_FILE);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FILE_CONTENT_ERROR;
    }
    
    // 获取下一个节点
    if (currNode->xmlChildrenNode) {
        return 0;
    } else {
        AUDIO_ERR_LOG("Missing node - audio_effects_conf: %s", AUDIO_EFFECT_CONFIG_FILE);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FILE_CONTENT_ERROR;
    }
}

static void LoadConfigVersion(OriginalEffectConfig &result, xmlNode* currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("version")))) {
        AUDIO_ERR_LOG("missing information: audio_effects_conf node has no version attribute");
        return;
    }

    float pVersion = atof(reinterpret_cast<char*>
    (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("version")))));
    result.version = pVersion;
}

static void LoadLibrary(OriginalEffectConfig &result, xmlNode* secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countLibrary = 0;
    while (currNode != nullptr) {
        if (countLibrary >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of library nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("library"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name")))) {
                AUDIO_ERR_LOG("missing information: library has no name attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("path")))) {
                AUDIO_ERR_LOG("missing information: library has no path attribute");
            } else {
                std::string pLibName = reinterpret_cast<char*>
                                      (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                std::string pLibPath = reinterpret_cast<char*>
                                      (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("path"))));
                Library tmp = {pLibName, pLibPath};
                result.libraries.push_back(tmp);
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be library", currNode->name);
        }
        countLibrary++;
        currNode = currNode->next;
    }
    if (countLibrary == 0) {
        AUDIO_ERR_LOG("missing information: libraries have no child library");
    }
}

static void LoadEffectConfigLibraries(OriginalEffectConfig &result, const xmlNode* currNode,
                                      int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[0] >= COUNT_FIRST_NODE_UPPER_LIMIT) {     // 0: index for libraries
        if (countFirstNode[0] == COUNT_FIRST_NODE_UPPER_LIMIT) { // 0: index for libraries
            countFirstNode[0]++;                      // 0: index for libraries
            AUDIO_ERR_LOG("the number of libraries nodes exceeds limit: %{public}d", COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadLibrary(result, currNode->xmlChildrenNode);
        countFirstNode[0]++;                          // 0: index for libraries
    } else {
        AUDIO_ERR_LOG("missing information: libraries have no child library");
        countFirstNode[0]++;                          // 0: index for libraries
    }
}

static void LoadEffect(OriginalEffectConfig &result, xmlNode* secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countEffect = 0;
    while (currNode != nullptr) {
        if (countEffect >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of effect nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effect"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name")))) {
                AUDIO_ERR_LOG("missing information: effect has no name attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("library")))) {
                AUDIO_ERR_LOG("missing information: effect has no library attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effect_id")))) {
                AUDIO_ERR_LOG("missing information: effect has no effect_id attribute");
            } else {
                std::string pEffectName = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                std::string pEffectLib = reinterpret_cast<char*>
                             (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("library"))));
                std::string pEffectID = reinterpret_cast<char*>
                            (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effect_id"))));
                Effect tmp = {pEffectName, pEffectLib, pEffectID};
                result.effects.push_back(tmp);
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be effect", currNode->name);
        }
        countEffect++;
        currNode = currNode->next;
    }
    if (countEffect == 0) {
        AUDIO_ERR_LOG("missing information: effects have no child effect");
    }
}

static void LoadEffectConfigEffects(OriginalEffectConfig &result, const xmlNode* currNode,
                                    int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[1] >= COUNT_FIRST_NODE_UPPER_LIMIT) {     // 1: index for effects
        if (countFirstNode[1] == COUNT_FIRST_NODE_UPPER_LIMIT) { // 1: index for effects
            countFirstNode[1]++;                      // 1: index for effects
            AUDIO_ERR_LOG("the number of effects nodes exceeds limit: %{public}d", COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadEffect(result, currNode->xmlChildrenNode);
        countFirstNode[1]++;                          // 1: index for effects
    } else {
        AUDIO_ERR_LOG("missing information: effects have no child effect");
        countFirstNode[1]++;                          // 1: index for effects
    }
}

static void LoadApply(OriginalEffectConfig &result, const xmlNode* thirdNode, const int32_t segInx)
{
    if (!thirdNode->xmlChildrenNode) {
        AUDIO_ERR_LOG("missing information: effectChain has no child apply");
        return;
    }
    int32_t countApply = 0;
    xmlNode *currNode = thirdNode->xmlChildrenNode;
    while (currNode != nullptr) {
        if (countApply >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of apply nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("apply"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effect")))) {
                AUDIO_ERR_LOG("missing information: apply has no effect attribute");
            } else {
                std::string ppValue = reinterpret_cast<char*>
                                     (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effect"))));
                result.effectChains[segInx].apply.push_back(ppValue);
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be apply", currNode->name);
        }
        countApply++;
        currNode = currNode->next;
    }
    if (countApply == 0) {
        AUDIO_ERR_LOG("missing information: effectChain has no child apply");
    }
}

static void LoadEffectChain(OriginalEffectConfig &result, xmlNode* secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countEffectChain = 0;
    int32_t segInx = 0;
    std::vector<std::string> apply;
    while (currNode != nullptr) {
        if (countEffectChain >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of effectChain nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effectChain"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name")))) {
                AUDIO_ERR_LOG("missing information: effectChain has no name attribute");
            } else {
                std::string peffectChainName = reinterpret_cast<char*>
                                   (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("name"))));
                effectChain tmp = {peffectChainName, apply};
                result.effectChains.push_back(tmp);
                LoadApply(result, currNode, segInx);
                segInx++;
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be effectChain", currNode->name);
        }
        countEffectChain++;
        currNode = currNode->next;
    }
    if (countEffectChain == 0) {
        AUDIO_ERR_LOG("missing information: effectChains have no child effectChain");
    }
}

static void LoadEffectConfigEffectChains(OriginalEffectConfig &result, const xmlNode* currNode,
                                         int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[2] >= COUNT_FIRST_NODE_UPPER_LIMIT) {     // 2: index for effectChains
        if (countFirstNode[2] == COUNT_FIRST_NODE_UPPER_LIMIT) { // 2: index for effectChains
            countFirstNode[2]++;                      // 2: index for effectChains
            AUDIO_ERR_LOG("the number of effectChains nodes exceeds limit: %{public}d", COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadEffectChain(result, currNode->xmlChildrenNode);
        countFirstNode[2]++;                          // 2: index for effectChains
    } else {
        AUDIO_ERR_LOG("missing information: effectChains have no child effectChain");
        countFirstNode[2]++;                          // 2: index for effectChains
    }
}

static void LoadPreDevice(OriginalEffectConfig &result, const xmlNode* fourthNode,
                          const int32_t modeNum, const int32_t streamNum)
{
    if (!fourthNode->xmlChildrenNode) {
        AUDIO_ERR_LOG("missing information: streamAE_mode has no child devicePort");
        return;
    }
    int32_t countDevice = 0;
    xmlNode *currNode = fourthNode->xmlChildrenNode;
    while (currNode != nullptr) {
        if (countDevice >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of devicePort nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("devicePort"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("type")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no type attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("address")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no address attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effectChain")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no effectChain attribute");
            } else {
                std::string pDevType = reinterpret_cast<char*>
                           (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
                std::string pDevAddress = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("address"))));
                std::string pChain = reinterpret_cast<char*>
                         (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effectChain"))));
                Device tmpdev = {pDevType, pDevAddress, pChain};
                result.preprocess[streamNum].device[modeNum].push_back(tmpdev);
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be devicePort", currNode->name);
        }
        countDevice++;
        currNode = currNode->next;
    }
    if (countDevice == 0) {
        AUDIO_ERR_LOG("missing information: streamAE_mode has no child devicePort");
    }
}

static void LoadPreMode(OriginalEffectConfig &result, const xmlNode* thirdNode, const int32_t streamNum)
{
    if (!thirdNode->xmlChildrenNode) {
        AUDIO_ERR_LOG("missing information: stream has no child streamAE_mode");
        return;
    }
    int32_t countMode = 0;
    int32_t modeNum = 0;
    xmlNode *currNode = thirdNode->xmlChildrenNode;
    while (currNode != nullptr) {
        if (countMode >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of streamAE_mode nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("streamAE_mode"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("mode")))) {
                AUDIO_ERR_LOG("missing information: streamAE_mode has no mode attribute");
            } else {
                std::string pStreamAEMode = reinterpret_cast<char*>
                                (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("mode"))));
                result.preprocess[streamNum].mode.push_back(pStreamAEMode);
                result.preprocess[streamNum].device.push_back({});
                LoadPreDevice(result, currNode, modeNum, streamNum);
                modeNum++;
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be streamAE_mode", currNode->name);
        }
        countMode++;
        currNode = currNode->next;
    }
    if (countMode == 0) {
        AUDIO_ERR_LOG("missing information: stream has no child streamAE_mode");
    }
}

static void LoadPreProcess(OriginalEffectConfig &result, xmlNode* secondNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    Preprocess tmp = {stream, mode, device};
    xmlNode *currNode = secondNode;
    int32_t countPreprocess = 0;
    int32_t streamNum = 0;
    while (currNode != nullptr) {
        if (countPreprocess >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of stream nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("stream"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("scene")))) {
                AUDIO_ERR_LOG("missing information: stream has no scene attribute");
            } else {
                std::string pStreamType = reinterpret_cast<char*>
                                         (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("scene"))));
                tmp.stream = pStreamType;
                result.preprocess.push_back(tmp);
                LoadPreMode(result, currNode, streamNum);
                streamNum++;
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be stream", currNode->name);
        }
        countPreprocess++;
        currNode = currNode->next;
    }
    if (countPreprocess == 0) {
        AUDIO_ERR_LOG("missing information: preprocess has no child stream");
    }
}

static void LoadEffectConfigPreProcess(OriginalEffectConfig &result, const xmlNode* currNode,
                                       int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[3] >= COUNT_FIRST_NODE_UPPER_LIMIT) {     // 3: index for preprocess
        if (countFirstNode[3] == COUNT_FIRST_NODE_UPPER_LIMIT) { // 3: index for preprocess
            countFirstNode[3]++;                      // 3: index for preprocess
            AUDIO_ERR_LOG("the number of preprocess nodes exceeds limit: %{public}d", COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadPreProcess(result, currNode->xmlChildrenNode);
        countFirstNode[3]++;                          // 3: index for preprocess
    } else {
        AUDIO_ERR_LOG("missing information: preprocess has no child stream");
        countFirstNode[3]++;                          // 3: index for preprocess
    }
}

static void LoadPostDevice(OriginalEffectConfig &result, const xmlNode* fourthNode,
                           const int32_t modeNum, const int32_t streamNum)
{
    if (!fourthNode->xmlChildrenNode) {
        AUDIO_ERR_LOG("missing information: streamAE_mode has no child devicePort");
        return;
    }
    int32_t countDevice = 0;
    xmlNode *currNode = fourthNode->xmlChildrenNode;
    while (currNode != nullptr) {
        if (countDevice >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of devicePort nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("devicePort"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("type")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no type attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("address")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no address attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effectChain")))) {
                AUDIO_ERR_LOG("missing information: devicePort has no effectChain attribute");
            } else {
                std::string pDevType = reinterpret_cast<char*>
                           (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
                std::string pDevAddress = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("address"))));
                std::string pChain = reinterpret_cast<char*>
                         (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("effectChain"))));
                Device tmpdev = {pDevType, pDevAddress, pChain};
                result.postprocess[streamNum].device[modeNum].push_back(tmpdev);
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be devicePort", currNode->name);
        }
        countDevice++;
        currNode = currNode->next;
    }
    if (countDevice == 0) {
        AUDIO_ERR_LOG("missing information: streamAE_mode has no child devicePort");
    }
}

static void LoadPostMode(OriginalEffectConfig &result, const xmlNode* thirdNode, const int32_t streamNum)
{
    if (!thirdNode->xmlChildrenNode) {
        AUDIO_ERR_LOG("missing information: stream has no child streamAE_mode");
        return;
    }
    int32_t countMode = 0;
    int32_t modeNum = 0;
    xmlNode *currNode = thirdNode->xmlChildrenNode;
    while (currNode != nullptr) {
        if (countMode >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of streamAE_mode nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("streamAE_mode"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("mode")))) {
                AUDIO_ERR_LOG("missing information: streamAE_mode has no mode attribute");
            } else {
                std::string pStreamAEMode = reinterpret_cast<char*>
                                (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("mode"))));
                result.postprocess[streamNum].mode.push_back(pStreamAEMode);
                result.postprocess[streamNum].device.push_back({});
                LoadPostDevice(result, currNode, modeNum, streamNum);
                modeNum++;
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be streamAE_mode", currNode->name);
        }
        countMode++;
        currNode = currNode->next;
    }
    if (countMode == 0) {
        AUDIO_ERR_LOG("missing information: stream has no child streamAE_mode");
    }
}

static void LoadPostProcess(OriginalEffectConfig &result, xmlNode* secondNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    Postprocess tmp = {stream, mode, device};
    xmlNode *currNode = secondNode;
    int32_t countPostprocess = 0;
    int32_t streamNum = 0;
    while (currNode != nullptr) {
        if (countPostprocess >= COUNT_UPPER_LIMIT) {
            AUDIO_ERR_LOG("the number of stream nodes exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
            return;
        }
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("stream"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("scene")))) {
                AUDIO_ERR_LOG("missing information: stream has no scene attribute");
            } else {
                std::string pStreamType = reinterpret_cast<char*>
                                         (xmlGetProp(currNode, reinterpret_cast<xmlChar*>(const_cast<char*>("scene"))));
                tmp.stream = pStreamType;
                result.postprocess.push_back(tmp);
                LoadPostMode(result, currNode, streamNum);
                streamNum++;
            }
        } else {
            AUDIO_ERR_LOG("wrong name: %{public}s, should be stream", currNode->name);
        }
        countPostprocess++;
        currNode = currNode->next;
    }
    if (countPostprocess == 0) {
        AUDIO_ERR_LOG("missing information: postprocess has no child stream");
    }
}

static void LoadEffectConfigPostProcess(OriginalEffectConfig &result, const xmlNode* currNode,
                                        int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[4] >= COUNT_FIRST_NODE_UPPER_LIMIT) {     // 4: index for postprocess
        if (countFirstNode[4] == COUNT_FIRST_NODE_UPPER_LIMIT) { // 4: index for postprocess
            countFirstNode[4]++;                      // 4: index for postprocess
            AUDIO_ERR_LOG("the number of postprocess nodes exceeds limit: %{public}d", COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadPostProcess(result, currNode->xmlChildrenNode);
        countFirstNode[4]++;                          // 4: index for postprocess
    } else {
        AUDIO_ERR_LOG("missing information: postprocess has no child stream");
        countFirstNode[4]++;                          // 4: index for postprocess
    }
}

static void LoadEffectConfigException(OriginalEffectConfig &result, const xmlNode* currNode,
                                      int32_t (&countFirstNode)[6]) // 6: size
{
    if (countFirstNode[5] >= COUNT_UPPER_LIMIT) {     // 5: index for exception
        if (countFirstNode[5] == COUNT_UPPER_LIMIT) { // 5: index for exception
            countFirstNode[5]++;                      // 5: index for exception
            AUDIO_ERR_LOG("the number of nodes with wrong name exceeds limit: %{public}d", COUNT_UPPER_LIMIT);
        }
    } else {
        AUDIO_ERR_LOG("wrong name: %{public}s", currNode->name);
        countFirstNode[5]++;                          // 5: index for exception
    }
}

int32_t AudioEffectConfigParser::LoadEffectConfig(OriginalEffectConfig &result)
{
    int32_t countFirstNode[6] = {0}; // 6 size
    int32_t i = 0;
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    AUDIO_INFO_LOG("AudioEffectParser::LoadConfig");
    if ((doc = xmlReadFile(AUDIO_EFFECT_CONFIG_FILE, nullptr, (1<<5) | (1<<6))) == nullptr) { // 5, 6: arguments
        AUDIO_ERR_LOG("error: could not parse file %{public}s", AUDIO_EFFECT_CONFIG_FILE);
        return FILE_PARSE_ERROR;
    }

    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;

    if (LoadConfigCheck(doc, currNode) == 0) {
        LoadConfigVersion(result, currNode);
        currNode = currNode->xmlChildrenNode;
    } else {
        return FILE_CONTENT_ERROR;
    }

    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }

        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("libraries"))) {
            LoadEffectConfigLibraries(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effects"))) {
            LoadEffectConfigEffects(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effectChains"))) {
            LoadEffectConfigEffectChains(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("preprocess"))) {
            LoadEffectConfigPreProcess(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("postprocess"))) {
            LoadEffectConfigPostProcess(result, currNode, countFirstNode);
        } else {
            LoadEffectConfigException(result, currNode, countFirstNode);
        }

        currNode = currNode->next;
    }
    for (i = 0; i < 5; i++) { // 5: the number of modules
        if (countFirstNode[i] == 0) {
            AUDIO_ERR_LOG("missing information: %{public}s", EFFECT_CONFIG_NAME[i].c_str());
        }
    }

    if (doc) {
    xmlFreeDoc(doc);
    xmlCleanupParser();
    }
    return 0;
}

} // namespace AudioStandard
} // namespace OHOS