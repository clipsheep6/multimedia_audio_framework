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

#include "hilog/log.h"
#include "napi_audio_enum.h"

using namespace std;
using OHOS::HiviewDFX::HiLog;
using OHOS::HiviewDFX::HiLogLabel;

namespace OHOS {
namespace AudioStandard {

napi_ref NapiAudioEnum::sConstructor_ = nullptr;
unique_ptr<AudioParameters> NapiAudioEnum::sAudioParameters_ = nullptr;

napi_ref NapiAudioEnum::audioChannel_ = nullptr;
napi_ref NapiAudioEnum::samplingRate_ = nullptr;
napi_ref NapiAudioEnum::encodingType_ = nullptr;
napi_ref NapiAudioEnum::contentType_ = nullptr;
napi_ref NapiAudioEnum::streamUsage_ = nullptr;
napi_ref NapiAudioEnum::deviceRole_ = nullptr;
napi_ref NapiAudioEnum::deviceType_ = nullptr;
napi_ref NapiAudioEnum::sourceType_ = nullptr;

static const std::string NAPI_AUDIO_ENUM_CLASS_NAME = "AudioEnum";

const std::map<std::string, AudioChannel> NapiAudioEnum::audioChannelMap = {
    {"CHANNEL_1", MONO},
    {"CHANNEL_2", STEREO}
};

const std::map<std::string, AudioSamplingRate> NapiAudioEnum::samplingRateMap = {
    {"SAMPLE_RATE_8000", SAMPLE_RATE_8000},
    {"SAMPLE_RATE_11025", SAMPLE_RATE_11025},
    {"SAMPLE_RATE_12000", SAMPLE_RATE_12000},
    {"SAMPLE_RATE_16000", SAMPLE_RATE_16000},
    {"SAMPLE_RATE_22050", SAMPLE_RATE_22050},
    {"SAMPLE_RATE_24000", SAMPLE_RATE_24000},
    {"SAMPLE_RATE_32000", SAMPLE_RATE_32000},
    {"SAMPLE_RATE_44100", SAMPLE_RATE_44100},
    {"SAMPLE_RATE_48000", SAMPLE_RATE_48000},
    {"SAMPLE_RATE_64000", SAMPLE_RATE_64000},
    {"SAMPLE_RATE_96000", SAMPLE_RATE_96000}
};

const std::map<std::string, AudioEncodingType> NapiAudioEnum::encodingTypeMap = {
    {"ENCODING_TYPE_INVALID", ENCODING_INVALID},
    {"ENCODING_TYPE_RAW", ENCODING_PCM}
};

const std::map<std::string, ContentType> NapiAudioEnum::contentTypeMap = {
    {"CONTENT_TYPE_UNKNOWN", CONTENT_TYPE_UNKNOWN},
    {"CONTENT_TYPE_SPEECH", CONTENT_TYPE_SPEECH},
    {"CONTENT_TYPE_MUSIC", CONTENT_TYPE_MUSIC},
    {"CONTENT_TYPE_MOVIE", CONTENT_TYPE_MOVIE},
    {"CONTENT_TYPE_SONIFICATION", CONTENT_TYPE_SONIFICATION},
    {"CONTENT_TYPE_RINGTONE", CONTENT_TYPE_RINGTONE},
    {"CONTENT_TYPE_ULTRASONIC", CONTENT_TYPE_ULTRASONIC}
};

const std::map<std::string, StreamUsage> NapiAudioEnum::streamUsageMap = {
    {"STREAM_USAGE_UNKNOWN", STREAM_USAGE_UNKNOWN},
    {"STREAM_USAGE_MEDIA", STREAM_USAGE_MEDIA},
    {"STREAM_USAGE_MUSIC", STREAM_USAGE_MUSIC},
    {"STREAM_USAGE_VOICE_COMMUNICATION", STREAM_USAGE_VOICE_COMMUNICATION},
    {"STREAM_USAGE_VOICE_ASSISTANT", STREAM_USAGE_VOICE_ASSISTANT},
    {"STREAM_USAGE_ALARM", STREAM_USAGE_ALARM},
    {"STREAM_USAGE_VOICE_MESSAGE", STREAM_USAGE_VOICE_MESSAGE},
    {"STREAM_USAGE_NOTIFICATION_RINGTONE", STREAM_USAGE_NOTIFICATION_RINGTONE},
    {"STREAM_USAGE_RINGTONE", STREAM_USAGE_RINGTONE},
    {"STREAM_USAGE_NOTIFICATION", STREAM_USAGE_NOTIFICATION},
    {"STREAM_USAGE_ACCESSIBILITY", STREAM_USAGE_ACCESSIBILITY},
    {"STREAM_USAGE_SYSTEM", STREAM_USAGE_SYSTEM},
    {"STREAM_USAGE_MOVIE", STREAM_USAGE_MOVIE},
    {"STREAM_USAGE_GAME", STREAM_USAGE_GAME},
    {"STREAM_USAGE_AUDIOBOOK", STREAM_USAGE_AUDIOBOOK},
    {"STREAM_USAGE_NAVIGATION", STREAM_USAGE_NAVIGATION},
    {"STREAM_USAGE_DTMF", STREAM_USAGE_DTMF},
    {"STREAM_USAGE_ENFORCED_TONE", STREAM_USAGE_ENFORCED_TONE},
    {"STREAM_USAGE_ULTRASONIC", STREAM_USAGE_ULTRASONIC}
};

const std::map<std::string, DeviceRole> NapiAudioEnum::deviceRoleMap = {
    {"DEVICE_ROLE_NONE", DEVICE_ROLE_NONE},
    {"INPUT_DEVICE", INPUT_DEVICE},
    {"OUTPUT_DEVICE", OUTPUT_DEVICE},
    {"DEVICE_ROLE_MAX", DEVICE_ROLE_MAX}
};

const std::map<std::string, DeviceType> NapiAudioEnum::deviceTypeMap = {
    {"NONE", DEVICE_TYPE_NONE},
    {"INVALID", DEVICE_TYPE_INVALID},
    {"EARPIECE", DEVICE_TYPE_EARPIECE},
    {"SPEAKER", DEVICE_TYPE_SPEAKER},
    {"WIRED_HEADSET", DEVICE_TYPE_WIRED_HEADSET},
    {"WIRED_HEADPHONES", DEVICE_TYPE_WIRED_HEADPHONES},
    {"BLUETOOTH_SCO", DEVICE_TYPE_BLUETOOTH_SCO},
    {"BLUETOOTH_A2DP", DEVICE_TYPE_BLUETOOTH_A2DP},
    {"MIC", DEVICE_TYPE_MIC},
    {"WAKEUP", DEVICE_TYPE_WAKEUP},
    {"USB_HEADSET", DEVICE_TYPE_USB_HEADSET},
    {"DEFAULT", DEVICE_TYPE_DEFAULT},
    {"MAX", DEVICE_TYPE_MAX},
};

const std::map<std::string, SourceType> NapiAudioEnum::sourceTypeMap = {
    {"SOURCE_TYPE_INVALID", SOURCE_TYPE_INVALID},
    {"SOURCE_TYPE_MIC", SOURCE_TYPE_MIC},
    {"SOURCE_TYPE_WAKEUP", SOURCE_TYPE_WAKEUP},
    {"SOURCE_TYPE_VOICE_RECOGNITION", SOURCE_TYPE_VOICE_RECOGNITION},
    {"SOURCE_TYPE_PLAYBACK_CAPTURE", SOURCE_TYPE_PLAYBACK_CAPTURE},
    {"SOURCE_TYPE_VOICE_COMMUNICATION", SOURCE_TYPE_VOICE_COMMUNICATION}
};

const std::map<std::string, VolumeAdjustType> NapiAudioEnum::volumeAdjustTypeMap = {
    {"VOLUME_UP", VOLUME_UP},
    {"VOLUME_DOWN", VOLUME_DOWN},
};

const std::map<std::string, ChannelBlendMode> NapiAudioEnum::channelBlendModeMap = {
    {"MODE_DEFAULT", MODE_DEFAULT},
    {"MODE_BLEND_LR", MODE_BLEND_LR},
    {"MODE_ALL_LEFT", MODE_ALL_LEFT},
    {"MODE_ALL_RIGHT", MODE_ALL_RIGHT},
};

NapiAudioEnum::NapiAudioEnum()
    : env_(nullptr) {
}

NapiAudioEnum::~NapiAudioEnum()
{
    audioParameters_ = nullptr;
}

void NapiAudioEnum::Destructor(napi_env env, void *nativeObject, void *finalizeHint)
{
    if (nativeObject != nullptr) {
        auto obj = static_cast<NapiAudioEnum *>(nativeObject);
        delete obj;
    }
}

napi_value NapiAudioEnum::CreateAudioChannelObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : audioChannelMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &audioChannel_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateAudioChannelObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateSamplingRateObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : samplingRateMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &samplingRate_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateSamplingRateObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateEncodingTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : encodingTypeMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &encodingType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateEncodingTypeObject is failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateContentTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : contentTypeMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &contentType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateContentTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateStreamUsageObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : streamUsageMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &streamUsage_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateStreamUsageObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateDeviceRoleObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : deviceRoleMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &deviceRole_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateDeviceRoleObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateDeviceTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : deviceTypeMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop!");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &deviceType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateDeviceRoleObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateSourceTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : sourceTypeMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop! in CreateSourceTypeObject");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &sourceType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateSourceTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateVolumeAdjustTypeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (auto &iter : volumeAdjustTypeMap) {
            propName = iter.first;
            status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
            if (status != napi_ok) {
                AUDIO_ERR_LOG("Failed to add named prop! in CreateVolumeAdjustTypeObject");
                break;
            }
            propName.clear();
        }
        if (status == napi_ok) {
            status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &sourceType_);
            if (status == napi_ok) {
                return result;
            }
        }
    }
    AUDIO_ERR_LOG("CreateVolumeAdjustTypeObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value NapiAudioEnum::CreateChannelBlendModeObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status != napi_ok) {
        AUDIO_ERR_LOG("CreateChannelBlendModeObject failed!");
        napi_get_undefined(env, &result);
        return result;
    }

    for (auto &iter : channelBlendModeMap) {
        propName = iter.first;
        status = NapiParamUtils::SetValueInt32(env, propName, iter.second, result);
        if (status != napi_ok) {
            AUDIO_ERR_LOG("Failed to add named prop! in CreateChannelBlendModeObject");
            break;
        }
        propName.clear();
    }

    if (status == napi_ok) {
        status = napi_create_reference(env, result, REFERENCE_CREATION_COUNT, &sourceType_);
        if (status == napi_ok) {
            return result;
        }
    }

    return result;
}

