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
#include "audio_converter_parser.h"
#include <map>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace OHOS {
namespace AudioStandard {

static constexpr char AUDIO_CONVERTER_CONFIG_FILE[] = "system/etc/audio/audio_converter_config.xml";

static constexpr int32_t FILE_CONTENT_ERROR = -2;
static constexpr int32_t FILE_PARSE_ERROR = -3;

enum XML_ERROR
{
    XML_PARSE_RECOVER = 1 << 0,   // recover on errors
    XML_PARSE_NOERROR = 1 << 5,   // suppress error reports
    XML_PARSE_NOWARNING = 1 << 6, // suppress warning reports
    XML_PARSE_PEDANTIC = 1 << 7   // pedantic error reporting
};

static std::map<std::string, AudioChannelLayout> str2layout = {
    {"CH_LAYOUT_UNKNOWN", CH_LAYOUT_UNKNOWN},
    {"CH_LAYOUT_MONO", CH_LAYOUT_MONO},
    {"CH_LAYOUT_STEREO", CH_LAYOUT_STEREO},
    {"CH_LAYOUT_2POINT1", CH_LAYOUT_2POINT1},
    {"CH_LAYOUT_2_1", CH_LAYOUT_2_1},
    {"CH_LAYOUT_SURROUND", CH_LAYOUT_SURROUND},
    {"CH_LAYOUT_3POINT1", CH_LAYOUT_3POINT1},
    {"CH_LAYOUT_4POINT0", CH_LAYOUT_4POINT0},
    {"CH_LAYOUT_2_2", CH_LAYOUT_2_2},
    {"CH_LAYOUT_QUAD", CH_LAYOUT_QUAD},
    {"CH_LAYOUT_2POINT0POINT2", CH_LAYOUT_2POINT0POINT2},
    {"CH_LAYOUT_4POINT1", CH_LAYOUT_4POINT1},
    {"CH_LAYOUT_5POINT0", CH_LAYOUT_5POINT0},
    {"CH_LAYOUT_5POINT0_BACK", CH_LAYOUT_5POINT0_BACK},
    {"CH_LAYOUT_2POINT1POINT2", CH_LAYOUT_2POINT1POINT2},
    {"CH_LAYOUT_3POINT0POINT2", CH_LAYOUT_3POINT0POINT2},
    {"CH_LAYOUT_5POINT1", CH_LAYOUT_5POINT1},
    {"CH_LAYOUT_5POINT1_BACK", CH_LAYOUT_5POINT1_BACK},
    {"CH_LAYOUT_6POINT0", CH_LAYOUT_6POINT0},
    {"CH_LAYOUT_HEXAGONAL", CH_LAYOUT_HEXAGONAL},
    {"CH_LAYOUT_3POINT1POINT2", CH_LAYOUT_3POINT1POINT2},
    {"CH_LAYOUT_6POINT1", CH_LAYOUT_6POINT1},
    {"CH_LAYOUT_6POINT1_BACK", CH_LAYOUT_6POINT1_BACK},
    {"CH_LAYOUT_7POINT0", CH_LAYOUT_7POINT0},
    {"CH_LAYOUT_7POINT1", CH_LAYOUT_7POINT1},
    {"CH_LAYOUT_OCTAGONAL", CH_LAYOUT_OCTAGONAL},
    {"CH_LAYOUT_5POINT1POINT2", CH_LAYOUT_5POINT1POINT2},
    {"CH_LAYOUT_5POINT1POINT4", CH_LAYOUT_5POINT1POINT4},
    {"CH_LAYOUT_7POINT1POINT2", CH_LAYOUT_7POINT1POINT2},
    {"CH_LAYOUT_7POINT1POINT4", CH_LAYOUT_7POINT1POINT4},
    {"CH_LAYOUT_9POINT1POINT4", CH_LAYOUT_9POINT1POINT4},
    {"CH_LAYOUT_9POINT1POINT6", CH_LAYOUT_9POINT1POINT6},
    {"CH_LAYOUT_HOA_ORDER1_ACN_N3D", CH_LAYOUT_HOA_ORDER1_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER1_ACN_SN3D", CH_LAYOUT_HOA_ORDER1_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER1_FUMA", CH_LAYOUT_HOA_ORDER1_FUMA},
    {"CH_LAYOUT_HOA_ORDER2_ACN_N3D", CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER2_ACN_SN3D", CH_LAYOUT_HOA_ORDER2_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER2_FUMA", CH_LAYOUT_HOA_ORDER2_FUMA},
    {"CH_LAYOUT_HOA_ORDER3_ACN_N3D", CH_LAYOUT_HOA_ORDER3_ACN_N3D},
    {"CH_LAYOUT_HOA_ORDER3_ACN_SN3D", CH_LAYOUT_HOA_ORDER3_ACN_SN3D},
    {"CH_LAYOUT_HOA_ORDER3_FUMA", CH_LAYOUT_HOA_ORDER3_FUMA}};

AudioConverterParser::AudioConverterParser()
{
    AUDIO_INFO_LOG("AudioConverterParser created");
}
AudioConverterParser::~AudioConverterParser()
{
    AUDIO_INFO_LOG("AudioConverterParser released");
}

static int32_t LoadConfigCheck(xmlDoc* doc, xmlNode* currNode)
{
    if (currNode == nullptr) {
        AUDIO_ERR_LOG("error: could not parse file %{public}s", AUDIO_CONVERTER_CONFIG_FILE);
        return FILE_PARSE_ERROR;
    }
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_converter_conf"))) {
        AUDIO_ERR_LOG("Missing tag - audio_converter_conf: %{public}s", AUDIO_CONVERTER_CONFIG_FILE);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FILE_CONTENT_ERROR;
    }
    
