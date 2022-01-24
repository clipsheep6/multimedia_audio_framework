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

#include "audio_renderer_napi.h"
#include "audio_renderer_callback_napi.h"

#include "audio_errors.h"
#include "audio_manager_napi.h"
#include "audio_parameters_napi.h"
#include "hilog/log.h"
#include "media_log.h"
#include "securec.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_rendererConstructor = nullptr;
std::unique_ptr<AudioParameters> AudioRendererNapi::sAudioParameters_ = nullptr;
napi_ref AudioRendererNapi::audioRendererRate_ = nullptr;
napi_ref AudioRendererNapi::interruptEventType_ = nullptr;
napi_ref AudioRendererNapi::interruptHintType_ = nullptr;
napi_ref AudioRendererNapi::interruptForceType_ = nullptr;
napi_ref AudioRendererNapi::audioState_ = nullptr;

namespace {
    const int ARGS_ONE = 1;
    const int ARGS_TWO = 2;

    const int PARAM0 = 0;
    const int PARAM1 = 1;

    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioRendererNapi"};

#define GET_PARAMS(env, info, num) \
    size_t argc = num;             \
    napi_value argv[num] = {0};    \
    napi_value thisVar = nullptr;  \
    void *data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)
}

AudioRendererNapi::AudioRendererNapi()
    : audioRenderer_(nullptr), contentType_(CONTENT_TYPE_MUSIC), streamUsage_(STREAM_USAGE_MEDIA),
      deviceRole_(OUTPUT_DEVICE), deviceType_(DEVICE_TYPE_SPEAKER), env_(nullptr), wrapper_(nullptr),
      scheduleFromApiCall_(true), doNotScheduleWrite_(false), isDrainWriteQInProgress_(false) {}

AudioRendererNapi::~AudioRendererNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

void AudioRendererNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<AudioRendererNapi *>(nativeObject);
        delete obj;
    }
}

napi_status AudioRendererNapi::AddNamedProperty(napi_env env, napi_value object,
                                                const std::string name, int32_t enumValue)
{
    napi_status status;
    napi_value enumNapiValue;

    status = napi_create_int32(env, enumValue, &enumNapiValue);
    if (status == napi_ok) {
        status = napi_set_named_property(env, object, name.c_str(), enumNapiValue);
    }

    return status;
}

