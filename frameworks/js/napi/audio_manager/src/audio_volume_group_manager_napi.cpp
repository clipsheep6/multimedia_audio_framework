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

#include "audio_volume_group_manager_napi.h"
#include "audio_common_napi.h"
#include "audio_errors.h"
#include "hilog/log.h"
#include "audio_log.h"
#include "xpower_event_js.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_groupmanagerConstructor = nullptr;
int32_t AudioVolumeGroupManagerNapi::isConstructSuccess_ = SUCCESS;
#define GET_PARAMS(env, info, num) \
    size_t argc = num;             \
    napi_value argv[num] = {0};    \
    napi_value thisVar = nullptr;  \
    void *data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

struct AudioVolumeGroupManagerAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef = nullptr;
    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    int32_t ringMode;
    int32_t scene;
    int32_t deviceFlag;
    int32_t intValue;
    int32_t status = SUCCESS;
    int32_t groupId;
    int32_t adjustType;
    int32_t volumeAdjustStatus;
    bool isMute;
    bool isActive;
    bool isTrue;
    double volumeInDb;
    std::string key;
    std::string valueStr;
    int32_t networkId;
    AudioVolumeGroupManagerNapi *objectInfo;
};
namespace {
    const int ARGS_ONE = 1;
    const int ARGS_TWO = 2;
    const int ARGS_THREE = 3;
    const int ARGS_FOUR = 4;
    const int PARAM0 = 0;
    const int PARAM1 = 1;
    const int PARAM2 = 2;
    const int PARAM3 = 3;
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioVolumeGroupManagerNapi"};
    const std::string RINGERMODE_CALLBACK_NAME = "ringerModeChange";
}

static AudioVolumeType GetNativeAudioVolumeType(int32_t volumeType)
{
    AudioVolumeType result = STREAM_MUSIC;

    switch (volumeType) {
        case AudioCommonNapi::RINGTONE:
            result = STREAM_RING;
            break;
        case AudioCommonNapi::MEDIA:
            result = STREAM_MUSIC;
            break;
        case AudioCommonNapi::VOICE_CALL:
            result = STREAM_VOICE_CALL;
            break;
        case AudioCommonNapi::VOICE_ASSISTANT:
            result = STREAM_VOICE_ASSISTANT;
            break;
        case AudioCommonNapi::ALARM:
            result = STREAM_ALARM;
            break;
        case AudioCommonNapi::ACCESSIBILITY:
            result = STREAM_ACCESSIBILITY;
            break;
        case AudioCommonNapi::ULTRASONIC:
            result = STREAM_ULTRASONIC;
            break;
        case AudioManagerNapi::ALL:
            result = STREAM_ALL;
            break;
        default:
            result = STREAM_MUSIC;
            HiLog::Error(LABEL, "Unknown volume type, Set it to default MEDIA!");
            break;
    }
    return result;
}

