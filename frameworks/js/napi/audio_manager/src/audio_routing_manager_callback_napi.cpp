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

#include "audio_routing_manager_callback_napi.h"

#include <uv.h>

#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

AudioPreferredOutputDeviceChangeCallbackNapi::AudioPreferredOutputDeviceChangeCallbackNapi(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("AudioPreferredOutputDeviceChangeCallbackNapi: instance create");
}

AudioPreferredOutputDeviceChangeCallbackNapi::~AudioPreferredOutputDeviceChangeCallbackNapi()
{
    AUDIO_DEBUG_LOG("AudioPreferredOutputDeviceChangeCallbackNapi: instance destroy");
}

void AudioPreferredOutputDeviceChangeCallbackNapi::SaveCallbackReference(AudioStreamType streamType,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = 1;

    bool isSameCallback = true;
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        isSameCallback = AudioCommonNapi::IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    preferredOutputDeviceCbList_.push_back({cb, streamType});
    AUDIO_INFO_LOG("Save callback reference success, prefer ouput device callback list size [%{public}zu]",
        preferredOutputDeviceCbList_.size());
}

void AudioPreferredOutputDeviceChangeCallbackNapi::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (callback == nullptr) {
        AUDIO_INFO_LOG("RemoveCallbackReference: js callback is nullptr, remove all callback reference");
        RemoveAllCallbacks();
        return;
    }
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        bool isSameCallback = AudioCommonNapi::IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCallbackReference: find js callback, delete it");
            napi_status status = napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            CHECK_AND_RETURN_LOG(status == napi_ok, "RemoveCallbackReference: delete reference for callback fail");
            preferredOutputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void AudioPreferredOutputDeviceChangeCallbackNapi::RemoveAllCallbacks()
{
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    preferredOutputDeviceCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

static void SetValueInt32(const napi_env& env, const std::string& fieldStr, const int intValue, napi_value& result)
{
    napi_value value = nullptr;
    napi_create_int32(env, intValue, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

static void SetValueString(const napi_env& env, const std::string& fieldStr, const std::string& stringValue,
    napi_value& result)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, stringValue.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, result, fieldStr.c_str(), value);
}

static void NativeDeviceDescToJsObj(const napi_env& env, napi_value& jsObj,
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    napi_value valueParam = nullptr;
    size_t size = desc.size();

    napi_create_array_with_length(env, size, &jsObj);
    for (size_t i = 0; i < size; i++) {
        (void)napi_create_object(env, &valueParam);
        SetValueInt32(env, "deviceRole", static_cast<int32_t>(desc[i]->deviceRole_), valueParam);
        SetValueInt32(env, "deviceType", static_cast<int32_t>(desc[i]->deviceType_), valueParam);
        SetValueInt32(env, "id", static_cast<int32_t>(desc[i]->deviceId_), valueParam);
        SetValueString(env, "name", desc[i]->deviceName_, valueParam);
        SetValueString(env, "address", desc[i]->macAddress_, valueParam);
        SetValueString(env, "networkId", static_cast<std::string>(desc[i]->networkId_), valueParam);
        SetValueInt32(env, "interruptGroupId", static_cast<int32_t>(desc[i]->interruptGroupId_), valueParam);
        SetValueInt32(env, "volumeGroupId", static_cast<int32_t>(desc[i]->volumeGroupId_), valueParam);

        napi_value value = nullptr;
        napi_value sampleRates;
        napi_create_array_with_length(env, 1, &sampleRates);
        napi_create_int32(env, desc[i]->audioStreamInfo_.samplingRate, &value);
        napi_set_element(env, sampleRates, 0, value);
        napi_set_named_property(env, valueParam, "sampleRates", sampleRates);

        napi_value channelCounts;
        napi_create_array_with_length(env, 1, &channelCounts);
        napi_create_int32(env, desc[i]->audioStreamInfo_.channels, &value);
        napi_set_element(env, channelCounts, 0, value);
        napi_set_named_property(env, valueParam, "channelCounts", channelCounts);

        napi_value channelMasks;
        napi_create_array_with_length(env, 1, &channelMasks);
        napi_create_int32(env, desc[i]->channelMasks_, &value);
        napi_set_element(env, channelMasks, 0, value);
        napi_set_named_property(env, valueParam, "channelMasks", channelMasks);

        napi_value channelIndexMasks;
        napi_create_array_with_length(env, 1, &channelIndexMasks);
        napi_create_int32(env, desc[i]->channelIndexMasks_, &value);
        napi_set_element(env, channelIndexMasks, 0, value);
        napi_set_named_property(env, valueParam, "channelIndexMasks", channelIndexMasks);

        napi_value encodingTypes;
        napi_create_array_with_length(env, 1, &encodingTypes);
        napi_create_int32(env, desc[i]->audioStreamInfo_.encoding, &value);
        napi_set_element(env, encodingTypes, 0, value);
        napi_set_named_property(env, valueParam, "encodingTypes", encodingTypes);

        napi_set_element(env, jsObj, i, valueParam);
    }
}

void AudioPreferredOutputDeviceChangeCallbackNapi::OnPreferredOutputDeviceUpdated(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(preferredOutputDeviceCbList_.size() > 0,
        "Cannot find the reference of prefer device callback");
    AUDIO_DEBUG_LOG("OnPreferredOutputDeviceUpdated: Cb list size [%{public}zu]",
        preferredOutputDeviceCbList_.size());

    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); it++) {
        std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> cb =
            std::make_unique<AudioActiveOutputDeviceChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        
        cb->callback = (*it).first;
        cb->callbackName = PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME;
        cb->desc = desc;
        OnJsCallbackActiveOutputDeviceChange(cb);
    }
    return;
}