napi_value AudioRendererNapi::CreateAudioRendererRateObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: rendererRateMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &audioRendererRate_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateAudioRendererRateObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioRendererNapi::CreateAudioStateObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: audioStateMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateAudioStateObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &audioState_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateAudioStateObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioRendererNapi::CreateInterruptEventTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptEventTypeMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptEventTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptEventType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptEventTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioRendererNapi::CreateInterruptForceTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptForceTypeMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptForceType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptForceTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioRendererNapi::CreateInterruptHintTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptHintTypeMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptHintTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptHintType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptHintTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioRendererNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_renderer_properties[] = {
        DECLARE_NAPI_FUNCTION("setParams", SetParams),
        DECLARE_NAPI_FUNCTION("getParams", GetParams),
        DECLARE_NAPI_FUNCTION("setRenderRate", SetRenderRate),
        DECLARE_NAPI_FUNCTION("getRenderRate", GetRenderRate),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("getAudioTime", GetAudioTime),
        DECLARE_NAPI_FUNCTION("drain", Drain),
        DECLARE_NAPI_FUNCTION("pause", Pause),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getBufferSize", GetBufferSize),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_GETTER("state", GetState),
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRenderer", CreateAudioRenderer),
        DECLARE_NAPI_PROPERTY("AudioRendererRate", CreateAudioRendererRateObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptType", CreateInterruptEventTypeObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptForceType", CreateInterruptForceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptHint", CreateInterruptHintTypeObject(env)),
        DECLARE_NAPI_PROPERTY("AudioState", CreateAudioStateObject(env))
    };

    status = napi_define_class(env, AUDIO_RENDERER_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_renderer_properties) / sizeof(audio_renderer_properties[PARAM0]),
        audio_renderer_properties, &constructor);
    if (status != napi_ok) {
        return result;
    }

    status = napi_create_reference(env, constructor, refCount, &g_rendererConstructor);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, AUDIO_RENDERER_NAPI_CLASS_NAME.c_str(), constructor);
        if (status == napi_ok) {
            status = napi_define_properties(env, exports,
                                            sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    HiLog::Error(LABEL, "Failure in AudioRendererNapi::Init()");

    return result;
}

static int32_t GetAudioStreamType(napi_env env, napi_value value)
{
    napi_status status;
    int32_t streamType = AudioStreamType::STREAM_DEFAULT;

    status = napi_get_value_int32(env, value, &streamType);
    if (status == napi_ok) {
        switch (streamType) {
            case AudioManagerNapi::RINGTONE:
                streamType = AudioStreamType::STREAM_RING;
                break;
            case AudioManagerNapi::MEDIA:
                streamType = AudioStreamType::STREAM_MUSIC;
                break;
            case AudioManagerNapi::VOICE_ASSISTANT:
                streamType = AudioStreamType::STREAM_VOICE_ASSISTANT;
                break;
            case AudioManagerNapi::VOICE_CALL:
                streamType = AudioStreamType::STREAM_VOICE_CALL;
                break;
            default:
                streamType = AudioStreamType::STREAM_MUSIC;
                break;
        }
    }

    return streamType;
}

napi_value AudioRendererNapi::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis = nullptr;
    napi_value result = nullptr;
    size_t argCount = 1;
    napi_value args[1] = {0};

    napi_get_undefined(env, &result);
    status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok) {
        return result;
    }

    int32_t streamType = GetAudioStreamType(env, args[0]);
    HiLog::Info(LABEL, "AudioRendererNapi: Audio stream type: %{public}d", streamType);

    CHECK_AND_RETURN_RET_LOG(streamType != AudioStreamType::STREAM_DEFAULT, nullptr, "invalid streamType");

    unique_ptr<AudioRendererNapi> rendererNapi = make_unique<AudioRendererNapi>();
    CHECK_AND_RETURN_RET_LOG(rendererNapi != nullptr, nullptr, "No memory");

    rendererNapi->env_ = env;
    rendererNapi->audioRenderer_ = AudioRenderer::Create(static_cast<AudioStreamType>(streamType));
    CHECK_AND_RETURN_RET_LOG(rendererNapi->audioRenderer_ != nullptr, nullptr, "No memory");

    if (rendererNapi->callbackNapi_ == nullptr) {
        rendererNapi->callbackNapi_ = std::make_shared<AudioRendererCallbackNapi>(env);
        (void)rendererNapi->audioRenderer_->SetRendererCallback(rendererNapi->callbackNapi_);
    }

    status = napi_wrap(env, jsThis, static_cast<void*>(rendererNapi.get()),
                       AudioRendererNapi::Destructor, nullptr, &(rendererNapi->wrapper_));
    if (status == napi_ok) {
        rendererNapi.release();
        return jsThis;
    }

    HiLog::Error(LABEL, "Failed in AudioRendererNapi::Construct()!");

    return result;
}

napi_value AudioRendererNapi::CreateAudioRenderer(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;
    size_t argCount = 1;
    napi_value args[1] = {0};

    status = napi_get_cb_info(env, info, &argCount, args, nullptr, nullptr);
    if (status != napi_ok || argCount != 1) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return nullptr;
    }

    status = napi_get_reference_value(env, g_rendererConstructor, &constructor);
    if (status == napi_ok) {
        status = napi_new_instance(env, constructor, argCount, args, &result);
        if (status == napi_ok) {
            return result;
        }
    }

    HiLog::Error(LABEL, "Create audio renderer failed");
    napi_get_undefined(env, &result);

    return result;
}

void AudioRendererNapi::CommonCallbackRoutine(napi_env env, AudioRendererAsyncContext* &asyncContext,
                                              const napi_value &valueParam)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, "Error, Operation not supported or Failed", NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[PARAM0]);
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

