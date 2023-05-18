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
#include <cassert>
#include <cstdint>
#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "audio_info.h"

#ifndef AUDIO_FRAMEWORK_AUDIO_EFFECT_H
#define AUDIO_FRAMEWORK_AUDIO_EFFECT_H

#define AUDIO_EFFECT_LIBRARY_INFO_SYM AELI
#define AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR "AELI"
#define EFFECT_STRING_LEN_MAX 64

namespace OHOS {
namespace AudioStandard {
// audio effect manager info
constexpr int32_t AUDIO_EFFECT_COUNT_UPPER_LIMIT = 20;
constexpr int32_t AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT = 1;

struct Library {
    std::string name;
    std::string path;
};

struct Effect {
    std::string name;
    std::string libraryName;
};

struct EffectChain {
    std::string name;
    std::vector<std::string> apply;
};

struct Device {
    std::string type;
    std::string chain;
};

struct Preprocess {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
};

struct Postprocess {
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
};

struct OriginalEffectConfig {
    float version;
    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<EffectChain> effectChains;
    std::vector<Preprocess> preProcess;
    std::vector<Postprocess> postProcess;
};

struct StreamEffectMode {
    std::string mode;
    std::vector<Device> devicePort;
};

struct Stream {
    std::string scene;
    std::vector<StreamEffectMode> streamEffectMode;
};

struct ProcessNew {
    std::vector<Stream> stream;
};

struct SupportedEffectConfig {
    std::vector<EffectChain> effectChains;
    ProcessNew preProcessNew;
    ProcessNew postProcessNew;
};

typedef struct EffectInterfaceS **EffectHandleT;

typedef struct AudioBufferS {
    size_t frameCount;       // number of frames in buffer
    union {
        void *raw;           // raw pointer to start of buffer
        float *f32;          // pointer to float 32 bit data at start of buffer
        int32_t *s32;        // pointer to signed 32 bit data at start of buffer
        int16_t *s16;        // pointer to signed 16 bit data at start of buffer
        uint8_t *u8;         // pointer to unsigned 8 bit data at start of buffer
    };
} AudioBufferT;

// for initial version
typedef struct EffectDescriptorS {
    std::string type;
    std::string id;
    uint32_t apiVersion;
    uint32_t flags;
    uint16_t cpuLoad;
    uint16_t memoryUsage;
    char name[EFFECT_STRING_LEN_MAX];
    char implementor[EFFECT_STRING_LEN_MAX];
} EffectDescriptorT;

struct EffectInterfaceS {
    int32_t (*Process)(EffectHandleT self, AudioBufferT *inBuffer, AudioBufferT *outBuffer);

    int32_t
    (*Command)(EffectHandleT self, uint32_t cmdCode, uint32_t cmdSize, void *pCmdData, uint32_t *replySize,
                void *pReplyData);

    int32_t (*GetDescriptor)(EffectHandleT self, EffectDescriptorT *pDescriptor);

    int32_t (*ProcessReverse)(EffectHandleT self, AudioBufferT *inBuffer, AudioBufferT *outBuffer);
};

// for initial version
typedef struct AudioEffectLibraryS {
    uint32_t tag;
    uint32_t version;
    const char *name;
    const char *implementor;

    int32_t (*CreateEffect)(const std::string *id, int32_t sessionId, int32_t ioId, EffectHandleT *pHandle);

    int32_t (*ReleaseEffect)(EffectHandleT handle);

    int32_t (*GetDescriptor)(const std::string *id, EffectDescriptorT *pDescriptor);
} AudioEffectLibraryT;

typedef struct LibEntryS {
    AudioEffectLibraryT *desc;
    std::string name;
    std::string path;
    void *handle;
    std::vector <std::unique_ptr<EffectDescriptorT>> effects;
} LibEntryT;

} // namespce AudioStandard
} // namespace OHOS

#endif //AUDIO_FRAMEWORK_AUDIO_EFFECT_H