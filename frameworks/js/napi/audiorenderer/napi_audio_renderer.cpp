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
#include "napi_audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref g_rendererConstructor = nullptr;

NapiAudioRenderer::NapiAudioRenderer()
    : audioRenderer_(nullptr), contentType_(CONTENT_TYPE_MUSIC), streamUsage_(STREAM_USAGE_MEDIA),
      /*deviceRole_(OUTPUT_DEVICE), deviceType_(DEVICE_TYPE_SPEAKER),*/ env_(nullptr),
      scheduleFromApiCall_(true), doNotScheduleWrite_(false), isDrainWriteQInProgress_(false) {}

void NapiAudioRenderer::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioRenderer *>(nativeObject);
        delete obj;
        obj = nullptr;
    }
    AUDIO_INFO_LOG("Destructor is successful");
}

napi_value NapiAudioRenderer::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_renderer_properties[] = {
        DECLARE_NAPI_FUNCTION("setRenderRate", SetRenderRate),
#if 0
        DECLARE_NAPI_FUNCTION("getRenderRate", GetRenderRate),
        DECLARE_NAPI_FUNCTION("getRenderRateSync", GetRenderRateSync),
        DECLARE_NAPI_FUNCTION("setRendererSamplingRate", SetRendererSamplingRate),
        DECLARE_NAPI_FUNCTION("getRendererSamplingRate", GetRendererSamplingRate),
        DECLARE_NAPI_FUNCTION("start", Start),
        DECLARE_NAPI_FUNCTION("write", Write),
        DECLARE_NAPI_FUNCTION("getAudioTime", GetAudioTime),
        DECLARE_NAPI_FUNCTION("getAudioTimeSync", GetAudioTimeSync),
        DECLARE_NAPI_FUNCTION("drain", Drain),
        DECLARE_NAPI_FUNCTION("pause", Pause),
        DECLARE_NAPI_FUNCTION("stop", Stop),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_FUNCTION("getBufferSize", GetBufferSize),
        DECLARE_NAPI_FUNCTION("getBufferSizeSync", GetBufferSizeSync),
        DECLARE_NAPI_FUNCTION("getAudioStreamId", GetAudioStreamId),
        DECLARE_NAPI_FUNCTION("getAudioStreamIdSync", GetAudioStreamIdSync),
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("getRendererInfo", GetRendererInfo),
        DECLARE_NAPI_FUNCTION("getRendererInfoSync", GetRendererInfoSync),
        DECLARE_NAPI_FUNCTION("getStreamInfo", GetStreamInfo),
        DECLARE_NAPI_FUNCTION("getStreamInfoSync", GetStreamInfoSync),
        DECLARE_NAPI_FUNCTION("setInterruptMode", SetInterruptMode),
        DECLARE_NAPI_FUNCTION("setInterruptModeSync", SetInterruptModeSync),
        DECLARE_NAPI_FUNCTION("getMinStreamVolume", GetMinStreamVolume),
        DECLARE_NAPI_FUNCTION("getMinStreamVolumeSync", GetMinStreamVolumeSync),
        DECLARE_NAPI_FUNCTION("getMaxStreamVolume", GetMaxStreamVolume),
        DECLARE_NAPI_FUNCTION("getMaxStreamVolumeSync", GetMaxStreamVolumeSync),
        DECLARE_NAPI_FUNCTION("getCurrentOutputDevices", GetCurrentOutputDevices),
        DECLARE_NAPI_FUNCTION("getCurrentOutputDevicesSync", GetCurrentOutputDevicesSync),
        DECLARE_NAPI_FUNCTION("getUnderflowCount", GetUnderflowCount),
        DECLARE_NAPI_FUNCTION("getUnderflowCountSync", GetUnderflowCountSync),
        DECLARE_NAPI_FUNCTION("getAudioEffectMode", GetAudioEffectMode),
        DECLARE_NAPI_FUNCTION("setAudioEffectMode", SetAudioEffectMode),
        DECLARE_NAPI_FUNCTION("setChannelBlendMode", SetChannelBlendMode),
        DECLARE_NAPI_GETTER("state", GetState)
    #endif
    };

    napi_property_descriptor static_prop[] = {
#if 0
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRenderer", CreateAudioRenderer),
        DECLARE_NAPI_STATIC_FUNCTION("createAudioRendererSync", CreateAudioRendererSync),
#endif
    };

    status = napi_define_class(env, NAPI_AUDIO_RENDERER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_renderer_properties) / sizeof(audio_renderer_properties[PARAM0]),
        audio_renderer_properties, &constructor);
    if (status != napi_ok) {
        return result;
    }
    status = napi_create_reference(env, constructor, refCount, &g_rendererConstructor);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, NAPI_AUDIO_RENDERER_CLASS_NAME.c_str(), constructor);
        if (status == napi_ok) {
            status = napi_define_properties(env, exports,
                                            sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
            if (status == napi_ok) {
                return exports;
            }
        }
    }

    AUDIO_ERR_LOG("Failure in NapiAudioRenderer::Init()");
    return result;
}

