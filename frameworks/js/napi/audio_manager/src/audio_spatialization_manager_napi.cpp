/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_spatialization_manager_napi.h"

#include "audio_common_napi.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "hilog/log.h"
#include "napi_base_context.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_spatializationManagerConstructor = nullptr;

namespace {
    const int ARGS_ONE = 1;
    const int ARGS_TWO = 2;
    const int ARGS_THREE = 3;
    const int SIZE = 100;
    const int PARAM0 = 0;
    const int PARAM1 = 1;
    const int PARAM2 = 2;
    const int PARAM3 = 3;
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioInterruptManagerNapi"};
}

struct AudioSpatializationManagerAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef = nullptr;
    int32_t deviceFlag;
    bool bArgTransFlag = true;
    int32_t status = SUCCESS;
    int32_t groupId;
    std::string networkId;
    AudioSpatializationManagerNapi *objectInfo;
};

AudioSpatializationManagerNapi::AudioSpatializationManagerNapi()
    : audioSpatializationMngr_(nullptr), env_(nullptr) {}

AudioSpatializationManagerNapi::~AudioSpatializationManagerNapi() = default;

void AudioSpatializationManagerNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<AudioSpatializationManagerNapi *>(nativeObject);
        delete obj;
        obj = nullptr;
    }
}

napi_value AudioSpatializationManagerNapi::Construct(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("Construct");
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    unique_ptr<AudioSpatializationManagerNapi> audioSpatializationManagerNapi =
        make_unique<AudioSpatializationManagerNapi>();
    CHECK_AND_RETURN_RET_LOG(audioSpatializationManagerNapi != nullptr, result, "No memory");

    audioSpatializationManagerNapi->audioSpatializationMngr_ = AudioSpatializationManager::GetInstance();
    audioSpatializationManagerNapi->env_ = env;

    status = napi_wrap(env, thisVar, static_cast<void*>(audioSpatializationManagerNapi.get()),
        AudioSpatializationManagerNapi::Destructor, nullptr, nullptr);
    if (status == napi_ok) {
        audioSpatializationManagerNapi.release();
        return thisVar;
    }

    HiLog::Error(LABEL, "Failed in AudioSpatializationManagerNapi::Construct()!");
    return result;
}

napi_value AudioSpatializationManagerNapi::CreateSpatializationManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, g_spatializationManagerConstructor, &constructor);
    if (status == napi_ok) {
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        if (status == napi_ok) {
            return result;
        }
    }
    HiLog::Error(LABEL, "Failed in AudioSpatializationManagerNapi::CreateSpatializationManagerWrapper!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioSpatializationManagerNapi::Init(napi_env env, napi_value exports)
{
    AUDIO_INFO_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_spatialization_manager_properties[] = {
        DECLARE_NAPI_FUNCTION("isSpatializationEnabled", IsSpatializationEnabled),
        DECLARE_NAPI_FUNCTION("setSpatializationEnabled", SetSpatializationEnabled),
        DECLARE_NAPI_FUNCTION("isHeadTrackingEnabled", IsHeadTrackingEnabled),
        DECLARE_NAPI_FUNCTION("setHeadTrackingEnabled", SetHeadTrackingEnabled),
    };

    status = napi_define_class(env, AUDIO_SPATIALIZATION_MANAGER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
        nullptr,
        sizeof(audio_spatialization_manager_properties) / sizeof(audio_spatialization_manager_properties[PARAM0]),
        audio_spatialization_manager_properties, &constructor);
    if (status != napi_ok) {
        return result;
    }
    status = napi_create_reference(env, constructor, refCount, &g_spatializationManagerConstructor);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, AUDIO_SPATIALIZATION_MANAGER_NAPI_CLASS_NAME.c_str(),
            constructor);
        if (status == napi_ok) {
            return exports;
        }
    }

    HiLog::Error(LABEL, "Failure in AudioSpatializationManagerNapi::Init()");
    return result;
}

napi_value AudioSpatializationManagerNapi::IsSpatializationEnabled(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    void *native = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    if (argc < ARGS_ONE) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    status = napi_unwrap(env, thisVar, &native);
    auto *audioRoutingManagerNapi = reinterpret_cast<AudioRoutingManagerNapi *>(native);
    if (status != napi_ok || audioRoutingManagerNapi == nullptr) {
        AUDIO_ERR_LOG("IsCommunicationDeviceActiveSync unwrap failure!");
        return result;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    if (valueType != napi_number) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    int32_t deviceType;
    napi_get_value_int32(env, argv[PARAM0], &deviceType);
    if (!AudioCommonNapi::IsLegalInputArgumentActiveDeviceType(deviceType)) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    bool isActive = audioRoutingManagerNapi->audioMngr_->IsDeviceActive(static_cast<ActiveDeviceType>(deviceType));
    napi_get_boolean(env, isActive, &result);

    return result;
}
} // namespace AudioStandard
} // namespace OHOS
