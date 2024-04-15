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
#undef LOG_TAG
#define LOG_TAG "NapiAsrProcessingController"

#include "napi_asr_processing_controller.h"

//#include "napi_audio_manager.h"
//#include "napi_audio_routing_manager.h"
//#include "napi_audio_stream_manager.h"
//#include "napi_audio_volume_manager.h"
//#include "napi_audio_interrupt_manager.h"
//#include "napi_audio_spatialization_manager.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "audio_utils.h"

//#include "xpower_event_js.h"
//
//#include "napi_audio_manager_callbacks.h"
//#include "napi_audio_ringermode_callback.h"
//#include "napi_audio_manager_interrupt_callback.h"
//#include "napi_audio_volume_key_event.h"

namespace OHOS {
    namespace AudioStandard {
        using namespace std;
        using namespace HiviewDFX;
        static __thread napi_ref g_managerConstructor = nullptr;

        NapiAsrProcessingController::NapiAsrProcessingController()
            : audioMngr_(nullptr), env_(nullptr) {}

        NapiAsrProcessingController::~NapiAsrProcessingController()
        {
            AUDIO_DEBUG_LOG("NapiAsrProcessingController::~NapiAsrProcessingController()");
        }

        

        NapiAsrProcessingController* NapiAsrProcessingController::GetParamWithSync(const napi_env& env, napi_callback_info info,
            size_t& argc, napi_value* args)
        {
            napi_status status;
            NapiAsrProcessingController* napiAsrProcessingController = nullptr;
            napi_value jsThis = nullptr;

            status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok && jsThis != nullptr, nullptr,
                "GetParamWithSync fail to napi_get_cb_info");

