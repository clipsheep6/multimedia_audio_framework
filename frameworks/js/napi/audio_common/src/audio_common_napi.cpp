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

#include "audio_common_napi.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
std::string AudioCommonNapi::GetStringArgument(napi_env env, napi_value value)
{
    std::string strValue = "";
    size_t bufLength = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLength);
    if (status == napi_ok && bufLength > 0 && bufLength < PATH_MAX) {
        char *buffer = (char *)malloc((bufLength + 1) * sizeof(char));
        CHECK_AND_RETURN_RET_LOG(buffer != nullptr, strValue, "no memory");
        status = napi_get_value_string_utf8(env, value, buffer, bufLength + 1, &bufLength);
        if (status == napi_ok) {
            AUDIO_DEBUG_LOG("argument = %{public}s", buffer);
            strValue = buffer;
        }
        free(buffer);
        buffer = nullptr;
    }
    return strValue;
}

int32_t AudioCommonNapi::ConvertChannelToJs(AudioChannel channels)
{
    switch (channels) {
        case MONO:
            return AudioCommonNapi::AudioChannelMask::CHANNEL_1;
        case STEREO:
            return (AudioCommonNapi::AudioChannelMask::CHANNEL_1 | AudioCommonNapi::AudioChannelMask::CHANNEL_2);
        default:
            AUDIO_ERR_LOG("ConvertChannelToJs failed");
            return AudioCommonNapi::AudioChannelMask::CHANNEL_1;
    }
}

AudioChannel AudioCommonNapi::ConvertChannelToNative(int32_t channels)
{
    switch (channels) {
        case AudioCommonNapi::AudioChannelMask::CHANNEL_1:
            return MONO;
        case (AudioCommonNapi::AudioChannelMask::CHANNEL_1 | AudioCommonNapi::AudioChannelMask::CHANNEL_2):
            return STEREO;
        default:
            AUDIO_ERR_LOG("ConvertChannelToNative failed");
            return MONO;
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