void AudioRendererNapi::WriteAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int32(env, asyncContext->intValue, &valueParam);
        }
        if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
        } else {
            napi_value message = nullptr;
            napi_create_string_utf8(env, "Error, Operation not supported or Failed", NAPI_AUTO_LENGTH, &message);
            napi_create_error(env, nullptr, message, &result[PARAM0]);
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
        // queue the next write request from internal queue to napi queue
        if (!asyncContext->objectInfo->doNotScheduleWrite_ && !asyncContext->objectInfo->isDrainWriteQInProgress_) {
            if (!asyncContext->objectInfo->writeRequestQ_.empty()) {
                napi_queue_async_work(env, asyncContext->objectInfo->writeRequestQ_.front());
                asyncContext->objectInfo->writeRequestQ_.pop();
            } else {
                asyncContext->objectInfo->scheduleFromApiCall_ = true;
            }
        }

        delete asyncContext;
        asyncContext = nullptr;
    } else {
            HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::PauseAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            // set pause result to doNotScheduleWrite_
            asyncContext->objectInfo->doNotScheduleWrite_ = asyncContext->isTrue;
            napi_get_boolean(env, asyncContext->isTrue, &valueParam);
        }
        if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
        } else {
            napi_value message = nullptr;
            napi_create_string_utf8(env, "Error, Operation not supported or Failed", NAPI_AUTO_LENGTH, &message);
            napi_create_error(env, nullptr, message, &result[PARAM0]);
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
        // Pause failed . Contine Write
        if (!asyncContext->isTrue) {
            HiLog::Info(LABEL, "PauseAsyncCallbackComplete: Pasue failed, Continue Write");
            if (!asyncContext->objectInfo->writeRequestQ_.empty()) {
                napi_queue_async_work(env, asyncContext->objectInfo->writeRequestQ_.front());
                asyncContext->objectInfo->writeRequestQ_.pop();
            } else {
                asyncContext->objectInfo->scheduleFromApiCall_ = true;
            }
        }

        delete asyncContext;
        asyncContext = nullptr;
    } else {
            HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::StartAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_get_boolean(env, asyncContext->isTrue, &valueParam);
        }
        if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
        } else {
            napi_value message = nullptr;
            napi_create_string_utf8(env, "Error, Operation not supported or Failed", NAPI_AUTO_LENGTH, &message);
            napi_create_error(env, nullptr, message, &result[PARAM0]);
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
        // If start success , set doNotScheduleWrite_ = false and queue write request
        if (asyncContext->isTrue) {
            asyncContext->objectInfo->doNotScheduleWrite_ = false;
            if (!asyncContext->objectInfo->writeRequestQ_.empty()) {
                napi_queue_async_work(env, asyncContext->objectInfo->writeRequestQ_.front());
                asyncContext->objectInfo->writeRequestQ_.pop();
            } else {
                asyncContext->objectInfo->scheduleFromApiCall_ = true;
            }
        }

        delete asyncContext;
        asyncContext = nullptr;
    } else {
            HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::StopAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    napi_value result[ARGS_TWO] = {0};
    napi_value retVal;

    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            // set pause result to doNotScheduleWrite_
            asyncContext->objectInfo->doNotScheduleWrite_ = asyncContext->isTrue;
            napi_get_boolean(env, asyncContext->isTrue, &valueParam);
        }
        if (!asyncContext->status) {
        napi_get_undefined(env, &result[PARAM0]);
        result[PARAM1] = valueParam;
        } else {
            napi_value message = nullptr;
            napi_create_string_utf8(env, "Error, Operation not supported or Failed", NAPI_AUTO_LENGTH, &message);
            napi_create_error(env, nullptr, message, &result[PARAM0]);
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
    } else {
            HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::SetFunctionAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_get_undefined(env, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::AudioParamsAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            unique_ptr<AudioParameters> audioParams = make_unique<AudioParameters>();
            audioParams->format = asyncContext->sampleFormat;
            audioParams->samplingRate = asyncContext->samplingRate;
            audioParams->channels = asyncContext->channelCount;
            audioParams->encoding = asyncContext->encodingType;
            audioParams->contentType = asyncContext->contentType;
            audioParams->usage = asyncContext->usage;
            audioParams->deviceRole = asyncContext->deviceRole;
            audioParams->deviceType = asyncContext->deviceType;

            valueParam = AudioParametersNapi::CreateAudioParametersWrapper(env, audioParams);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::IsTrueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioRendererAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_get_boolean(env, asyncContext->isTrue, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::GetIntValueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioRendererAsyncContext *>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int32(env, asyncContext->intValue, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

void AudioRendererNapi::GetInt64ValueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioRendererAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int64(env, asyncContext->time, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioRendererAsyncContext* is Null!");
    }
}

int32_t AudioRendererNapi::SetAudioParameters(napi_env env, napi_value arg)
{
    int32_t format;
    int32_t channels;
    int32_t samplingRate;
    int32_t encoding;
    int32_t contentType = static_cast<int32_t>(CONTENT_TYPE_MUSIC);
    int32_t usage = static_cast<int32_t>(STREAM_USAGE_MEDIA);
    int32_t deviceRole = static_cast<int32_t>(OUTPUT_DEVICE);
    int32_t deviceType = static_cast<int32_t>(DEVICE_TYPE_SPEAKER);

    napi_value property;

    if ((napi_get_named_property(env, arg, "format", &property) != napi_ok)
        || napi_get_value_int32(env, property, &format) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the format argument!");
        return ERROR;
    }

    if ((napi_get_named_property(env, arg, "channels", &property) != napi_ok)
        || napi_get_value_int32(env, property, &channels) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the channels argument!");
        return ERROR;
    }

    if ((napi_get_named_property(env, arg, "samplingRate", &property) != napi_ok)
        || napi_get_value_int32(env, property, &samplingRate) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the samplingRate argument!");
        return ERROR;
    }

    if ((napi_get_named_property(env, arg, "encoding", &property) != napi_ok)
        || napi_get_value_int32(env, property, &encoding) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the encoding argument!");
        return ERROR;
    }

    if ((napi_get_named_property(env, arg, "contentType", &property) != napi_ok)
        || napi_get_value_int32(env, property, &contentType) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the contentType argument, set existing values");
        contentType = this->contentType_;
    }

    if ((napi_get_named_property(env, arg, "usage", &property) != napi_ok)
        || napi_get_value_int32(env, property, &usage) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the usage argument, set existing value");
        usage = this->streamUsage_;
    }

    if ((napi_get_named_property(env, arg, "deviceRole", &property) != napi_ok)
        || napi_get_value_int32(env, property, &deviceRole) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the device role argument, set existing value");
        deviceRole = this->deviceRole_;
    }

    if ((napi_get_named_property(env, arg, "deviceType", &property) != napi_ok)
        || napi_get_value_int32(env, property, &deviceType) != napi_ok) {
        HiLog::Error(LABEL, "Could not get the device type argument, set existing value");
        deviceType = this->deviceType_;
    }

    if (sAudioParameters_) {
        sAudioParameters_.reset();
    }

    sAudioParameters_ = std::make_unique<AudioParameters>();
    sAudioParameters_->format = static_cast<AudioSampleFormat>(format);
    sAudioParameters_->channels = static_cast<AudioChannel>(channels);
    sAudioParameters_->samplingRate = static_cast<AudioSamplingRate>(samplingRate);
    sAudioParameters_->encoding = static_cast<AudioEncodingType>(encoding);
    sAudioParameters_->contentType = static_cast<ContentType>(contentType);
    sAudioParameters_->usage = static_cast<StreamUsage>(usage);
    sAudioParameters_->deviceRole = static_cast<DeviceRole>(deviceRole);
    sAudioParameters_->deviceType = static_cast<DeviceType>(deviceType);

    return SUCCESS;
}

napi_value AudioRendererNapi::SetParams(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameters minimum");

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_object) {
                int32_t ret = asyncContext->objectInfo->SetAudioParameters(env, argv[PARAM0]);
                NAPI_ASSERT(env, ret == SUCCESS, "missing properties");
                asyncContext->sampleFormat = sAudioParameters_->format;
                asyncContext->samplingRate =  sAudioParameters_->samplingRate;
                asyncContext->channelCount = sAudioParameters_->channels;
                asyncContext->encodingType = sAudioParameters_->encoding;
                asyncContext->objectInfo->contentType_ = sAudioParameters_->contentType;
                asyncContext->objectInfo->streamUsage_ = sAudioParameters_->usage;
                asyncContext->objectInfo->deviceRole_ = sAudioParameters_->deviceRole;
                asyncContext->objectInfo->deviceType_ = sAudioParameters_->deviceType;
            } else if (i == PARAM1 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetParams", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                AudioRendererParams rendererParams;
                rendererParams.sampleFormat = context->sampleFormat;
                rendererParams.sampleRate =  context->samplingRate;
                rendererParams.channelCount = context->channelCount;
                rendererParams.encodingType = context->encodingType;
                context->status = context->objectInfo->audioRenderer_->SetParams(rendererParams);
                // To be removed when new renderer d.ts changes merged.
                if (context->objectInfo->callbackNapi_ == nullptr) {
                    context->objectInfo->callbackNapi_ = std::make_shared<AudioRendererCallbackNapi>(env);
                }
                (void)context->objectInfo->audioRenderer_->SetRendererCallback(context->objectInfo->callbackNapi_);
            },
            SetFunctionAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::GetParams(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetParams", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                AudioRendererParams rendererParams;
                context->status = context->objectInfo->audioRenderer_->GetParams(rendererParams);
                if (context->status == SUCCESS) {
                    context->sampleFormat = rendererParams.sampleFormat;
                    context->samplingRate = rendererParams.sampleRate;
                    context->channelCount = rendererParams.channelCount;
                    context->encodingType = rendererParams.encodingType;
                    context->contentType = context->objectInfo->contentType_;
                    context->usage = context->objectInfo->streamUsage_;
                    context->deviceRole = context->objectInfo->deviceRole_;
                    context->deviceType = context->objectInfo->deviceType_;
                }
            },
            AudioParamsAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::SetRenderRate(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[PARAM0], &asyncContext->audioRendererRate);
            } else if (i == PARAM1 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "SetRenderRate", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                AudioRendererRate audioRenderRate = static_cast<AudioRendererRate>(context->audioRendererRate);
                context->intValue = context->objectInfo->audioRenderer_->SetRenderRate(audioRenderRate);
                context->status = SUCCESS;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::GetRenderRate(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetRenderRate", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->intValue = context->objectInfo->audioRenderer_->GetRenderRate();
                context->status = SUCCESS;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Start(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Start", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->isTrue = context->objectInfo->audioRenderer_->Start();
                context->status = SUCCESS;
            },
            StartAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Write(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameters minimum");

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if ((i == PARAM0) && (valueType == napi_object)) {
                napi_get_arraybuffer_info(env, argv[i], &asyncContext->data, &asyncContext->bufferLen);
            } else if (i == PARAM1 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Write", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->status = ERROR;
                size_t bufferLen = context->bufferLen;
                auto buffer = std::make_unique<uint8_t[]>(bufferLen);
                if (buffer == nullptr) {
                    HiLog::Error(LABEL, "Renderer write buffer allocation failed");
                    return;
                }

                if (memcpy_s(buffer.get(), bufferLen, context->data, bufferLen)) {
                    HiLog::Info(LABEL, "Renderer mem copy failed");
                    return;
                }

                int32_t bytesWritten = 0;
                size_t minBytes = 4;
                while ((bytesWritten < bufferLen) && ((bufferLen - bytesWritten) > minBytes)) {
                    bytesWritten += context->objectInfo->audioRenderer_->Write(buffer.get() + bytesWritten,
                                                                               bufferLen - bytesWritten);
                    if (bytesWritten < 0) {
                        break;
                    }
                }

                context->status = SUCCESS;
                context->intValue = bytesWritten;
            },
            WriteAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else if (asyncContext->objectInfo->scheduleFromApiCall_) {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext->objectInfo->scheduleFromApiCall_ = false;
                asyncContext.release();
            } else {
                result = nullptr;
            }
        } else {
            asyncContext->objectInfo->writeRequestQ_.push(asyncContext->work);
            asyncContext.release();
        }
    }

    return result;
}

napi_value AudioRendererNapi::GetAudioTime(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetAudioTime", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->status = ERROR;
                Timestamp timestamp;
                if (context->objectInfo->audioRenderer_->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC)) {
                    const uint64_t secToNanosecond = 1000000000;
                    context->time = timestamp.time.tv_nsec + timestamp.time.tv_sec * secToNanosecond;
                    context->status = SUCCESS;
                }
            },
            GetInt64ValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Drain(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Drain", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->isTrue = context->objectInfo->audioRenderer_->Drain();
                context->status = SUCCESS;
            },
            IsTrueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            if (!asyncContext->objectInfo->doNotScheduleWrite_) {
                asyncContext->objectInfo->isDrainWriteQInProgress_ = true;
                while (!asyncContext->objectInfo->writeRequestQ_.empty()) {
                    napi_queue_async_work(env, asyncContext->objectInfo->writeRequestQ_.front());
                    asyncContext->objectInfo->writeRequestQ_.pop();
                }
                asyncContext->objectInfo->isDrainWriteQInProgress_ = false;
            }
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Pause(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Pause", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->isTrue = context->objectInfo->audioRenderer_->Pause();
                context->status = SUCCESS;
            },
            PauseAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext->objectInfo->doNotScheduleWrite_ = true;
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Stop(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Stop", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->isTrue = context->objectInfo->audioRenderer_->Stop();
                context->status = SUCCESS;
            },
            StopAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::Release(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "Release", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                context->isTrue = context->objectInfo->audioRenderer_->Release();
                context->status = SUCCESS;
            },
            IsTrueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::GetBufferSize(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioRendererAsyncContext> asyncContext = make_unique<AudioRendererAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_function) {
                napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
                break;
            } else {
                NAPI_ASSERT(env, false, "type mismatch");
            }
        }

        if (asyncContext->callbackRef == nullptr) {
            napi_create_promise(env, &asyncContext->deferred, &result);
        } else {
            napi_get_undefined(env, &result);
        }

        napi_value resource = nullptr;
        napi_create_string_utf8(env, "GetBufferSize", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioRendererAsyncContext *>(data);
                size_t bufferSize;
                context->status = context->objectInfo->audioRenderer_->GetBufferSize(bufferSize);
                if (context->status == SUCCESS) {
                    context->intValue = bufferSize;
                }
            },
            GetIntValueAsyncCallbackComplete, static_cast<void *>(asyncContext.get()), &asyncContext->work);
        if (status != napi_ok) {
            result = nullptr;
        } else {
            status = napi_queue_async_work(env, asyncContext->work);
            if (status == napi_ok) {
                asyncContext.release();
            } else {
                result = nullptr;
            }
        }
    }

    return result;
}