static AudioRingerMode GetNativeAudioRingerMode(int32_t ringMode)
{
    AudioRingerMode result = RINGER_MODE_NORMAL;

    switch (ringMode) {
        case AudioVolumeGroupManagerNapi::RINGER_MODE_SILENT:
            result = RINGER_MODE_SILENT;
            break;
        case AudioVolumeGroupManagerNapi::RINGER_MODE_VIBRATE:
            result = RINGER_MODE_VIBRATE;
            break;
        case AudioVolumeGroupManagerNapi::RINGER_MODE_NORMAL:
            result = RINGER_MODE_NORMAL;
            break;
        default:
            result = RINGER_MODE_NORMAL;
            HiLog::Error(LABEL, "Unknown ringer mode requested by JS, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

static AudioVolumeGroupManagerNapi::AudioRingMode GetJsAudioRingMode(int32_t ringerMode)
{
    AudioVolumeGroupManagerNapi::AudioRingMode result = AudioVolumeGroupManagerNapi::RINGER_MODE_NORMAL;

    switch (ringerMode) {
        case RINGER_MODE_SILENT:
            result = AudioVolumeGroupManagerNapi::RINGER_MODE_SILENT;
            break;
        case RINGER_MODE_VIBRATE:
            result = AudioVolumeGroupManagerNapi::RINGER_MODE_VIBRATE;
            break;
        case RINGER_MODE_NORMAL:
            result = AudioVolumeGroupManagerNapi::RINGER_MODE_NORMAL;
            break;
        default:
            result = AudioVolumeGroupManagerNapi::RINGER_MODE_NORMAL;
            HiLog::Error(LABEL, "Unknown ringer mode returned from native, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}
static void CommonCallbackRoutine(napi_env env, AudioVolumeGroupManagerAsyncContext* &asyncContext,
    const napi_value &valueParam)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
    } else {
        napi_value message = nullptr;
        std::string messageValue = AudioCommonNapi::getMessageByCode(asyncContext->status);
        napi_create_string_utf8(env, messageValue.c_str(), NAPI_AUTO_LENGTH, &message);

        napi_value code = nullptr;
        napi_create_string_utf8(env, (std::to_string(asyncContext->status)).c_str(), NAPI_AUTO_LENGTH, &code);

        napi_create_error(env, code, message, &result[PARAM0]);
        napi_get_undefined(env, &result[PARAM1]);
    }

    if (asyncContext->deferred) {
        if (!asyncContext->status) {
            napi_resolve_deferred(env, asyncContext->deferred, result[PARAM1]);
        } else {
            napi_reject_deferred(env, asyncContext->deferred, result[PARAM0]);
        }
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, ARGS_TWO, result, &retVal);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);

    delete asyncContext;
    asyncContext = nullptr;
}

static void GetIntValueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int32(env, asyncContext->intValue, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioVolumeGroupManagerAsyncContext* is Null!");
    }
}

static void GetVolumeAdjustByStepAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int32(env, asyncContext->volumeAdjustStatus, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioVolumeGroupManagerAsyncContext* is Null!");
    }
}

static void GetVolumeDbAsyncCallbackComplete(napi_env env, napi_status, void *data)
{
    auto asyncContext = static_cast<AudioVolumeGroupManagerAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_double(env, asyncContext->volumeInDb, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioVolumeGroupManagerAsyncContext* is Null!");
    }
}

static void SetFunctionAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_get_undefined(env, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioManagerAsyncContext* is Null!");
    }
}

static void IsTrueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_get_boolean(env, asyncContext->isTrue, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioManagerAsyncContext* is Null!");
    }
}

// Constructor callback
napi_value AudioVolumeGroupManagerNapi::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);
    size_t argCount = 1;
    int32_t groupId = 0;

    napi_value args[1] = { nullptr};
    status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    napi_get_value_int32(env, args[0], &groupId);
    HiLog::Info(LABEL, "Construct() %{public}d", groupId);

    if (status == napi_ok) {
        unique_ptr<AudioVolumeGroupManagerNapi> groupmanagerNapi = make_unique<AudioVolumeGroupManagerNapi>();
        if (groupmanagerNapi != nullptr) {
            groupmanagerNapi->audioGroupMngr_ = AudioSystemManager::GetInstance()->GetGroupManager(groupId);
            if (groupmanagerNapi->audioGroupMngr_ == nullptr) {
                HiLog::Error(LABEL, "Failed in AudioVolumeGroupManagerNapi::Construct()!");
                AudioVolumeGroupManagerNapi::isConstructSuccess_ = NAPI_ERR_SYSTEM;
            }
            groupmanagerNapi->cachedClientId_ = getpid();
            status = napi_wrap(env, jsThis, static_cast<void*>(groupmanagerNapi.get()),
                AudioVolumeGroupManagerNapi::Destructor, nullptr, nullptr);
            if (status == napi_ok) {
                groupmanagerNapi.release();
                return jsThis;
            }
        }
    }

    HiLog::Error(LABEL, "Failed in AudioVolumeGroupManagerNapi::Construct()!");

    return undefinedResult;
}

void AudioVolumeGroupManagerNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<AudioVolumeGroupManagerNapi*>(nativeObject);
        delete obj;
        obj = nullptr;
        AUDIO_DEBUG_LOG("AudioVolumeGroupManagerNapi::Destructor delete AudioVolumeGroupManagerNapi obj done");
    }
}

napi_value AudioVolumeGroupManagerNapi::CreateAudioVolumeGroupManagerWrapper(napi_env env, int32_t groupId)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;
    napi_value groupId_;
    napi_create_int64(env, groupId, &groupId_);
    napi_value args[PARAM1] = {groupId_};
    status = napi_get_reference_value(env, g_groupmanagerConstructor, &constructor);
    if (status == napi_ok) {
        status = napi_new_instance(env, constructor, PARAM1, args, &result);
        if (status == napi_ok) {
            return result;
        }
    }

    HiLog::Error(LABEL, "Failed in AudioVolumeGroupManagerNapi::CreateaudioMngrWrapper!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->volLevel = context->objectInfo->audioGroupMngr_->GetVolume(
                        GetNativeAudioVolumeType(context->volType));
                    context->intValue = context->volLevel;
                    if (context->volLevel < 0) {
                        context->status = context->volLevel;
                    }
                }
            }, GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetVolumeSync(napi_env env, napi_callback_info info)
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
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("GetVolumeSync unwrap failure!");
        return result;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    if (valueType != napi_number) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    int32_t volType;
    napi_get_value_int32(env, argv[PARAM0], &volType);
    if (!AudioCommonNapi::IsLegalInputArgumentVolType(volType)) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    int32_t volLevel = audioVolumeGroupManagerNapi->audioGroupMngr_->GetVolume(GetNativeAudioVolumeType(volType));
    napi_create_int32(env, volLevel, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::SetVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_TWO) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volLevel);
                if (!AudioCommonNapi::IsLegalInputArgumentVolLevel(asyncContext->volLevel)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM2) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        HiviewDFX::ReportXPowerJsStackSysEvent(env, "VOLUME_CHANGE", "SRC=Audio");
        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->status = context->objectInfo->audioGroupMngr_->SetVolume(GetNativeAudioVolumeType(
                        context->volType), context->volLevel);
                }
            }, SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_user_initiated);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetMaxVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetMaxVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->volLevel = context->objectInfo->audioGroupMngr_->GetMaxVolume(
                        GetNativeAudioVolumeType(context->volType));
                    context->intValue = context->volLevel;
                    if (context->volLevel < 0) {
                        context->status = context->volLevel;
                    }
                }
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetMaxVolumeSync(napi_env env, napi_callback_info info)
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
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("GetMaxVolumeSync unwrap failure!");
        return result;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    if (valueType != napi_number) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    int32_t volType;
    napi_get_value_int32(env, argv[PARAM0], &volType);
    if (!AudioCommonNapi::IsLegalInputArgumentVolType(volType)) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    int32_t volLevel = audioVolumeGroupManagerNapi->audioGroupMngr_->GetMaxVolume(GetNativeAudioVolumeType(volType));
    napi_create_int32(env, volLevel, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetMinVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetMinVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->volLevel = context->objectInfo->audioGroupMngr_->GetMinVolume(
                        GetNativeAudioVolumeType(context->volType));
                    context->intValue = context->volLevel;
                    if (context->volLevel < 0) {
                        context->status = context->volLevel;
                    }
                }
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetMinVolumeSync(napi_env env, napi_callback_info info)
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
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("GetMinVolumeSync unwrap failure!");
        return result;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    if (valueType != napi_number) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    int32_t volType;
    napi_get_value_int32(env, argv[PARAM0], &volType);
    if (!AudioCommonNapi::IsLegalInputArgumentVolType(volType)) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    int32_t volLevel = audioVolumeGroupManagerNapi->audioGroupMngr_->GetMinVolume(GetNativeAudioVolumeType(volType));
    napi_create_int32(env, volLevel, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::SetMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1 && valueType == napi_boolean) {
                napi_get_value_bool(env, argv[i], &asyncContext->isMute);
            } else if (i == PARAM2) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetStreamMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->status = context->objectInfo->audioGroupMngr_->SetMute(GetNativeAudioVolumeType(
                        context->volType), context->isMute);
                }
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::IsStreamMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
                if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                    asyncContext->status = (asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM) ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "IsStreamMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->status = context->objectInfo->audioGroupMngr_->IsStreamMute(
                        GetNativeAudioVolumeType(context->volType), context->isMute);
                    context->isTrue = context->isMute;
                }
            },
            IsTrueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::IsStreamMuteSync(napi_env env, napi_callback_info info)
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
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("IsStreamMuteSync unwrap failure!");
        return result;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[PARAM0], &valueType);
    if (valueType != napi_number) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    int32_t volType;
    napi_get_value_int32(env, argv[PARAM0], &volType);
    if (!AudioCommonNapi::IsLegalInputArgumentVolType(volType)) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
        return result;
    }

    bool isMute;
    int32_t ret = audioVolumeGroupManagerNapi->audioGroupMngr_->IsStreamMute(
        GetNativeAudioVolumeType(volType), isMute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("IsStreamMute failure!");
        return result;
    }
    napi_get_boolean(env, isMute, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::SetRingerMode(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, " %{public}s IN", __func__);
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->ringMode);
                if (!AudioCommonNapi::IsLegalInputArgumentRingMode(asyncContext->ringMode)) {
                    asyncContext->status = asyncContext->status ==
                        NAPI_ERR_INVALID_PARAM ? NAPI_ERR_INVALID_PARAM : NAPI_ERR_UNSUPPORTED;
                }
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetRingerMode", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->status =
                    context->objectInfo->audioGroupMngr_->SetRingerMode(GetNativeAudioRingerMode(context->ringMode));
                }
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetRingerMode(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, " %{public}s IN", __func__);
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc > PARAM0) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[PARAM0], &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, argv[PARAM0], refCount, &asyncContext->callbackRef);
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetRingerMode", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->ringMode = GetJsAudioRingMode(context->objectInfo->audioGroupMngr_->GetRingerMode());
                    context->intValue = context->ringMode;
                    context->status = 0;
                }
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetRingerModeSync(napi_env env, napi_callback_info info)
{
    HiLog::Info(LABEL, " %{public}s IN", __func__);
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argCount = 0;
    void *native = nullptr;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Invalid parameters!");
        return result;
    }

    status = napi_unwrap(env, thisVar, &native);
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("GetRingerModeSync unwrap failure!");
        return result;
    }

    AudioRingerMode ringerMode = audioVolumeGroupManagerNapi->audioGroupMngr_->GetRingerMode();
    napi_create_int32(env, ringerMode, &result);

    return result;
}


