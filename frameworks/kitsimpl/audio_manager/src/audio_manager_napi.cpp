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

#include "audio_capturer_napi.h"
#include "audio_device_descriptor_napi.h"
#include "audio_errors.h"
#include "audio_manager_callback_napi.h"
#include "audio_parameters_napi.h"
#include "audio_renderer_napi.h"
#include "hilog/log.h"
#include "media_log.h"
#include "audio_manager_napi.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {
static __thread napi_ref managerConstructor_ = nullptr;
napi_ref AudioManagerNapi::audioVolumeTypeRef_ = nullptr;
napi_ref AudioManagerNapi::deviceFlagRef_ = nullptr;
napi_ref AudioManagerNapi::deviceRoleRef_ = nullptr;
napi_ref AudioManagerNapi::deviceTypeRef_ = nullptr;
napi_ref AudioManagerNapi::activeDeviceTypeRef_ = nullptr;
napi_ref AudioManagerNapi::audioRingModeRef_ = nullptr;
napi_ref AudioManagerNapi::interruptActionType_ = nullptr;
napi_ref AudioManagerNapi::interruptHint_ = nullptr;
napi_ref AudioManagerNapi::interruptType_ = nullptr;

#define GET_PARAMS(env, info, num) \
    size_t argc = num;             \
    napi_value argv[num] = {0};    \
    napi_value thisVar = nullptr;  \
    void *data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

struct AudioManagerAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef = nullptr;
    int32_t volType;
    int32_t volLevel;
    int32_t deviceType;
    int32_t ringMode;
    int32_t deviceFlag;
    int32_t intValue;
    int32_t status;
    bool isMute;
    bool isActive;
    bool isTrue;
    string key;
    string valueStr;
    AudioManagerNapi *objectInfo;
    vector<sptr<AudioDeviceDescriptor>> deviceDescriptors;
};

namespace {
    const int ARGS_ONE = 1;
    const int ARGS_TWO = 2;
    const int ARGS_THREE = 3;
    const int PARAM0 = 0;
    const int PARAM1 = 1;
    const int PARAM2 = 2;
    constexpr HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AudioManagerNapi"};
}

AudioManagerNapi::AudioManagerNapi()
    : audioMngr_(nullptr), env_(nullptr), wrapper_(nullptr) {}

AudioManagerNapi::~AudioManagerNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
    MEDIA_DEBUG_LOG("AudioManagerNapi::~AudioManagerNapi()");
}

void AudioManagerNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<AudioManagerNapi*>(nativeObject);
        delete obj;
        MEDIA_DEBUG_LOG("AudioManagerNapi::Destructor delete AudioManagerNapi obj done");
    }
}

static AudioSystemManager::AudioVolumeType GetNativeAudioVolumeType(int32_t volumeType)
{
    AudioSystemManager::AudioVolumeType result = AudioSystemManager::STREAM_MUSIC;

    switch (volumeType) {
        case AudioManagerNapi::RINGTONE:
            result = AudioSystemManager::STREAM_RING;
            break;
        case AudioManagerNapi::MEDIA:
            result = AudioSystemManager::STREAM_MUSIC;
            break;
        case AudioManagerNapi::VOICE_ASSISTANT:
            result = AudioSystemManager::STREAM_VOICE_ASSISTANT;
            break;
        default:
            result = AudioSystemManager::STREAM_MUSIC;
            HiLog::Error(LABEL, "Unknown volume type, Set it to default MEDIA!");
            break;
    }

    return result;
}