napi_value AudioRendererNapi::GetState(napi_env env, napi_callback_info info)
{
    napi_value jsThis = nullptr;
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    size_t argCount = 0;
    napi_status status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        MEDIA_INFO_LOG("Failed to retrieve details about the callback");
        return undefinedResult;
    }

    AudioRendererNapi *rendererNapi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&rendererNapi));
    CHECK_AND_RETURN_RET_LOG(status == napi_ok && rendererNapi != nullptr, undefinedResult, "Failed to get instance");

    CHECK_AND_RETURN_RET_LOG(rendererNapi->audioRenderer_ != nullptr, undefinedResult, "No memory");
    int32_t rendererState = rendererNapi->audioRenderer_->GetStatus();

    napi_value jsResult = nullptr;
    status = napi_create_int32(env, rendererState, &jsResult);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, undefinedResult, "napi_create_int32 error");

    MEDIA_INFO_LOG("AudioRendererNapi: GetState Complete, Current state: %{public}d", rendererState);
    return jsResult;
}

// Function to read string argument from napi_value
static string GetStringArgument(napi_env env, napi_value value)
{
    napi_status status;
    string strValue = "";
    size_t bufLength = 0;
    char *buffer = nullptr;

    status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLength);
    if (status == napi_ok && bufLength > 0) {
        buffer = (char *)malloc((bufLength + 1) * sizeof(char));
        if (buffer != nullptr) {
            status = napi_get_value_string_utf8(env, value, buffer, bufLength + 1, &bufLength);
            if (status == napi_ok) {
                strValue = buffer;
            }
            free(buffer);
            buffer = nullptr;
        }
    }

    return strValue;
}