void NapiAudioRenderer::CreateRendererFailed()
{
    AUDIO_ERR_LOG("Renderer Create failed");
    NapiAudioRenderer::isConstructSuccess_ = NAPI_ERR_SYSTEM;
    if (AudioRenderer::CheckMaxRendererInstances() == ERR_OVERFLOW) {
        NapiAudioRenderer::isConstructSuccess_ = NAPI_ERR_STREAM_LIMIT;
    }
}

napi_value NapiAudioRenderer::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    size_t argCount = ARGS_TWO;
    napi_value thisVar = nullptr;
    status = napi_get_cb_info(env, info, &argCount, nullptr, &thisVar, nullptr);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "failed to napi_get_cb_info");

    unique_ptr<NapiAudioRenderer> rendererNapi = make_unique<NapiAudioRenderer>();
    CHECK_AND_RETURN_RET_LOG(rendererNapi != nullptr, result, "No memory");

    rendererNapi->env_ = env;
    rendererNapi->contentType_ = sRendererOptions_->rendererInfo.contentType;
    rendererNapi->streamUsage_ = sRendererOptions_->rendererInfo.streamUsage;

    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.samplingRate = sRendererOptions_->streamInfo.samplingRate;
    rendererOptions.streamInfo.encoding = sRendererOptions_->streamInfo.encoding;
    rendererOptions.streamInfo.format = sRendererOptions_->streamInfo.format;
    rendererOptions.streamInfo.channels = sRendererOptions_->streamInfo.channels;
    rendererOptions.rendererInfo.contentType = sRendererOptions_->rendererInfo.contentType;
    rendererOptions.rendererInfo.streamUsage = sRendererOptions_->rendererInfo.streamUsage;
    rendererOptions.privacyType = sRendererOptions_->privacyType;

    std::shared_ptr<AbilityRuntime::Context> abilityContext = NapiParamUtils::GetAbilityContext(env);
    std::string cacheDir = "";
    if (abilityContext != nullptr) {
        cacheDir = abilityContext->GetCacheDir();
    } else {
        cacheDir = "/data/storage/el2/base/temp";
    }
    rendererNapi->audioRenderer_ = AudioRenderer::Create(cacheDir, rendererOptions);

    if (rendererNapi->audioRenderer_ == nullptr) {
        CreateRendererFailed();
    }

#if 0 //等回调类实现后，进行补充
    if (rendererNapi->audioRenderer_ != nullptr && rendererNapi->callbackNapi_ == nullptr) {
         rendererNapi->callbackNapi_ = std::make_shared<AudioRendererCallbackNapi>(env);
        CHECK_AND_RETURN_RET_LOG(rendererNapi->callbackNapi_ != nullptr, result, "No memory");
        int32_t ret = rendererNapi->audioRenderer_->SetRendererCallback(rendererNapi->callbackNapi_);
        if (ret) {
            AUDIO_DEBUG_LOG("Construct SetRendererCallback failed");
        }
    }