napi_value AudioVolumeGroupManagerNapi::SetMicrophoneMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc < ARGS_ONE) {
            asyncContext->status = NAPI_ERR_INVALID_PARAM;
        }
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_boolean) {
                napi_get_value_bool(env, argv[i], &asyncContext->isMute);
            } else if (i == PARAM1) {
                if (valueType == napi_function) {
                    napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                }
                break;
            } else {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetMicrophoneMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->status = context->objectInfo->audioGroupMngr_->SetMicrophoneMute(context->isMute);
                }
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::IsMicrophoneMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        if (argc > PARAM0) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[PARAM0], &valueType);
            if (valueType == napi_function) {
                napi_create_reference(env, argv[PARAM0], refCount, &asyncContext->callbackRef);
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "IsMicrophoneMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
                if (context->status == SUCCESS) {
                    context->isMute = context->objectInfo->audioGroupMngr_->IsMicrophoneMute();
                    context->isTrue = context->isMute;
                }
            },
            IsTrueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioVolumeGroupManagerNapi::IsMicrophoneMuteSync(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value result = nullptr;
    size_t argCount = 0;
    void *native = nullptr;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("Invalid parameters!");
        return result;
    }

    status = napi_unwrap(env, thisVar, &native);
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("IsMicrophoneMuteSync unwrap failure!");
        return result;
    }

    bool isMute = audioVolumeGroupManagerNapi->audioGroupMngr_->IsMicrophoneMute();
    napi_get_boolean(env, isMute, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::On(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = 2;
    size_t argCount = 3;
    napi_value args[minArgCount + 1] = {nullptr, nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || argCount < minArgCount) {
        AUDIO_ERR_LOG("On fail to napi_get_cb_info/Requires min 2 parameters");
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
    }

    napi_valuetype eventType = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &eventType) != napi_ok || eventType != napi_string) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }
    std::string callbackName = AudioCommonNapi::GetStringArgument(env, args[0]);
    AUDIO_INFO_LOG("On callbackName: %{public}s", callbackName.c_str());

    AudioVolumeGroupManagerNapi *volumeGroupManagerNapi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&volumeGroupManagerNapi));

    napi_valuetype handler = napi_undefined;
    if (napi_typeof(env, args[PARAM1], &handler) != napi_ok || handler != napi_function) {
        AUDIO_ERR_LOG("On type mismatch for parameter 2");
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return undefinedResult;
    }

    if (!callbackName.compare(RINGERMODE_CALLBACK_NAME)) {
        if (volumeGroupManagerNapi->ringerModecallbackNapi_ == nullptr) {
            volumeGroupManagerNapi->ringerModecallbackNapi_ = std::make_shared<AudioRingerModeCallbackNapi>(env);
            int32_t ret = volumeGroupManagerNapi->audioGroupMngr_->SetRingerModeCallback(
                volumeGroupManagerNapi->cachedClientId_, volumeGroupManagerNapi->ringerModecallbackNapi_);
            if (ret) {
                AUDIO_ERR_LOG("SetRingerModeCallback Failed");
                return undefinedResult;
            } else {
                AUDIO_INFO_LOG("SetRingerModeCallback Success");
            }
        }

        std::shared_ptr<AudioRingerModeCallbackNapi> cb =
            std::static_pointer_cast<AudioRingerModeCallbackNapi>(volumeGroupManagerNapi->ringerModecallbackNapi_);
        cb->SaveCallbackReference(callbackName, args[PARAM1]);
    } else if (!callbackName.compare(MIC_STATE_CHANGE_CALLBACK_NAME)) {
        if (!volumeGroupManagerNapi->micStateChangeCallbackNapi_) {
            volumeGroupManagerNapi->micStateChangeCallbackNapi_=
                std::make_shared<AudioManagerMicStateChangeCallbackNapi>(env);
            if (!volumeGroupManagerNapi->micStateChangeCallbackNapi_) {
                AUDIO_ERR_LOG("Memory Allocation Failed !!");
            }

            int32_t ret = volumeGroupManagerNapi->audioGroupMngr_->SetMicStateChangeCallback(
                volumeGroupManagerNapi->micStateChangeCallbackNapi_);
            if (ret) {
                AUDIO_ERR_LOG("Registering Microphone Change Callback Failed");
            }
        }

        std::shared_ptr<AudioManagerMicStateChangeCallbackNapi> cb =
            std::static_pointer_cast<AudioManagerMicStateChangeCallbackNapi>(volumeGroupManagerNapi->
                micStateChangeCallbackNapi_);
        cb->SaveCallbackReference(callbackName, args[PARAM1]);

        AUDIO_DEBUG_LOG("On SetMicStateChangeCallback is successful");
    } else {
        AUDIO_ERR_LOG("No such callback supported");
        AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
    }
    return undefinedResult;
}