static AudioRingerMode GetNativeAudioRingerMode(int32_t ringMode)
{
    AudioRingerMode result = RINGER_MODE_NORMAL;

    switch (ringMode) {
        case AudioManagerNapi::RINGER_MODE_SILENT:
            result = RINGER_MODE_SILENT;
            break;
        case AudioManagerNapi::RINGER_MODE_VIBRATE:
            result = RINGER_MODE_VIBRATE;
            break;
        case AudioManagerNapi::RINGER_MODE_NORMAL:
            result = RINGER_MODE_NORMAL;
            break;
        default:
            result = RINGER_MODE_NORMAL;
            HiLog::Error(LABEL, "Unknown ringer mode requested by JS, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

static AudioManagerNapi::AudioRingMode GetJsAudioRingMode(int32_t ringerMode)
{
    AudioManagerNapi::AudioRingMode result = AudioManagerNapi::RINGER_MODE_NORMAL;

    switch (ringerMode) {
        case RINGER_MODE_SILENT:
            result = AudioManagerNapi::RINGER_MODE_SILENT;
            break;
        case RINGER_MODE_VIBRATE:
            result = AudioManagerNapi::RINGER_MODE_VIBRATE;
            break;
        case RINGER_MODE_NORMAL:
            result = AudioManagerNapi::RINGER_MODE_NORMAL;
            break;
        default:
            result = AudioManagerNapi::RINGER_MODE_NORMAL;
            HiLog::Error(LABEL, "Unknown ringer mode returned from native, Set it to default RINGER_MODE_NORMAL!");
            break;
    }

    return result;
}

napi_status AudioManagerNapi::AddNamedProperty(napi_env env, napi_value object, const string name, int32_t enumValue)
{
    napi_status status;
    napi_value enumNapiValue;

    status = napi_create_int32(env, enumValue, &enumNapiValue);
    if (status == napi_ok) {
        status = napi_set_named_property(env, object, name.c_str(), enumNapiValue);
    }

    return status;
}

napi_value AudioManagerNapi::CreateInterruptTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptTypeMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateInterruptActionTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptActionTypeMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptActionType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptActionTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateInterruptHintObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;
    int32_t refCount = 1;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter: interruptHintMap) {
            propName = iter.first;
            status = AddNamedProperty(env, result, propName, iter.second);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop in CreateInterruptTypeObject!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &interruptHint_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateInterruptHintObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateAudioVolumeTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = AudioManagerNapi::RINGTONE; i <= AudioManagerNapi::VOICE_ASSISTANT; i++) {
            switch (i) {
                case AudioManagerNapi::RINGTONE:
                    propName = "RINGTONE";
                    break;
                case AudioManagerNapi::MEDIA:
                    propName = "MEDIA";
                    break;
                case AudioManagerNapi::VOICE_CALL:
                    propName = "VOICE_CALL";
                    break;
                case AudioManagerNapi::VOICE_ASSISTANT:
                    propName = "VOICE_ASSISTANT";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateAudioVolumeTypeObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &audioVolumeTypeRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateAudioVolumeTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateDeviceFlagObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = DEVICE_FLAG_NONE + 1; i < DEVICE_FLAG_MAX; i++) {
            switch (i) {
                case OUTPUT_DEVICES_FLAG:
                    propName = "OUTPUT_DEVICES_FLAG";
                    break;
                case INPUT_DEVICES_FLAG:
                    propName = "INPUT_DEVICES_FLAG";
                    break;
                case ALL_DEVICES_FLAG:
                    propName = "ALL_DEVICES_FLAG";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateDeviceFlagObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &deviceFlagRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateDeviceFlagObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateDeviceRoleObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = DEVICE_ROLE_NONE + 1; i < DEVICE_ROLE_MAX; i++) {
            switch (i) {
                case INPUT_DEVICE:
                    propName = "INPUT_DEVICE";
                    break;
                case OUTPUT_DEVICE:
                    propName = "OUTPUT_DEVICE";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateDeviceRoleObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &deviceRoleRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateDeviceRoleObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateDeviceTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = DEVICE_TYPE_NONE + 1; i < DEVICE_TYPE_MAX; i++) {
            switch (i) {
                case DEVICE_TYPE_INVALID:
                    propName = "INVALID";
                    break;
                case DEVICE_TYPE_EARPIECE:
                    propName = "EARPIECE";
                    break;
                case DEVICE_TYPE_SPEAKER:
                    propName = "SPEAKER";
                    break;
                case DEVICE_TYPE_WIRED_HEADSET:
                    propName = "WIRED_HEADSET";
                    break;
                case DEVICE_TYPE_BLUETOOTH_SCO:
                    propName = "BLUETOOTH_SCO";
                    break;
                case DEVICE_TYPE_BLUETOOTH_A2DP:
                    propName = "BLUETOOTH_A2DP";
                    break;
                case DEVICE_TYPE_MIC:
                    propName = "MIC";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateDeviceTypeObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &deviceTypeRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateDeviceTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateActiveDeviceTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = ACTIVE_DEVICE_TYPE_NONE + 1; i < ACTIVE_DEVICE_TYPE_MAX; i++) {
            switch (i) {
                case SPEAKER:
                    propName = "SPEAKER";
                    break;
                case BLUETOOTH_SCO:
                    propName = "BLUETOOTH_SCO";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateActiveDeviceTypeObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &activeDeviceTypeRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateActiveDeviceTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateAudioRingModeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    int32_t refCount = 1;
    string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (int i = AudioManagerNapi::RINGER_MODE_SILENT; i <= AudioManagerNapi::RINGER_MODE_NORMAL; i++) {
            switch (i) {
                case AudioManagerNapi::RINGER_MODE_SILENT:
                    propName = "RINGER_MODE_SILENT";
                    break;
                case AudioManagerNapi::RINGER_MODE_VIBRATE:
                    propName = "RINGER_MODE_VIBRATE";
                    break;
                case AudioManagerNapi::RINGER_MODE_NORMAL:
                    propName = "RINGER_MODE_NORMAL";
                    break;
                default:
                    HiLog::Error(LABEL, "CreateAudioRingModeObject: No prob with this value try next value!");
                    continue;
            }
            status = AddNamedProperty(env, result, propName, i);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, refCount, &audioRingModeRef_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    HiLog::Error(LABEL, "CreateAudioRingModeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::Init(napi_env env, napi_value exports)
{
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    const int32_t refCount = 1;

    napi_property_descriptor audio_svc_mngr_properties[] = {
        DECLARE_NAPI_FUNCTION("setVolume", SetVolume),
        DECLARE_NAPI_FUNCTION("getVolume", GetVolume),
        DECLARE_NAPI_FUNCTION("getMaxVolume", GetMaxVolume),
        DECLARE_NAPI_FUNCTION("getMinVolume", GetMinVolume),
        DECLARE_NAPI_FUNCTION("getDevices", GetDevices),
        DECLARE_NAPI_FUNCTION("mute", SetStreamMute),
        DECLARE_NAPI_FUNCTION("isMute", IsStreamMute),
        DECLARE_NAPI_FUNCTION("isActive", IsStreamActive),
        DECLARE_NAPI_FUNCTION("setRingerMode", SetRingerMode),
        DECLARE_NAPI_FUNCTION("getRingerMode", GetRingerMode),
        DECLARE_NAPI_FUNCTION("setDeviceActive", SetDeviceActive),
        DECLARE_NAPI_FUNCTION("isDeviceActive", IsDeviceActive),
        DECLARE_NAPI_FUNCTION("setAudioParameter", SetAudioParameter),
        DECLARE_NAPI_FUNCTION("getAudioParameter", GetAudioParameter),
        DECLARE_NAPI_FUNCTION("setMicrophoneMute", SetMicrophoneMute),
        DECLARE_NAPI_FUNCTION("isMicrophoneMute", IsMicrophoneMute),
        DECLARE_NAPI_FUNCTION("activateAudioInterrupt", ActivateAudioInterrupt),
        DECLARE_NAPI_FUNCTION("deactivateAudioInterrupt", DeactivateAudioInterrupt),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off)
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("getAudioManager", GetAudioManager),
        DECLARE_NAPI_PROPERTY("AudioVolumeType", CreateAudioVolumeTypeObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceFlag", CreateDeviceFlagObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceRole", CreateDeviceRoleObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceType", CreateDeviceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("ActiveDeviceType", CreateActiveDeviceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("AudioRingMode", CreateAudioRingModeObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptActionType", CreateInterruptActionTypeObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptHint", CreateInterruptHintObject(env)),
        DECLARE_NAPI_PROPERTY("InterruptType", CreateInterruptTypeObject(env))
    };

    status = napi_define_class(env, AUDIO_MNGR_NAPI_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct, nullptr,
        sizeof(audio_svc_mngr_properties) / sizeof(audio_svc_mngr_properties[PARAM0]),
        audio_svc_mngr_properties, &constructor);
    if (status == napi_ok) {
        status = napi_create_reference(env, constructor, refCount, &managerConstructor_);
        if (status == napi_ok) {
            status = napi_set_named_property(env, exports, AUDIO_MNGR_NAPI_CLASS_NAME.c_str(), constructor);
            if (status == napi_ok) {
                status = napi_define_properties(env, exports,
                                                sizeof(static_prop) / sizeof(static_prop[PARAM0]), static_prop);
                if (status == napi_ok) {
                    return exports;
                }
            }
        }
    }
    HiLog::Error(LABEL, "Failure in AudioManagerNapi::Init()");
    napi_get_undefined(env, &result);

    return result;
}

// Constructor callback
napi_value AudioManagerNapi::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis;
    napi_value  result = nullptr;
    size_t argCount = 0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        unique_ptr<AudioManagerNapi> obj = make_unique<AudioManagerNapi>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->audioMngr_ = AudioSystemManager::GetInstance();
            status = napi_wrap(env, jsThis, static_cast<void*>(obj.get()),
                               AudioManagerNapi::Destructor, nullptr, &(obj->wrapper_));
            if (status == napi_ok) {
                obj.release();
                return jsThis;
            }
        }
    }
    HiLog::Error(LABEL, "Failed in AudioManagerNapi::Construct()!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::CreateAudioManagerWrapper(napi_env env)
{
    napi_status status;
    napi_value result = nullptr;
    napi_value constructor;

    status = napi_get_reference_value(env, managerConstructor_, &constructor);
    if (status == napi_ok) {
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        if (status == napi_ok) {
            return result;
        }
    }
    HiLog::Error(LABEL, "Failed in AudioManagerNapi::CreateaudioMngrWrapper!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value AudioManagerNapi::GetAudioManager(napi_env env, napi_callback_info info)
{
    napi_status status;
    size_t argCount = 0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, nullptr, nullptr);
    if (status != napi_ok || argCount != 0) {
        HiLog::Error(LABEL, "Invalid arguments!");
        return nullptr;
    }

    return AudioManagerNapi::CreateAudioManagerWrapper(env);
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

static void CommonCallbackRoutine(napi_env env, AudioManagerAsyncContext* &asyncContext, const napi_value &valueParam)
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

static void SetFunctionAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioManagerAsyncContext*>(data);
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
    auto asyncContext = static_cast<AudioManagerAsyncContext*>(data);
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

static void GetStringValueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_string_utf8(env, asyncContext->valueStr.c_str(), NAPI_AUTO_LENGTH, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioManagerAsyncContext* is Null!");
    }
}

static void GetIntValueAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioManagerAsyncContext*>(data);
    napi_value valueParam = nullptr;

    if (asyncContext != nullptr) {
        if (!asyncContext->status) {
            napi_create_int32(env, asyncContext->intValue, &valueParam);
        }
        CommonCallbackRoutine(env, asyncContext, valueParam);
    } else {
        HiLog::Error(LABEL, "ERROR: AudioManagerAsyncContext* is Null!");
    }
}

napi_value AudioManagerNapi::SetMicrophoneMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_boolean) {
                napi_get_value_bool(env, argv[i], &asyncContext->isMute);
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
        napi_create_string_utf8(env, "SetMicrophoneMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->status = context->objectInfo->audioMngr_->SetMicrophoneMute(context->isMute);
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::IsMicrophoneMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

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
        napi_create_string_utf8(env, "IsMicrophoneMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->isMute = context->objectInfo->audioMngr_->IsMicrophoneMute();
                context->isTrue = context->isMute;
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

napi_value AudioManagerNapi::SetRingerMode(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->ringMode);
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
        napi_create_string_utf8(env, "SetRingerMode", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->status =
                    context->objectInfo->audioMngr_->SetRingerMode(GetNativeAudioRingerMode(context->ringMode));
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::GetRingerMode(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

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
        napi_create_string_utf8(env, "GetRingerMode", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->ringMode = GetJsAudioRingMode(context->objectInfo->audioMngr_->GetRingerMode());
                context->intValue = context->ringMode;
                context->status = 0;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::SetStreamMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);
    NAPI_ASSERT(env, argc >= ARGS_TWO, "requires 2 parameters minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
            } else if (i == PARAM1 && valueType == napi_boolean) {
                napi_get_value_bool(env, argv[i], &asyncContext->isMute);
            } else if (i == PARAM2 && valueType == napi_function) {
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
        napi_create_string_utf8(env, "SetStreamMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->status = context->objectInfo->audioMngr_->SetMute(GetNativeAudioVolumeType(context->volType),
                                                                           context->isMute);
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::IsStreamMute(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
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
        napi_create_string_utf8(env, "IsStreamMute", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->isMute =
                    context->objectInfo->audioMngr_->IsStreamMute(GetNativeAudioVolumeType(context->volType));
                context->isTrue = context->isMute;
                context->status = 0;
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

napi_value AudioManagerNapi::IsStreamActive(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
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
        napi_create_string_utf8(env, "IsStreamActive", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->isActive =
                    context->objectInfo->audioMngr_->IsStreamActive(GetNativeAudioVolumeType(context->volType));
                context->isTrue = context->isActive;
                context->status = 0;
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

napi_value AudioManagerNapi::SetDeviceActive(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);
    NAPI_ASSERT(env, argc >= ARGS_TWO, "requires 2 parameters minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->deviceType);
            } else if (i == PARAM1 && valueType == napi_boolean) {
                napi_get_value_bool(env, argv[i], &asyncContext->isActive);
            } else if (i == PARAM2 && valueType == napi_function) {
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
        napi_create_string_utf8(env, "SetDeviceActive", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->status = context->objectInfo->audioMngr_->SetDeviceActive(
                    static_cast<ActiveDeviceType>(context->deviceType), context->isActive);
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::IsDeviceActive(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->deviceType);
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
        napi_create_string_utf8(env, "IsDeviceActive", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->isActive =
                    context->objectInfo->audioMngr_->IsDeviceActive(static_cast<ActiveDeviceType>(context->deviceType));
                context->isTrue = context->isActive;
                context->status = 0;
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

napi_value AudioManagerNapi::SetAudioParameter(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);
    NAPI_ASSERT(env, argc >= ARGS_TWO, "requires 2 parameters minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_string) {
                asyncContext->key = GetStringArgument(env, argv[i]);
            } else if (i == PARAM1 && valueType == napi_string) {
                asyncContext->valueStr = GetStringArgument(env, argv[i]);
            } else if (i == PARAM2 && valueType == napi_function) {
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
        napi_create_string_utf8(env, "SetAudioParameter", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->objectInfo->audioMngr_->SetAudioParameter(context->key, context->valueStr);
                context->status = 0;
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::GetAudioParameter(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_string) {
                asyncContext->key = GetStringArgument(env, argv[i]);
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
        napi_create_string_utf8(env, "GetAudioParameter", NAPI_AUTO_LENGTH, &resource);
        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->valueStr = context->objectInfo->audioMngr_->GetAudioParameter(context->key);
                context->status = 0;
            },
            GetStringValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::SetVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_THREE);
    NAPI_ASSERT(env, argc >= ARGS_TWO, "requires 2 parameters minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
            } else if (i == PARAM1 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volLevel);
            } else if (i == PARAM2 && valueType == napi_function) {
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
        napi_create_string_utf8(env, "SetVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->status = context->objectInfo->audioMngr_->SetVolume(GetNativeAudioVolumeType(context->volType),
                                                                             context->volLevel);
            },
            SetFunctionAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::GetVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
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
        napi_create_string_utf8(env, "GetVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->volLevel = context->objectInfo->audioMngr_->GetVolume(
                    GetNativeAudioVolumeType(context->volType));
                context->intValue = context->volLevel;
                context->status = 0;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::GetMaxVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
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
        napi_create_string_utf8(env, "GetMaxVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->volLevel = context->objectInfo->audioMngr_->GetMaxVolume(
                    GetNativeAudioVolumeType(context->volType));
                context->intValue = context->volLevel;
                context->status = 0;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value AudioManagerNapi::GetMinVolume(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->volType);
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
        napi_create_string_utf8(env, "GetMinVolume", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->volLevel = context->objectInfo->audioMngr_->GetMinVolume(
                    GetNativeAudioVolumeType(context->volType));
                context->intValue = context->volLevel;
                context->status = 0;
            },
            GetIntValueAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

static void GetDevicesAsyncCallbackComplete(napi_env env, napi_status status, void *data)
{
    auto asyncContext = static_cast<AudioManagerAsyncContext*>(data);
    napi_value result[ARGS_TWO] = {0};
    napi_value ddWrapper = nullptr;
    napi_value retVal;
    int32_t size = asyncContext->deviceDescriptors.size();

    napi_create_array_with_length(env, size, &result[PARAM1]);

    for (int i = 0; i < size; i += 1) {
        AudioDeviceDescriptor *curDeviceDescriptor = asyncContext->deviceDescriptors[i];
        ddWrapper = AudioDeviceDescriptorNapi::CreateAudioDeviceDescriptorWrapper(env, curDeviceDescriptor);
        napi_set_element(env, result[PARAM1], i, ddWrapper);
    }

    napi_get_undefined(env, &result[PARAM0]);

    if (asyncContext->deferred) {
        napi_resolve_deferred(env, asyncContext->deferred, result[PARAM1]);
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, ARGS_TWO, result, &retVal);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);

    delete asyncContext;
}

napi_value AudioManagerNapi::GetDevices(napi_env env, napi_callback_info info)
{
    napi_status status;
    const int32_t refCount = 1;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_TWO);
    NAPI_ASSERT(env, argc >= ARGS_ONE, "requires 1 parameter minimum");

    unique_ptr<AudioManagerAsyncContext> asyncContext = make_unique<AudioManagerAsyncContext>();

    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->objectInfo));
    if (status == napi_ok && asyncContext->objectInfo != nullptr) {
        for (size_t i = PARAM0; i < argc; i++) {
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, argv[i], &valueType);

            if (i == PARAM0 && valueType == napi_number) {
                napi_get_value_int32(env, argv[i], &asyncContext->deviceFlag);
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
        napi_create_string_utf8(env, "GetDevices", NAPI_AUTO_LENGTH, &resource);

        status = napi_create_async_work(
            env, nullptr, resource,
            [](napi_env env, void *data) {
                auto context = static_cast<AudioManagerAsyncContext*>(data);
                context->deviceDescriptors = context->objectInfo->audioMngr_->GetDevices(
                    static_cast<DeviceFlag>(context->deviceFlag));
                context->status = 0;
            },
            GetDevicesAsyncCallbackComplete, static_cast<void*>(asyncContext.get()), &asyncContext->work);
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

napi_value UndefinedNapiValue(const napi_env& env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JsObjectToInt(const napi_env& env, const napi_value& object, std::string fieldStr, int32_t& fieldRef)
{
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, fieldStr.c_str(), &hasProperty));
    if (hasProperty) {
        napi_value field;
        napi_valuetype valueType = napi_undefined;

        napi_get_named_property(env, object, fieldStr.c_str(), &field);
        NAPI_CALL(env, napi_typeof(env, field, &valueType));
        NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. Number expected.");
        napi_get_value_int32(env, field, &fieldRef);
    } else {
        MEDIA_ERR_LOG("audio manager napi js to int no property: %{public}s", fieldStr.c_str());
    }

    return UndefinedNapiValue(env);
}


static void JsObjToAudioInterrupt(const napi_env& env, const napi_value& object, AudioInterrupt& audioInterrupt)
{
    int32_t propValue = -1;
    int32_t musicSession = 1000;
    int32_t voiceSession = 1001;

    JsObjectToInt(env, object, "streamType", propValue);
    audioInterrupt.streamType = static_cast<AudioStreamType>(GetNativeAudioVolumeType(propValue));
    // For now assign fixed values, these are not used currently
    if (audioInterrupt.streamType == STREAM_MUSIC) {
        audioInterrupt.streamUsage = STREAM_USAGE_MEDIA;
        audioInterrupt.contentType = CONTENT_TYPE_MUSIC;
        audioInterrupt.sessionID = musicSession;
    } else if (audioInterrupt.streamType == STREAM_VOICE_ASSISTANT) {
        audioInterrupt.streamUsage = STREAM_USAGE_VOICE_ASSISTANT;
        audioInterrupt.contentType = CONTENT_TYPE_SPEECH;
        audioInterrupt.sessionID = voiceSession;
    }
}

napi_value AudioManagerNapi::ActivateAudioInterrupt(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);
    NAPI_ASSERT(env, argc == ARGS_ONE, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    AudioManagerNapi *managerNapi = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&managerNapi));
    NAPI_ASSERT(env, status == napi_ok && managerNapi != nullptr, "Failed to retrieve audio manager napi instance.");

    AudioInterrupt audioInterrupt;
    JsObjToAudioInterrupt(env, argv[0], audioInterrupt);

    int32_t ret = managerNapi->audioMngr_->ActivateAudioInterrupt(audioInterrupt);
    napi_get_boolean(env, ret == SUCCESS, &result);
    return result;
}

napi_value AudioManagerNapi::DeactivateAudioInterrupt(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;

    GET_PARAMS(env, info, ARGS_ONE);
    NAPI_ASSERT(env, argc == ARGS_ONE, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    AudioManagerNapi *managerNapi = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&managerNapi));
    NAPI_ASSERT(env, status == napi_ok && managerNapi != nullptr, "Failed to retrieve audio manager napi instance.");
    NAPI_ASSERT(env, managerNapi->audioMngr_ != nullptr, "audio system manager instance is null.");

    AudioInterrupt audioInterrupt;
    JsObjToAudioInterrupt(env, argv[0], audioInterrupt);

    int32_t ret = managerNapi->audioMngr_->DeactivateAudioInterrupt(audioInterrupt);
    napi_get_boolean(env, ret == SUCCESS, &result);
    return result;
}

napi_value AudioManagerNapi::On(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = 3;
    size_t argCount = minArgCount;
    napi_value args[minArgCount] = { nullptr, nullptr, nullptr };
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[PARAM0] == nullptr || args[PARAM1] == nullptr ||
        args[PARAM2] == nullptr) {
        HiLog::Error(LABEL, "On fail to napi_get_cb_info");
        return undefinedResult;
    }

    AudioManagerNapi *managerNapi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&managerNapi));
    NAPI_ASSERT(env, status == napi_ok && managerNapi != nullptr, "Failed to retrieve audio manager napi instance.");
    NAPI_ASSERT(env, managerNapi->audioMngr_ != nullptr, "audio system manager instance is null.");

    napi_valuetype valueType0 = napi_undefined;
    napi_valuetype valueType1 = napi_undefined;
    napi_valuetype valueType2 = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &valueType0) != napi_ok || valueType0 != napi_string ||
        napi_typeof(env, args[PARAM1], &valueType1) != napi_ok || valueType1 != napi_number ||
        napi_typeof(env, args[PARAM2], &valueType2) != napi_ok || valueType2 != napi_function) {
        return undefinedResult;
    }

    int32_t streamType = -1;
    status = napi_get_value_int32(env, args[1], &streamType);
    if (status != napi_ok) {
        return undefinedResult;
    }

    if (managerNapi->callbackNapi_ == nullptr) {
        managerNapi->callbackNapi_ = std::make_shared<AudioManagerCallbackNapi>(env);
        int32_t ret = managerNapi->audioMngr_->SetAudioManagerCallback(
            GetNativeAudioVolumeType(streamType), managerNapi->callbackNapi_);
        if (ret) {
            MEDIA_ERR_LOG("AudioManagerNapi: SetAudioManagerCallback Failed");
            return undefinedResult;
        } else {
            MEDIA_DEBUG_LOG("AudioManagerNapi: SetAudioManagerCallback Success");
        }
    }

    std::string callbackName = GetStringArgument(env, args[0]);
    MEDIA_DEBUG_LOG("AudioManagerNapi: callbackName: %{public}s", callbackName.c_str());
    std::shared_ptr<AudioManagerCallbackNapi> cb =
        std::static_pointer_cast<AudioManagerCallbackNapi>(managerNapi->callbackNapi_);
    cb->SaveCallbackReference(callbackName, args[PARAM2]);

    return undefinedResult;
}

napi_value AudioManagerNapi::Off(napi_env env, napi_callback_info info)
{
    napi_value undefinedResult = nullptr;
    napi_get_undefined(env, &undefinedResult);

    const size_t minArgCount = 2;
    size_t argCount = minArgCount;
    napi_value args[minArgCount] = {nullptr, nullptr};
    napi_value jsThis = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argCount, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[PARAM0] == nullptr || args[PARAM1] == nullptr) {
        HiLog::Error(LABEL, "On fail to napi_get_cb_info");
        return undefinedResult;
    }

    AudioManagerNapi *managerNapi = nullptr;
    status = napi_unwrap(env, jsThis, reinterpret_cast<void **>(&managerNapi));
    NAPI_ASSERT(env, status == napi_ok && managerNapi != nullptr, "Failed to retrieve audio manager napi instance.");
    NAPI_ASSERT(env, managerNapi->audioMngr_ != nullptr, "audio system manager instance is null.");

    napi_valuetype valueType0 = napi_undefined;
    napi_valuetype valueType1 = napi_undefined;
    if (napi_typeof(env, args[PARAM0], &valueType0) != napi_ok || valueType0 != napi_string ||
        napi_typeof(env, args[PARAM1], &valueType1) != napi_ok || valueType1 != napi_number) {
        MEDIA_ERR_LOG("AudioManagerNapi: Off: args type mismatch");
        return undefinedResult;
    }

    int32_t streamType = -1;
    status = napi_get_value_int32(env, args[1], &streamType);
    if (status != napi_ok) {
        MEDIA_ERR_LOG("AudioManagerNapi: Off: get streamType value failed");
        return undefinedResult;
    }

    int32_t ret = managerNapi->audioMngr_->UnsetAudioManagerCallback(GetNativeAudioVolumeType(streamType));
    if (ret) {
        MEDIA_ERR_LOG("AudioManagerNapi: UnsetAudioManagerCallback Failed");
        return undefinedResult;
    } else {
        HiLog::Error(LABEL, "UnsetAudioManagerCallback Success NAPI");
        MEDIA_DEBUG_LOG("AudioManagerNapi: UnsetAudioManagerCallback Success");
    }
    managerNapi->callbackNapi_ = nullptr;

    return undefinedResult;
}

static napi_value Init(napi_env env, napi_value exports)
{
    AudioManagerNapi::Init(env, exports);
    AudioDeviceDescriptorNapi::Init(env, exports);
    AudioCapturerNapi::Init(env, exports);
    AudioRendererNapi::Init(env, exports);
    AudioParametersNapi::Init(env, exports);
    AudioSoundManagerNapi::Init(env, exports);

    return exports;
}

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "multimedia.audio",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
} // namespace AudioStandard
} // namespace OHOS
