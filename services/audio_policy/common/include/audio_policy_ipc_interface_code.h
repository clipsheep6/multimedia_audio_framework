/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_POLICY_INTERFACE_H
#define ST_AUDIO_POLICY_INTERFACE_H

/* SAID: 3009 */
namespace OHOS {
namespace AudioStandard {
enum class AudioPolicyInterfaceCode {
    GET_MAX_VOLUMELEVEL,
    GET_MIN_VOLUMELEVEL,
    SET_SYSTEM_VOLUMELEVEL,
    GET_SYSTEM_VOLUMELEVEL,
    SET_STREAM_MUTE,
    GET_STREAM_MUTE,
    IS_STREAM_ACTIVE,
    SET_DEVICE_ACTIVE,
    IS_DEVICE_ACTIVE,
    GET_ACTIVE_OUTPUT_DEVICE,
    GET_ACTIVE_INPUT_DEVICE,
    SET_RINGER_MODE,
    GET_RINGER_MODE,
    SET_AUDIO_SCENE,
    GET_AUDIO_SCENE,
    SET_MICROPHONE_MUTE,
    SET_MICROPHONE_MUTE_AUDIO_CONFIG,
    IS_MICROPHONE_MUTE,
    SET_CALLBACK,
    UNSET_CALLBACK,
    ACTIVATE_INTERRUPT,
    DEACTIVATE_INTERRUPT,
    SET_INTERRUPT_CALLBACK,
    UNSET_INTERRUPT_CALLBACK,
    REQUEST_AUDIO_FOCUS,
    ABANDON_AUDIO_FOCUS,
    GET_STREAM_IN_FOCUS,
    GET_SESSION_INFO_IN_FOCUS,
    GET_DEVICES,
    SET_WAKEUP_AUDIOCAPTURER,
    QUERY_MICROPHONE_PERMISSION,
    SELECT_OUTPUT_DEVICE,
    GET_SELECTED_DEVICE_INFO,
    SELECT_INPUT_DEVICE,
    RECONFIGURE_CHANNEL,
    GET_AUDIO_LATENCY,
    GET_SINK_LATENCY,
    GET_PREFERRED_OUTPUT_STREAM_TYPE,
    GET_PREFERRED_INPUT_STREAM_TYPE,
    REGISTER_TRACKER,
    UPDATE_TRACKER,
    GET_RENDERER_CHANGE_INFOS,
    GET_CAPTURER_CHANGE_INFOS,
    SET_LOW_POWER_STREM_VOLUME,
    GET_LOW_POWRR_STREM_VOLUME,
    UPDATE_STREAM_STATE,
    GET_SINGLE_STREAM_VOLUME,
    GET_VOLUME_GROUP_INFO,
    GET_NETWORKID_BY_GROUP_ID,
#ifdef FEATURE_DTMF_TONE
    GET_TONEINFO,
    GET_SUPPORTED_TONES,
#endif
    IS_AUDIO_RENDER_LOW_LATENCY_SUPPORTED,
    GET_USING_PEMISSION_FROM_PRIVACY,
    GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS,
    GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS,
    SET_CALLBACKS_ENABLE,
    GET_AUDIO_FOCUS_INFO_LIST,
    SET_SYSTEM_SOUND_URI,
    GET_SYSTEM_SOUND_URI,
    GET_MIN_VOLUME_STREAM,
    GET_MAX_VOLUME_STREAM,
    GET_MAX_RENDERER_INSTANCES,
    IS_VOLUME_UNADJUSTABLE,
    ADJUST_VOLUME_BY_STEP,
    ADJUST_SYSTEM_VOLUME_BY_STEP,
    GET_SYSTEM_VOLUME_IN_DB,
    QUERY_EFFECT_SCENEMODE,
    SET_PLAYBACK_CAPTURER_FILTER_INFO,
    SET_CAPTURER_SILENT_STATE,
    GET_HARDWARE_OUTPUT_SAMPLING_RATE,
    GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS,
    GET_AVAILABLE_MICROPHONE_DESCRIPTORS,
    SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED,
    GET_ABS_VOLUME_SCENE,
    SET_A2DP_DEVICE_VOLUME,
    GET_AVAILABLE_DESCRIPTORS,
    SET_AVAILABLE_DEVICE_CHANGE_CALLBACK,
    UNSET_AVAILABLE_DEVICE_CHANGE_CALLBACK,
    IS_SPATIALIZATION_ENABLED,
    SET_SPATIALIZATION_ENABLED,
    IS_HEAD_TRACKING_ENABLED,
    SET_HEAD_TRACKING_ENABLED,
    GET_SPATIALIZATION_STATE,
    IS_SPATIALIZATION_SUPPORTED,
    IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE,
    IS_HEAD_TRACKING_SUPPORTED,
    IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE,
    UPDATE_SPATIAL_DEVICE_STATE,
    REGISTER_SPATIALIZATION_STATE_EVENT,
    CONFIG_DISTRIBUTED_ROUTING_ROLE,
    SET_DISTRIBUTED_ROUTING_ROLE_CALLBACK,
    UNSET_DISTRIBUTED_ROUTING_ROLE_CALLBACK,
    UNREGISTER_SPATIALIZATION_STATE_EVENT,
    REGISTER_POLICY_CALLBACK_CLIENT,
    CREATE_AUDIO_INTERRUPT_ZONE,
    ADD_AUDIO_INTERRUPT_ZONE_PIDS,
    REMOVE_AUDIO_INTERRUPT_ZONE_PIDS,
    RELEASE_AUDIO_INTERRUPT_ZONE,
    SET_CALL_DEVICE_ACTIVE,
    GET_AUDIO_CONVERTER_CONFIG,
    GET_ACTIVE_BLUETOOTH_DESCRIPTOR,
    FETCH_OUTPUT_DEVICE_FOR_TRACK,
    FETCH_INPUT_DEVICE_FOR_TRACK,
    IS_HIGH_RESOLUTION_EXIST,
    SET_HIGH_RESOLUTION_EXIST,
    GET_SPATIALIZATION_SCENE_TYPE,
    SET_SPATIALIZATION_SCENE_TYPE,
    GET_MAX_AMPLITUDE,
    IS_HEAD_TRACKING_DATA_REQUESTED,
    SET_AUDIO_DEVICE_REFINER_CALLBACK,
    UNSET_AUDIO_DEVICE_REFINER_CALLBACK,
    TRIGGER_FETCH_DEVICE,
    MOVE_TO_NEW_PIPE,
    DISABLE_SAFE_MEDIA_VOLUME,
    GET_DEVICES_INNER,
    SET_AUDIO_CONCURRENCY_CALLBACK,
    UNSET_AUDIO_CONCURRENCY_CALLBACK,
    ACTIVATE_AUDIO_CONCURRENCY,
    SET_RINGER_MODE_MUTE,
    SET_MICROPHONE_MUTE_PERSISTENT,
    GET_MICROPHONE_MUTE_PERSISTENT,
    GET_SUPPORT_AUDIO_ENHANCE_PARAM,
    GET_SUPPORT_AUDIO_EFFECT_PARAM,
    GET_AUDIO_ENHANCE_PARAM,
    GET_AUDIO_EFFECT_PARAM,
    SET_AUDIO_ENHANCE_PARAM,
    SET_AUDIO_EFFECT_PARAM,
    AUDIO_POLICY_MANAGER_CODE_MAX = GET_MICROPHONE_MUTE_PERSISTENT,
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_INTERFACE_H