napi_value AudioVolumeGroupManagerNapi::IsVolumeUnadjustable(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    void* native = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);
    NAPI_ASSERT(env, argc <= ARGS_ONE, "requires 1 parameters maximum");
    status = napi_unwrap(env, thisVar, &native);
    auto* audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi*>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("IsVolumeUnadjustable unwrap failure!");
        return nullptr;
    }

    bool isVolumeUnadjustable = audioVolumeGroupManagerNapi->audioGroupMngr_->IsVolumeUnadjustable();
    napi_get_boolean(env, isVolumeUnadjustable, &result);

    AUDIO_INFO_LOG("IsVolumeUnadjustable is successful");
    return result;
}

bool GetArgvForAdjustVolumeByStep(napi_env env, size_t argc, napi_value* argv,
    unique_ptr<AudioVolumeGroupManagerAsyncContext> &asyncContext)
{
    const int32_t refCount = 1;

    if (argv == nullptr) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }
    if (argc < ARGS_ONE) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == PARAM0 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->adjustType);
            if (!AudioCommonNapi::IsLegalInputArgumentVolumeAdjustType(asyncContext->adjustType)) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        } else if (i == PARAM1) {
            if (valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
            }
            break;
        } else {
            AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
            return false;
        }
    }

    return true;
}

