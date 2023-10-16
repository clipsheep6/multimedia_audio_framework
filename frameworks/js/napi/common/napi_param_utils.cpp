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

#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {
napi_value NapiParamUtils::GetUndefinedValue(napi_env env)
{
    napi_value result {};
    napi_get_undefined(env, &result);
    return result;
}

napi_status NapiParamUtils::GetParam(const napi_env& env, napi_callback_info info, size_t &argc, napi_value *args)
{
    napi_value thisVar = nullptr;
    void *data;
    return napi_get_cb_info(env, info, &argc, args, &thisVar, &data);
}

std::shared_ptr<AbilityRuntime::Context> NapiParamUtils::GetAbilityContext(napi_env env)
{
    AUDIO_INFO_LOG("Getting context with FA model");
    auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
    if (ability == nullptr) {
        AUDIO_ERR_LOG("Failed to obtain ability in FA mode");
        return nullptr;
    }

    auto faContext = ability->GetAbilityContext();
    if (faContext == nullptr) {
        AUDIO_ERR_LOG("GetAbilityContext returned null in FA model");
        return nullptr;
    }

    return faContext;
}

napi_status NapiParamUtils::GetValueInt32(const napi_env& env, int32_t &value, napi_value in)
{
    napi_status status = napi_get_value_int32(env, in, &value);
    CHECK_RETURN(status == napi_ok, "GetValueInt32 napi_get_value_int32 failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueInt32(const napi_env& env, const int32_t &value, napi_value &result)
{
    napi_status status = napi_create_int32(env, value, &result);
    CHECK_RETURN(status == napi_ok, "SetValueInt32 napi_create_int32 failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueInt32(const napi_env& env, const std::string& fieldStr,
    int32_t &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueInt32 napi_get_named_property failed", status);
    status = GetValueInt32(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueInt32 napi_get_value_int32 failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueInt32(const napi_env& env, const std::string& fieldStr,
    const int32_t value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueInt32(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueInt32 napi_create_int32 failed", status);
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueInt32 napi_create_int32 failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueDouble(const napi_env& env, double &value, napi_value in)
{
    napi_status status = napi_get_value_double(env, in, &value);
    CHECK_RETURN(status == napi_ok, "GetValueDouble napi_get_value_double failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueDouble(const napi_env& env, const double &value, napi_value &result)
{
    napi_status status = napi_create_double(env, value, &result);
    CHECK_RETURN(status == napi_ok, "SetValueDouble napi_create_double failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueDouble(const napi_env& env, const std::string& fieldStr,
    double &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueDouble napi_get_named_property failed", status);
    status = GetValueDouble(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueDouble failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueDouble(const napi_env& env, const std::string& fieldStr,
    const double value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueDouble(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueDouble SetValueDouble failed", status);
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueDouble napi_set_named_property failed", status);
    return status;
}

std::string NapiParamUtils::GetPropertyString(napi_env env, napi_value value, const std::string &fieldStr)
{
    std::string invalid = "";
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, fieldStr.c_str(), &exist);
    if (status != napi_ok || !exist) {
        AUDIO_ERR_LOG("can not find %{public}s property", fieldStr.c_str());
        return invalid;
    }

    napi_value item = nullptr;
    if (napi_get_named_property(env, value, fieldStr.c_str(), &item) != napi_ok) {
        AUDIO_ERR_LOG("get %{public}s property fail", fieldStr.c_str());
        return invalid;
    }

    return GetStringArgument(env, item);
}

std::string NapiParamUtils::GetStringArgument(napi_env env, napi_value value)
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

napi_status NapiParamUtils::SetValueString(const napi_env &env, const std::string stringValue, napi_value &result)
{
    napi_status status = napi_create_string_utf8(env, stringValue.c_str(), NAPI_AUTO_LENGTH, &result);
    CHECK_RETURN(status == napi_ok, "SetValueString napi_create_string_utf8 failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueString(const napi_env &env, const std::string &fieldStr,
    const std::string stringValue, napi_value &result)
{
    napi_value value = nullptr;
    napi_status status = SetValueString(env, stringValue.c_str(), value);
    CHECK_RETURN(status == napi_ok, "SetValueString failed", status);
    status = napi_set_named_property(env, result, fieldStr.c_str(), value);
    CHECK_RETURN(status == napi_ok, "SetValueString napi_set_named_property failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueBoolean(const napi_env& env, bool &boolValue, napi_value in)
{
    napi_status status = napi_get_value_bool(env, in, &boolValue);
    CHECK_RETURN(status == napi_ok, "GetValueBoolean napi_get_boolean failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueBoolean(const napi_env& env, const bool boolValue, napi_value& result)
{
    napi_status status = napi_get_boolean(env, boolValue, &result);
    CHECK_RETURN(status == napi_ok, "SetValueBoolean napi_get_boolean failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueBoolean(const napi_env& env, const std::string& fieldStr,
    bool &boolValue, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueBoolean napi_get_named_property failed", status);
    status = GetValueBoolean(env, boolValue, jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueBoolean failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueBoolean(const napi_env& env, const std::string& fieldStr,
    const bool boolValue, napi_value& result)
{
    napi_value value = nullptr;
    napi_status status = SetValueBoolean(env, boolValue, value);
    CHECK_RETURN(status == napi_ok, "SetValueBoolean SetValueBoolean failed", status);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
    CHECK_RETURN(status == napi_ok, "SetValueBoolean napi_get_boolean failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueInt64(const napi_env& env, int64_t &value, napi_value in)
{
    napi_status status = napi_get_value_int64(env, in, &value);
    CHECK_RETURN(status == napi_ok, "GetValueInt64 napi_get_value_int64 failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueInt64(const napi_env& env, const int64_t &value, napi_value &result)
{
    napi_status status = napi_create_int64(env, value, &result);
    CHECK_RETURN(status == napi_ok, "SetValueInt64 napi_create_int64 failed", status);
    return status;
}

napi_status NapiParamUtils::GetValueInt64(const napi_env& env, const std::string& fieldStr,
    int64_t &value, napi_value in)
{
    napi_value jsValue = nullptr;
    napi_status status = napi_get_named_property(env, in, fieldStr.c_str(), &jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueInt64 napi_get_named_property failed", status);
    status = GetValueInt64(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "GetValueInt64 failed", status);
    return status;
}

napi_status NapiParamUtils::SetValueInt64(const napi_env& env, const std::string& fieldStr,
    const int64_t value, napi_value &result)
{
    napi_value jsValue = nullptr;
    napi_status status = SetValueInt64(env, value, jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueInt64 failed", status);
    status = napi_set_named_property(env, result, fieldStr.c_str(), jsValue);
    CHECK_RETURN(status == napi_ok, "SetValueInt64 napi_set_named_property failed", status);
    return status;
}
} // namespace AudioStandard
} // namespace OHOS