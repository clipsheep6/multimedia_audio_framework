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

#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
napi_status NapiAudioError::ThrowError(napi_env env, const char* napiMessage, int32_t napiCode)
{
    napi_value message = nullptr;
    napi_value code = nullptr;
    napi_value result = nullptr;
    napi_create_string_utf8(env, napiMessage, NAPI_AUTO_LENGTH, &message);
    napi_create_error(env, nullptr, message, &result);
    napi_create_int32(env, napiCode, &code);
    napi_set_named_property(env, result, "code", code);
    napi_throw(env, result);
    return napi_ok;
}

void NapiAudioError::ThrowError(napi_env env, int32_t code)
{
    std::string messageValue = getMessageByCode(code);
    napi_throw_error(env, (std::to_string(code)).c_str(), messageValue.c_str());
}

std::string NapiAudioError::getMessageByCode(int32_t &code)
{
    std::string err_message;
    switch (code) {
        case NAPI_ERR_INVALID_PARAM:
            err_message = NAPI_ERR_INVALID_PARAM_INFO;
            break;
        case NAPI_ERR_NO_MEMORY:
            err_message = NAPI_ERR_NO_MEMORY_INFO;
            break;
        case NAPI_ERR_ILLEGAL_STATE:
            err_message = NAPI_ERR_ILLEGAL_STATE_INFO;
            break;
        case NAPI_ERR_UNSUPPORTED:
        case ERR_NOT_SUPPORTED:
            err_message = NAPI_ERR_UNSUPPORTED_INFO;
            code = NAPI_ERR_UNSUPPORTED;
            break;
        case NAPI_ERR_TIMEOUT:
            err_message = NAPI_ERR_TIMEOUT_INFO;
            break;
        case NAPI_ERR_STREAM_LIMIT:
            err_message = NAPI_ERR_STREAM_LIMIT_INFO;
            break;
        case NAPI_ERR_SYSTEM:
            err_message = NAPI_ERR_SYSTEM_INFO;
            break;
        case NAPI_ERR_INPUT_INVALID:
            err_message = NAPI_ERR_INPUT_INVALID_INFO;
            break;
        case NAPI_ERR_PERMISSION_DENIED:
        case ERR_PERMISSION_DENIED:
            err_message = NAPI_ERROR_PERMISSION_DENIED_INFO;
            code = NAPI_ERR_PERMISSION_DENIED;
            break;
        case NAPI_ERR_NO_PERMISSION:
            err_message = NAPI_ERR_NO_PERMISSION_INFO;
            break;
        default:
            err_message = NAPI_ERR_SYSTEM_INFO;
            code = NAPI_ERR_SYSTEM;
            break;
    }
    return err_message;
}
} // namespace AudioStandard
} // namespace OHOS