napi_value AudioRendererNapi::On(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = 2;
    size_t argCount = minArgCount;
    napi_value args[minArgCount] = { nullptr, nullptr };
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[PARAM0] == nullptr || args[PARAM1] == nullptr) {
        HiLog::Error(LABEL, "On fail to napi_get_cb_info");
        return undefinedResult;
    }

    AudioRendererNapi *rendererNapi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&rendererNapi));
    NAPI_ASSERT(env, status == napi_ok && rendererNapi != nullptr, "Failed to retrieve audio renderer napi instance.");
    NAPI_ASSERT(env, rendererNapi->audioRenderer_ != nullptr, "audio renderer instance is null.");

    napi_valuetype valueType0 = napi_undefined;
    napi_valuetype valueType1 = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &valueType0) != napi_ok || valueType0 != napi_string ||
        napi_typeof(env, args[PARAM1], &valueType1) != napi_ok || valueType1 != napi_function) {
        return undefinedResult;
    }

    std::string callbackName = GetStringArgument(env, args[0]);
    MEDIA_DEBUG_LOG("AudioRenderNapi: callbackName: %{public}s", callbackName.c_str());

    CHECK_AND_RETURN_RET_LOG(rendererNapi->callbackNapi_ != nullptr, undefinedResult,
                             "AudioRendererNapi: callbackNapi_ is nullptr");
    std::shared_ptr<AudioRendererCallbackNapi> cb =
        std::static_pointer_cast<AudioRendererCallbackNapi>(rendererNapi->callbackNapi_);
    cb->SaveCallbackReference(callbackName, args[PARAM1]);
    return undefinedResult;
}
} // namespace AudioStandard
} // namespace OHOS
