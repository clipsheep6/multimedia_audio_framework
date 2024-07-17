/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_EN_INFO_H
#define ST_AUDIO_EN_INFO_H

#include <string>

#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {

enum EcType {
    EC_TYPE_NONE,
    EC_TYPE_SAME_ADAPTER,
    EC_TYPE_DIFF_ADAPTER
};

class AudioEcInfo {
public:
    AudioEcInfo() = default;
    virtual ~AudioEcInfo() = default;

    DeviceType inputDevice;
    DeviceType outputDevice;
    EcType ecType;
    std::string ecInputAdapter;
    std::string ecOutputAdapter;
    std::string samplingRate;
    std::string format;
    std::string channels;
    PipeInfo pipeInfo;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_EN_INFO_H