napi_value AudioVolumeGroupManagerNapi::AdjustVolumeByStep(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status != napi_ok || asyncContext->objectInfo == nullptr) {
        AUDIO_ERR_LOG("AdjustVolumeByStep unwrap failure!");
        return nullptr;
    }

    if (!GetArgvForAdjustVolumeByStep(env, argc, argv, asyncContext)) {
        return nullptr;
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "AdjustVolumeByStep", NAPI_AUTO_LENGTH, &resource);

    status = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
            if (context->status == SUCCESS) {
                context->volumeAdjustStatus = context->objectInfo->audioGroupMngr_->AdjustVolumeByStep(
                    static_cast<VolumeAdjustType>(context->adjustType));
                if (context->volumeAdjustStatus == SUCCESS) {
                    context->status = SUCCESS;
                } else if (context->volumeAdjustStatus == ERR_PERMISSION_DENIED) {
                    context->status = NAPI_ERR_NO_PERMISSION;
                } else {
                    context->status = NAPI_ERR_SYSTEM;
                }
            }
        },
        GetVolumeAdjustByStepAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
    if (status != napi_ok) {
        result = nullptr;
    } else {
        NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default));
        asyncContext.release();
    }

    AUDIO_INFO_LOG("AdjustVolumeByStep is successful");
    return result;
}

bool GetArgvForAdjustSystemVolumeByStep(napi_env env, size_t argc, napi_value* argv,
    unique_ptr<AudioVolumeGroupManagerAsyncContext> &asyncContext)
{
    const int32_t refCount = 1;
    if (argv == nullptr) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }
    if (argc < ARGS_ONE) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == PARAM0 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->volType);
            if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)
                || asyncContext->volType == AudioManagerNapi::ALL) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        } else if (i == PARAM1 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->adjustType);
            if (!AudioCommonNapi::IsLegalInputArgumentVolumeAdjustType(asyncContext->adjustType)) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        } else if (i == PARAM2) {
            if (valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
            }
            break;
        } else {
            AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
            return false;
        }
    }

    return true;
}

napi_value AudioVolumeGroupManagerNapi::AdjustSystemVolumeByStep(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status != napi_ok || asyncContext->objectInfo == nullptr) {
        AUDIO_ERR_LOG("AdjustSystemVolumeByStep unwrap failure!");
        return nullptr;
    }

    if (!GetArgvForAdjustSystemVolumeByStep(env, argc, argv, asyncContext)) {
        return nullptr;
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "AdjustSystemVolumeByStep", NAPI_AUTO_LENGTH, &resource);

    status = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
            if (context->status == SUCCESS) {
                context->volumeAdjustStatus = context->objectInfo->audioGroupMngr_->AdjustSystemVolumeByStep(
                    GetNativeAudioVolumeType(context->volType),
                    static_cast<VolumeAdjustType>(context->adjustType));
                if (context->volumeAdjustStatus == SUCCESS) {
                    context->status = SUCCESS;
                } else if (context->volumeAdjustStatus == ERR_PERMISSION_DENIED) {
                    context->status = NAPI_ERR_NO_PERMISSION;
                } else {
                    context->status = NAPI_ERR_SYSTEM;
                }
            }
        },
        GetVolumeAdjustByStepAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
    if (status != napi_ok) {
        result = nullptr;
    } else {
        NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default));
        asyncContext.release();
    }

    AUDIO_INFO_LOG("AdjustSystemVolumeByStep is successful");
    return result;
}