#endif

    status = napi_wrap(env, thisVar, static_cast<void*>(rendererNapi.get()),
                       NapiAudioRenderer::Destructor, nullptr, nullptr);
    if (status == napi_ok) {
        rendererNapi.release();
        return thisVar;
    }

    AUDIO_ERR_LOG("Failed in NapiAudioRenderer::Construct()!");
    return result;
}

bool NapiAudioRenderer::CheckContextStatus(std::shared_ptr<AudioRendererAsyncContext> context)
{
    if (context == nullptr) {
        AUDIO_ERR_LOG("context object is nullptr.");
        return false;
    }
    if (context->native == nullptr) {
        context->status = napi_generic_failure;
        context->errCode = NAPI_ERR_SYSTEM;
        context->errMessage = "CheckContextStatus failed : renderernapi is nullptr";
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return true;
}

bool NapiAudioRenderer::CheckAudioRendererStatus(NapiAudioRenderer *napi,
    std::shared_ptr<AudioRendererAsyncContext> context)
{
    if (napi == nullptr) {
        AUDIO_ERR_LOG("napi object is nullptr.");
        return false;
    }
    if (napi->audioRenderer_ == nullptr) {
        context->status = napi_generic_failure;
        context->errCode = NAPI_ERR_SYSTEM;
        context->errMessage = "CheckContextStatus failed : renderernapi is nullptr";
        AUDIO_ERR_LOG("context object state is error.");
        return false;
    }
    return false;
}

napi_value NapiAudioRenderer::SetRenderRate(napi_env env, napi_callback_info info)
{
    AUDIO_INFO_LOG("SetRenderRate");
    auto context = std::make_shared<AudioRendererAsyncContext>();
    if (context == nullptr) {
        AUDIO_ERR_LOG("SetRenderRate failed : no memory");
        NapiAudioError::ThrowError(env, "SetRenderRate failed : no memory", NAPI_ERR_NO_MEMORY);
        return NapiParamUtils::GetUndefinedValue(env);
    }

    auto inputParser = [env, context](size_t argc, napi_value* argv) {
        CHECK_ARGS_RETURN_VOID(context, argc >= ARGS_ONE, "invalid arguments", NAPI_ERR_INVALID_PARAM);
        context->status = NapiParamUtils::GetValueInt32(env, context->audioRendererRate, argv[PARAM0]);
        CHECK_ARGS_RETURN_VOID(context, context->status == napi_ok, "get audioRendererRate failed",
            NAPI_ERR_INVALID_PARAM);
    };
    context->GetCbInfo(env, info, inputParser);

    auto executor = [context]() {
        if (!CheckContextStatus(context)) {
            return;
        }
        auto* napiAudioRenderer = reinterpret_cast<NapiAudioRenderer*>(context->native);
        if (context->status == napi_ok) {
            AudioRendererRate audioRenderRate = static_cast<AudioRendererRate>(context->audioRendererRate);
            if (!CheckAudioRendererStatus(napiAudioRenderer, context)) {
                return;
            }
            int32_t audioClientInvalidParamsErr = -2;
            context->intValue = napiAudioRenderer->audioRenderer_->SetRenderRate(audioRenderRate);
            if (context->intValue == SUCCESS) {
                context->status = napi_ok;
            } else if (context->intValue == audioClientInvalidParamsErr) {
                context->status = napi_generic_failure;
                context->errMessage = "error InvalidParams";
                context->errCode = NAPI_ERR_UNSUPPORTED;
            } else {
                context->status = napi_generic_failure;
                context->errMessage = "error system operation";
                context->errCode = NAPI_ERR_SYSTEM;
            }
        }
    };
    auto complete = [env](napi_value& output) {
        output = NapiParamUtils::GetUndefinedValue(env);
    };
    return NapiAsyncWork::Enqueue(env, context, "SetRenderRate", executor, complete);
}
} // namespace AudioStandard
} // namespace OHOS