    if (currNode->xmlChildrenNode) {
        return 0;
    } else {
        AUDIO_ERR_LOG("Missing node - audio_converter_conf: %s", AUDIO_CONVERTER_CONFIG_FILE);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FILE_CONTENT_ERROR;
    }
}

static void LoadConfigLibrary(ConverterConfig &result, xmlNode* currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("name"))) {
        AUDIO_ERR_LOG("missing information: library has no name attribute");
    } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("path"))) {
        AUDIO_ERR_LOG("missing information: library has no path attribute");
    } else {
        std::string libName = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name")));
        std::string libPath = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("path")));
        result.library = {libName, libPath};
    }
}

static void LoadConfigChannelLayout(ConverterConfig &result, xmlNode* currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("out_channel_layout"))) {
        AUDIO_ERR_LOG("missing information: config has no out_channel_layout attribute");
    } else {
        std::string strChannelLayout = reinterpret_cast<char*>
                                       (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("out_channel_layout")));
        if (str2layout.count(strChannelLayout) == 0) {
            AUDIO_ERR_LOG("unsupported format: invalid channel layout");
        } else 
            result.outChannelLayout = str2layout[strChannelLayout];
    }
}

static void LoadConfigVersion(ConverterConfig &result, xmlNode* currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("version"))) {
        AUDIO_ERR_LOG("missing information: audio_converter_conf node has no version attribute");
        return;
    }

    float pVersion = atof(reinterpret_cast<char*>
    (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("version"))));
    result.version = pVersion;
}

int32_t AudioConverterParser::LoadConfig(ConverterConfig &result)
{
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    AUDIO_INFO_LOG("AudioConverterParser::LoadConfig");
    if ((doc = xmlReadFile(AUDIO_CONVERTER_CONFIG_FILE, nullptr, 
                           XML_PARSE_NOERROR | XML_PARSE_NOWARNING)) == nullptr) {
        AUDIO_ERR_LOG("error: could not parse file %{public}s", AUDIO_CONVERTER_CONFIG_FILE);
        return FILE_PARSE_ERROR;
    }

    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;

    if (LoadConfigCheck(doc, currNode) == 0) {
        LoadConfigVersion(result, currNode);
        currNode = currNode->xmlChildrenNode;
    } else {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return FILE_CONTENT_ERROR;
    }

    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }

        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("library"))) {
            LoadConfigLibrary(result, currNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("converter_conf"))) {
            LoadConfigChannelLayout(result, currNode);
        }

        currNode = currNode->next;
    }
    
    if (doc) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
    }
    
    return 0;
}

} // namespace AudioStandard
} // namespace OHOS