            status = napi_unwrap(env, jsThis, (void**)&napiAsrProcessingController);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, nullptr, "napi_unwrap failed");
            CHECK_AND_RETURN_RET_LOG(napiAsrProcessingController != nullptr && napiAsrProcessingController->audioMngr_ !=
                nullptr, napiAsrProcessingController, "GetParamWithSync fail to napi_unwrap");
            return napiAsrProcessingController;
        }

        

        napi_status NapiAsrProcessingController::InitNapiAsrProcessingController(napi_env env, napi_value& constructor)
        {
            napi_property_descriptor audio_svc_mngr_properties[] = {
                
                DECLARE_NAPI_FUNCTION("setAsrAecMode", SetAsrAecMode),
                DECLARE_NAPI_FUNCTION("getAsrAecMode", GetAsrAecMode),
                DECLARE_NAPI_FUNCTION("setAsrNoiseSuppressionMode", SetAsrNoiseSuppressionMode),
                DECLARE_NAPI_FUNCTION("getAsrNoiseSuppressionMode", GetAsrNoiseSuppressionMode),

                DECLARE_NAPI_FUNCTION("isWhispering", IsWhispering),

            };

            napi_status status = napi_define_class(env, NAPI_ASR_PROCESSING_CONTROLLER_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH,
                Construct, nullptr,
                sizeof(audio_svc_mngr_properties) / sizeof(audio_svc_mngr_properties[PARAM0]),
                audio_svc_mngr_properties, &constructor);
            return status;
        }

        napi_value NapiAsrProcessingController::Init(napi_env env, napi_value exports)
        {
            AUDIO_DEBUG_LOG("Init");
            napi_status status;
            napi_value constructor;
            napi_value result = nullptr;
            const int32_t refCount = 1;

            napi_property_descriptor static_prop[] = {
                DECLARE_NAPI_STATIC_FUNCTION("createAsrProcessingController", createAsrProcessingController),
            };

            status = InitNapiAsrProcessingController(env, constructor);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "InitNapiAudioRenderer fail");

            status = napi_create_reference(env, constructor, refCount, &g_managerConstructor);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_create_reference fail");
            status = napi_set_named_property(env, exports, NAPI_ASR_PROCESSING_CONTROLLER_CLASS_NAME.c_str(), constructor);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_set_named_property fail");
            status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[PARAM0]),
                static_prop);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, result, "napi_define_properties fail");
            return exports;
        }

        void NapiAsrProcessingController::Destructor(napi_env env, void* nativeObject, void* finalizeHint)
        {
            if (nativeObject != nullptr) {
                auto obj = static_cast<NapiAsrProcessingController*>(nativeObject);
                ObjectRefMap<NapiAsrProcessingController>::DecreaseRef(obj);
                AUDIO_DEBUG_LOG("NapiAsrProcessingController::Destructor delete NapiAsrProcessingController obj done");
            }
        }

        napi_value NapiAsrProcessingController::Construct(napi_env env, napi_callback_info info)
        {
            napi_status status;
            napi_value jsThis;
            napi_value undefinedResult = nullptr;
            NapiParamUtils::GetUndefinedValue(env);
            size_t argCount = PARAM0;

            status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
            if (status == napi_ok) {
                unique_ptr<NapiAsrProcessingController> managerNapi = make_unique<NapiAsrProcessingController>();
                if (managerNapi != nullptr) {
                    ObjectRefMap<NapiAsrProcessingController>::Insert(managerNapi.get());
                    managerNapi->env_ = env;
                    managerNapi->audioMngr_ = AudioSystemManager::GetInstance();
                    managerNapi->cachedClientId_ = getpid();

                    status = napi_wrap(env, jsThis, static_cast<void*>(managerNapi.get()),
                        NapiAsrProcessingController::Destructor, nullptr, nullptr);
                    if (status != napi_ok) {
                        ObjectRefMap<NapiAsrProcessingController>::Erase(managerNapi.get());
                        return undefinedResult;
                    }
                    managerNapi.release();
                    return jsThis;
                }
            }
            return undefinedResult;
        }

        napi_value NapiAsrProcessingController::CreateAudioManagerWrapper(napi_env env)
        {
            napi_status status;
            napi_value result = nullptr;
            napi_value constructor;

            status = napi_get_reference_value(env, g_managerConstructor, &constructor);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed in CreateAudioManagerWrapper, %{public}d", status);
                goto fail;
            }
            status = napi_new_instance(env, constructor, 0, nullptr, &result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("napi_new_instance failed, sttaus:%{public}d", status);
                goto fail;
            }
            return result;

        fail:
            napi_get_undefined(env, &result);
            return result;
        }

        /** ssl **/
        napi_value NapiAsrProcessingController::createAsrProcessingController(napi_env env, napi_callback_info info) 
        {
            napi_status status;
            size_t argCount = PARAM0;
            status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
            if (status != napi_ok || argCount != 0) {
                AUDIO_ERR_LOG("Invalid arguments!");
                return nullptr;
            }
            return NapiAsrProcessingController::CreateAudioManagerWrapper(env);
        }

        /** ssl **/

        napi_value NapiAsrProcessingController::SetAsrAecMode(napi_env env, napi_callback_info info)
        {
            AUDIO_INFO_LOG("SetAsrAecMode");
            napi_value result = nullptr;
            size_t argc = PARAM0;
            auto* napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
            if (argc != PARAM1) {
                NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
                return result;
            }
            int32_t asrAecMode=0;
            /*NapiParamUtils::GetValueInt32(env, asrAecMode, args[PARAM0]);*/
            CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
            //int32_t result = 
            napiAudioManager->audioMngr_->SetAsrAecMode(static_cast<AsrAecMode>(asrAecMode));
            // 处理返回值
            // switch () {} todo
            return result;
        }
        napi_value NapiAsrProcessingController::GetAsrAecMode(napi_env env, napi_callback_info info)
        {
            AUDIO_INFO_LOG("GetAsrAecMode");
            napi_value result = nullptr;
            size_t argc = PARAM0;
            auto* napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
            if (argc > PARAM0) {
                NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
                return result;
            }
            AsrAecMode asrAecMode;
            //NapiParamUtils::GetValueInt32(env, asrAecMode, args[PARAM0]);
            CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
            //int32_t result = napiAudioManager->audioMngr_->GetAsrAecMode(static_cast<AsrAecMode>(asrAecMode));
            napiAudioManager->audioMngr_->GetAsrAecMode(asrAecMode);
            // 处理返回值
            // switch () {} todo
            return result;
        }
        napi_value NapiAsrProcessingController::SetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info)
        {
            AUDIO_INFO_LOG("SetAsrNoiseSuppressionMode");
            napi_value result = nullptr;
            size_t argc = PARAM0;
            auto* napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
            if (argc > PARAM0) {
                NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
                return result;
            }
            int32_t asrNoiseSuppressionModeInt=0;
            //NapiParamUtils::GetValueInt32(env, asrNoiseSuppressionModeInt, args[PARAM0]);
            CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
            napiAudioManager->audioMngr_->SetAsrNoiseSuppressionMode(static_cast<AsrNoiseSuppressionMode>(asrNoiseSuppressionModeInt));
            // 处理返回值
            // switch () {} todo
            return result;
        }
        napi_value NapiAsrProcessingController::GetAsrNoiseSuppressionMode(napi_env env, napi_callback_info info)
        {
            AUDIO_INFO_LOG("GetAsrNoiseSuppressionMode");
            napi_value result = nullptr;
            size_t argc = PARAM0;
            auto* napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
            if (argc > PARAM0) {
                NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
                return result;
            }
            AsrNoiseSuppressionMode asrNoiseSuppressionModeInt;
            //NapiParamUtils::GetValueInt32(env, asrNoiseSuppressionModeInt, args[PARAM0]);
            CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
            napiAudioManager->audioMngr_->GetAsrNoiseSuppressionMode(asrNoiseSuppressionModeInt);
            // 处理返回值
            // switch () {} todo
            return result;
        }
        napi_value NapiAsrProcessingController::IsWhispering(napi_env env, napi_callback_info info)
        {
            AUDIO_INFO_LOG("IsWhispering");
            napi_value result = nullptr;
            size_t argc = PARAM0;
            auto* napiAudioManager = GetParamWithSync(env, info, argc, nullptr);
            if (argc >PARAM0) {
                NapiAudioError::ThrowError(env, NAPI_ERROR_INVALID_PARAM);
                return result;
            }
            CHECK_AND_RETURN_RET_LOG(napiAudioManager != nullptr, result, "napiAudioManager is nullptr");
            CHECK_AND_RETURN_RET_LOG(napiAudioManager->audioMngr_ != nullptr, result, "audioMngr_ is nullptr");
            napiAudioManager->audioMngr_->IsWhispering();
            // 处理返回值
            // switch () {} todo
            return result;
        }

        

    }  // namespace AudioStandard
}  // namespace OHOS