bool GetArgvForSystemVolumeInDb(napi_env env, size_t argc, napi_value* argv,
    unique_ptr<AudioVolumeGroupManagerAsyncContext> &asyncContext)
{
    const int32_t refCount = 1;
    if (argv == nullptr) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }
    if (argc > PARAM3) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[PARAM3], &valueType);
        if (valueType == napi_function) {
            napi_create_reference(env, argv[PARAM3], refCount, &asyncContext->callbackRef);
        }
    } else if (argc < PARAM3) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return false;
    }

    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == PARAM0 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->volType);
            if (!AudioCommonNapi::IsLegalInputArgumentVolType(asyncContext->volType)) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        } else if (i == PARAM1 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->volLevel);
            if (!AudioCommonNapi::IsLegalInputArgumentVolLevel(asyncContext->volLevel)) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
        } else if (i == PARAM2 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->deviceType);
            if (!AudioCommonNapi::IsLegalInputArgumentDeviceType(asyncContext->deviceType)) {
                asyncContext->status = NAPI_ERR_INVALID_PARAM;
            }
            break;
        } else {
            AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
            return false;
        }
    }
    return true;
}

napi_value AudioVolumeGroupManagerNapi::GetSystemVolumeInDb(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    GET_PARAMS(env, info, ARGS_FOUR);

    unique_ptr<AudioVolumeGroupManagerAsyncContext> asyncContext = make_unique<AudioVolumeGroupManagerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status != napi_ok || asyncContext->objectInfo == nullptr) {
        AUDIO_ERR_LOG("GetSystemVolumeInDb unwrap failure!");
        return nullptr;
    }

    if (!GetArgvForSystemVolumeInDb(env, argc, argv, asyncContext)) {
        return nullptr;
    }
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "GetSystemVolumeInDb", NAPI_AUTO_LENGTH, &resource);

    status = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            auto context = static_cast<AudioVolumeGroupManagerAsyncContext*>(data);
            if (context->status == SUCCESS) {
                context->volumeInDb = context->objectInfo->audioGroupMngr_->GetSystemVolumeInDb(
                    GetNativeAudioVolumeType(context->volType), context->volLevel,
                    static_cast<DeviceType>(context->deviceType));
                if (context->volumeInDb < 0) {
                    context->status = NAPI_ERR_SYSTEM;
                } else {
                    context->status = SUCCESS;
                }
            }
        },
        GetVolumeDbAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
    if (status != napi_ok) {
        result = nullptr;
    } else {
        NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncContext->work, napi_qos_default));
        asyncContext.release();
    }

    AUDIO_INFO_LOG("GetSystemVolumeInDb is successful");
    return result;
}

