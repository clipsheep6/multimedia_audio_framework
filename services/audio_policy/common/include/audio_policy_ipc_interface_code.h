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

#ifndef ST_AUDIO_POLICY_INTERFACE_H
#define ST_AUDIO_POLICY_INTERFACE_H

#include <audio_info.h>

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
    SET_RINGERMODE_CALLBACK,
    UNSET_RINGERMODE_CALLBACK,
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
    CLOSE_WAKEUP_AUDIOCAPTURER,
    REGISTER_DEVICE_CHANGE_CALLBACK,
    UNREGISTER_DEVICE_CHANGE_CALLBACK,
    QUERY_MICROPHONE_PERMISSION,
    SELECT_OUTPUT_DEVICE,
    GET_SELECTED_DEVICE_INFO,
    SELECT_INPUT_DEVICE,
    RECONFIGURE_CHANNEL,
    GET_AUDIO_LATENCY,
    GET_SINK_LATENCY,
    REGISTER_PLAYBACK_EVENT,
    UNREGISTER_PLAYBACK_EVENT,
    REGISTER_RECORDING_EVENT,
    UNREGISTER_RECORDING_EVENT,
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
    SET_MIC_STATE_CHANGE_CALLBACK,
    GET_USING_PEMISSION_FROM_PRIVACY,
    GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS,
    GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS,
    SET_ACTIVE_OUTPUT_DEVICE_CHANGE_CALLBACK,
    SET_ACTIVE_INPUT_DEVICE_CHANGE_CALLBACK,
    UNSET_ACTIVE_OUTPUT_DEVICE_CHANGE_CALLBACK,
    UNSET_ACTIVE_INPUT_DEVICE_CHANGE_CALLBACK,
    GET_AUDIO_FOCUS_INFO_LIST,
    REGISTER_FOCUS_INFO_CHANGE_CALLBACK,
    UNREGISTER_FOCUS_INFO_CHANGE_CALLBACK,
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
    GET_HARDWARE_OUTPUT_SAMPLING_RATE,
    GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS,
    GET_AVAILABLE_MICROPHONE_DESCRIPTORS,
    REGISTER_VOLUME_KEY_ENVENT_CALLBACK_CLIENT,
    UNREGISTER_VOLUME_KEY_ENVENT_CALLBACK_CLIENT,
    SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED,
    SET_A2DP_DEVICE_VOLUME,
    AUDIO_POLICY_MANAGER_CODE_MAX = SET_A2DP_DEVICE_VOLUME,
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_INTERFACE_H
