/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include <map>
#include <string>

namespace OHOS {
namespace AudioStandard{
const std::map<AudioServiceErrCode, std::string> ASERRCODE_INFOS = {
    {ASERR_OK, "success"},
    {ASERR_NO_MEMORY, "no memory"},
    {ASERR_INVALID_OPERATION, "opertation not be permitted"},
    {ASERR_INVALID_VAL, "invalid argument"},
    {ASERR_UNKNOWN, "unkown error"},
    {ASERR_SERVICE_DIED, "meida service died"},
    {ASERR_EXTEND_START, "extend start error code"},
};

std::string ASErrorToString(AudioServiceErrCode code)
{
    if (ASERRCODE_INFOS.count(code) != 0) {
        return ASERRCODE_INFOS.at(code);
    }

    if (code > ASERR_EXTEND_START) {
        return "extend error:" + std::to_string(static_cast<int32_t>(code - ASERR_EXTEND_START));
    }

    return "invalid error code:" + std::to_string(static_cast<int32_t>(code));
}
} // namespace AudioStandard
} // namespace OHOS