napi_value NapiAudioEnum::Init(napi_env env, napi_value exports)
{
    AUDIO_INFO_LOG("NapiAudioEnum::Init()");
    napi_status status;
    napi_value constructor;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_property_descriptor audio_parameters_properties[] = {
        DECLARE_NAPI_GETTER_SETTER("format", GetAudioSampleFormat, SetAudioSampleFormat),
        DECLARE_NAPI_GETTER_SETTER("channels", GetAudioChannel, SetAudioChannel),
        DECLARE_NAPI_GETTER_SETTER("samplingRate", GetAudioSamplingRate, SetAudioSamplingRate),
        DECLARE_NAPI_GETTER_SETTER("encoding", GetAudioEncodingType, SetAudioEncodingType),
        DECLARE_NAPI_GETTER_SETTER("contentType", GetContentType, SetContentType),
        DECLARE_NAPI_GETTER_SETTER("usage", GetStreamUsage, SetStreamUsage),
        DECLARE_NAPI_GETTER_SETTER("deviceRole", GetDeviceRole, SetDeviceRole),
        DECLARE_NAPI_GETTER_SETTER("deviceType", GetDeviceType, SetDeviceType)
    };

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_PROPERTY("AudioChannel", CreateAudioChannelObject(env)),
        DECLARE_NAPI_PROPERTY("AudioSamplingRate", CreateSamplingRateObject(env)),
        DECLARE_NAPI_PROPERTY("AudioEncodingType", CreateEncodingTypeObject(env)),
        DECLARE_NAPI_PROPERTY("ContentType", CreateContentTypeObject(env)),
        DECLARE_NAPI_PROPERTY("StreamUsage", CreateStreamUsageObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceRole", CreateDeviceRoleObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceType", CreateDeviceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("SourceType", CreateSourceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("VolumeAdjustType", CreateVolumeAdjustTypeObject(env)),
        DECLARE_NAPI_PROPERTY("ChannelBlendMode", CreateChannelBlendModeObject(env))
    };

    status = napi_define_class(env, NAPI_AUDIO_ENUM_CLASS_NAME.c_str(), NAPI_AUTO_LENGTH, Construct,
                               nullptr, sizeof(audio_parameters_properties) / sizeof(audio_parameters_properties[0]),
                               audio_parameters_properties, &constructor);
    if (status != napi_ok) {
        return result;
    }

    status = napi_create_reference(env, constructor, REFERENCE_CREATION_COUNT, &sConstructor_);
    if (status == napi_ok) {
        status = napi_set_named_property(env, exports, NAPI_AUDIO_ENUM_CLASS_NAME.c_str(),
                                         constructor);
        if (status == napi_ok) {
            status = napi_define_properties(env, exports,
                                            sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
            if (status == napi_ok) {
                return exports;
            }
        }
    }
    AUDIO_ERR_LOG("Failure in NapiAudioEnum::Init()");

    return result;
}

napi_value NapiAudioEnum::Construct(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value jsThis = nullptr;
    size_t argCount = 0;

    status = napi_get_cb_info(env, info, &argCount, nullptr, &jsThis, nullptr);
    if (status == napi_ok) {
        unique_ptr<NapiAudioEnum> obj = make_unique<NapiAudioEnum>();
        if (obj != nullptr) {
            obj->env_ = env;
            obj->audioParameters_ = move(sAudioParameters_);
            status = napi_wrap(env, jsThis, static_cast<void*>(obj.get()),
                               NapiAudioEnum::Destructor, nullptr, nullptr);
            if (status == napi_ok) {
                obj.release();
                return jsThis;
            }
        }
    }
    AUDIO_ERR_LOG("Failed in NapiAudioEnum::Construct()!");
    napi_get_undefined(env, &jsThis);

    return jsThis;
}

napi_value NapiAudioEnum::GetAudioSampleFormat(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    AudioSampleFormat audioSampleFormat;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get audio sample format fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        if (!((napiAudioEnum != nullptr) && (napiAudioEnum->audioParameters_ != nullptr))) {
            AUDIO_ERR_LOG("Get audio sample format fail to napi_unwrap");
            return jsResult;
        }
        audioSampleFormat = napiAudioEnum->audioParameters_->format;
        status = napi_create_int32(env, audioSampleFormat, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioSampleFormat(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t audioSampleFormat;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set sample format fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set sample format fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &audioSampleFormat);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->format = static_cast<AudioSampleFormat>(audioSampleFormat);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioChannel(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    AudioChannel audioChannel;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get audio channels fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        audioChannel = napiAudioEnum->audioParameters_->channels;
        status = napi_create_int32(env, audioChannel, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioChannel(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t audioChannel;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set audio channel fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set audio channel fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &audioChannel);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->channels = static_cast<AudioChannel>(audioChannel);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioSamplingRate(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    AudioSamplingRate samplingRate;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get sampling rate fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        samplingRate = napiAudioEnum->audioParameters_->samplingRate;
        status = napi_create_int32(env, samplingRate, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioSamplingRate(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t samplingRate;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set sampling rate fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set sampling rate fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &samplingRate);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->samplingRate = static_cast<AudioSamplingRate>(samplingRate);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetAudioEncodingType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    AudioEncodingType encodingType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get encoding type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        encodingType = napiAudioEnum->audioParameters_->encoding;
        status = napi_create_int32(env, encodingType, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetAudioEncodingType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t encodingType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set audio encoding type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set audio encoding type fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &encodingType);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->encoding = static_cast<AudioEncodingType>(encodingType);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetContentType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    ContentType contentType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get content type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        contentType = napiAudioEnum->audioParameters_->contentType;
        status = napi_create_int32(env, contentType, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetContentType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t contentType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set content type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set content type fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &contentType);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->contentType = static_cast<ContentType>(contentType);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetStreamUsage(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    StreamUsage usage;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get stream usage fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        usage = napiAudioEnum->audioParameters_->usage;
        status = napi_create_int32(env, usage, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetStreamUsage(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t usage;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set stream usage fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set stream usage fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &usage);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->usage = static_cast<StreamUsage>(usage);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetDeviceRole(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    DeviceRole deviceRole;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get device role fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        deviceRole = napiAudioEnum->audioParameters_->deviceRole;
        status = napi_create_int32(env, deviceRole, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetDeviceRole(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t deviceRole;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr || args[0] == nullptr) {
        AUDIO_ERR_LOG("set device role fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set device role fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &deviceRole);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->deviceRole = static_cast<DeviceRole>(deviceRole);
    }

    return jsResult;
}

napi_value NapiAudioEnum::GetDeviceType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 0;
    napi_value jsThis = nullptr;
    DeviceType deviceType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, nullptr, &jsThis, nullptr);
    if (status != napi_ok || jsThis == nullptr) {
        AUDIO_ERR_LOG("Get device type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        deviceType = napiAudioEnum->audioParameters_->deviceType;
        AUDIO_INFO_LOG("get device type: %d", deviceType);
        status = napi_create_int32(env, deviceType, &jsResult);
        if (status == napi_ok) {
            return jsResult;
        }
    }

    return jsResult;
}

napi_value NapiAudioEnum::SetDeviceType(napi_env env, napi_callback_info info)
{
    napi_status status;
    NapiAudioEnum *napiAudioEnum = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value jsThis = nullptr;
    int32_t deviceType;
    napi_value jsResult = nullptr;
    napi_get_undefined(env, &jsResult);

    status = napi_get_cb_info(env, info, &argc, args, &jsThis, nullptr);
    if ((status != napi_ok) || (jsThis == nullptr) || (args[0] == nullptr)) {
        AUDIO_ERR_LOG("set device type fail to napi_get_cb_info");
        return jsResult;
    }

    status = napi_unwrap(env, jsThis, (void **)&napiAudioEnum);
    if (status == napi_ok) {
        napi_valuetype valueType = napi_undefined;
        if (napi_typeof(env, args[0], &valueType) != napi_ok || valueType != napi_number) {
            AUDIO_ERR_LOG("set device type fail: wrong data type");
            return jsResult;
        }
    }

    status = napi_get_value_int32(env, args[0], &deviceType);
    if (status == napi_ok) {
        napiAudioEnum->audioParameters_->deviceType = static_cast<DeviceType>(deviceType);
        AUDIO_INFO_LOG("set device type: %d", napiAudioEnum->audioParameters_->deviceType);
    }

    return jsResult;
}
}  // namespace AudioStandard
}  // namespace OHOS