void AudioPreferredOutputDeviceChangeCallbackNapi::OnJsCallbackActiveOutputDeviceChange(
    std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        AUDIO_ERR_LOG("AudioPreferredOutputDeviceChangeCallbackNapi: OnJsCallbackRingerMode: No memory");
        return;
    }
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("AudioPreferredOutputDeviceChangeCallbackNapi: OnJsCallbackRingerMode: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        // Js Thread
        AudioActiveOutputDeviceChangeJsCallback *event =
            reinterpret_cast<AudioActiveOutputDeviceChangeJsCallback *>(work->data);
        std::string request = event->callbackName;
        napi_env env = event->callback->env_;
        napi_ref callback = event->callback->cb_;
        AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
        do {
            CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

            napi_value jsCallback = nullptr;
            napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
                request.c_str());

            // Call back function
            napi_value args[1] = { nullptr };
            NativeDeviceDescToJsObj(env, args[0], event->desc);
            
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
                "%{public}s fail to create ringer mode callback", request.c_str());

            const size_t argCount = 1;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback", request.c_str());
        } while (0);
        delete event;
        delete work;
    }, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}

AudioPreferredInputDeviceChangeCallbackNapi::AudioPreferredInputDeviceChangeCallbackNapi(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("AudioPreferredInputDeviceChangeCallbackNapi: instance create");
}

AudioPreferredInputDeviceChangeCallbackNapi::~AudioPreferredInputDeviceChangeCallbackNapi()
{
    AUDIO_DEBUG_LOG("AudioPreferredInputDeviceChangeCallbackNapi: instance destroy");
}

void AudioPreferredInputDeviceChangeCallbackNapi::SaveCallbackReference(SourceType sourceType, napi_value callback)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = 1;

    bool isSameCallback = true;
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        isSameCallback = AudioCommonNapi::IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    preferredInputDeviceCbList_.push_back({cb, sourceType});
    AUDIO_INFO_LOG("Save callback reference success, prefer input device callback list size [%{public}zu]",
        preferredInputDeviceCbList_.size());
}

void AudioPreferredInputDeviceChangeCallbackNapi::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);

    if (callback == nullptr) {
        AUDIO_INFO_LOG("RemoveCallbackReference: js callback is nullptr, remove all callback reference");
        RemoveAllCallbacks();
        return;
    }
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        bool isSameCallback = AudioCommonNapi::IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCallbackReference: find js callback, delete it");
            napi_status status = napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            CHECK_AND_RETURN_LOG(status == napi_ok, "RemoveCallbackReference: delete reference for callback fail");
            preferredInputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void AudioPreferredInputDeviceChangeCallbackNapi::RemoveAllCallbacks()
{
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    preferredInputDeviceCbList_.clear();
}

void AudioPreferredInputDeviceChangeCallbackNapi::OnPreferredInputDeviceUpdated(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);
    CHECK_AND_RETURN_LOG(preferredInputDeviceCbList_.size() > 0, "Cannot find the reference of prefer device callback");
    AUDIO_DEBUG_LOG("OnPreferredInputDeviceUpdated: Cb list size [%{public}zu]", preferredInputDeviceCbList_.size());

    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); it++) {
        std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> cb =
            std::make_unique<AudioActiveInputDeviceChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

        cb->callback = (*it).first;
        cb->callbackName = PREFERRED_INPUT_DEVICE_CALLBACK_NAME;
        cb->desc = desc;
        OnJsCallbackActiveInputDeviceChange(cb);
    }
    return;
}

void AudioPreferredInputDeviceChangeCallbackNapi::OnJsCallbackActiveInputDeviceChange(
    std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveInputDeviceChange: No memory");
        return;
    }
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveInputDeviceChange: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, [] (uv_work_t *work, int status) {
        // Js Thread
        AudioActiveInputDeviceChangeJsCallback *event =
            reinterpret_cast<AudioActiveInputDeviceChangeJsCallback *>(work->data);
        std::string request = event->callbackName;
        napi_env env = event->callback->env_;
        napi_ref callback = event->callback->cb_;
        AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());

        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[1] = { nullptr };
        NativeDeviceDescToJsObj(env, args[0], event->desc);

        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create ringer mode callback", request.c_str());

        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback", request.c_str());
        delete event;
        delete work;
    }, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