napi_value AudioVolumeGroupManagerNapi::GetSystemVolumeInDbSync(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    void *native = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);

    if (argc < ARGS_THREE) {
        AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
        return result;
    }

    status = napi_unwrap(env, thisVar, &native);
    auto *audioVolumeGroupManagerNapi = reinterpret_cast<AudioVolumeGroupManagerNapi *>(native);
    if (status != napi_ok || audioVolumeGroupManagerNapi == nullptr) {
        AUDIO_ERR_LOG("GetSystemVolumeInDbSync unwrap failure!");
        return result;
    }

    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (valueType != napi_number) {
            AudioCommonNapi::throwError(env, NAPI_ERR_INPUT_INVALID);
            return result;
        }

        bool isLegalInput = false;
        if (i == PARAM0) {
            napi_get_value_int32(env, argv[i], &volType);
            isLegalInput = AudioCommonNapi::IsLegalInputArgumentVolType(volType);
        } else if (i == PARAM1) {
            napi_get_value_int32(env, argv[i], &volLevel);
            isLegalInput = AudioCommonNapi::IsLegalInputArgumentVolLevel(volLevel);
        } else if (i == PARAM2) {
            napi_get_value_int32(env, argv[i], &deviceType);
            isLegalInput = AudioCommonNapi::IsLegalInputArgumentDeviceType(deviceType);
        }
        if (!isLegalInput) {
            AudioCommonNapi::throwError(env, NAPI_ERR_INVALID_PARAM);
            return result;
        }
    }

    double volumeInDb = audioVolumeGroupManagerNapi->audioGroupMngr_->GetSystemVolumeInDb(
        GetNativeAudioVolumeType(volType), volLevel, static_cast<DeviceType>(deviceType));
    napi_create_double(env, volumeInDb, &result);

    return result;
}

napi_value AudioVolumeGroupManagerNapi::Init(napi_env env, napi_value exports)
{
    AUDIO_INFO_LOG("Init");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_svc_group_mngr_properties[] = {
        DECLARE_NAPI_FUNCTION("setVolume", AudioVolumeGroupManagerNapi::SetVolume),
        DECLARE_NAPI_FUNCTION("getVolume", AudioVolumeGroupManagerNapi::GetVolume),
        DECLARE_NAPI_FUNCTION("getVolumeSync", AudioVolumeGroupManagerNapi::GetVolumeSync),
        DECLARE_NAPI_FUNCTION("getMaxVolume", AudioVolumeGroupManagerNapi::GetMaxVolume),
        DECLARE_NAPI_FUNCTION("getMaxVolumeSync", AudioVolumeGroupManagerNapi::GetMaxVolumeSync),
        DECLARE_NAPI_FUNCTION("getMinVolume", AudioVolumeGroupManagerNapi::GetMinVolume),
        DECLARE_NAPI_FUNCTION("getMinVolumeSync", AudioVolumeGroupManagerNapi::GetMinVolumeSync),
        DECLARE_NAPI_FUNCTION("mute", AudioVolumeGroupManagerNapi::SetMute),
        DECLARE_NAPI_FUNCTION("isMute", AudioVolumeGroupManagerNapi::IsStreamMute),
        DECLARE_NAPI_FUNCTION("isMuteSync", AudioVolumeGroupManagerNapi::IsStreamMuteSync),
        DECLARE_NAPI_FUNCTION("setRingerMode", SetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerMode", GetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerModeSync", GetRingerModeSync),
        DECLARE_NAPI_FUNCTION("setMicrophoneMute", SetMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMute", IsMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMuteSync", IsMicrophoneMuteSync),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("isVolumeUnadjustable", IsVolumeUnadjustable),
        DECLARE_NAPI_FUNCTION("adjustVolumeByStep", AdjustVolumeByStep),
        DECLARE_NAPI_FUNCTION("adjustSystemVolumeByStep", AdjustSystemVolumeByStep),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDb", GetSystemVolumeInDb),
        DECLARE_NAPI_FUNCTION("getSystemVolumeInDbSync", GetSystemVolumeInDbSync),
    };

    status = napi_define_class(env, AUDIO_VOLUME_GROUP_MNGR_NAPI_CLASS_NAME.c_str(),
        NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_svc_group_mngr_properties) / sizeof(audio_svc_group_mngr_properties[PARAM0]),
        audio_svc_group_mngr_properties, &constructor);
    if (status != napi_ok) {
        return result;
    }
    status = napi_create_reference(env, constructor, refCount, &g_groupmanagerConstructor);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, AUDIO_VOLUME_GROUP_MNGR_NAPI_CLASS_NAME.c_str(), constructor);
        if (status == napi_ok) {
            return exports;
        }
    }

    HiLog::Error(LABEL, "Failure in AudioVolumeGroupManagerNapi::Init()");
    return result;
}
} // namespace AudioStandard
} // namespace OHOS
