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

#ifndef ST_AUDIO_POLICY_TYPES_H
#define ST_AUDIO_POLICY_TYPES_H

#include <audio_info.h>

namespace OHOS {
namespace AudioStandard {
enum AudioPolicyCommand {
    SET_STREAM_VOLUME,
    GET_STREAM_VOLUME,
    SET_STREAM_MUTE,
    GET_STREAM_MUTE,
    IS_STREAM_ACTIVE,
    SET_DEVICE_ACTIVE,
    IS_DEVICE_ACTIVE,
    SET_RINGER_MODE,
    GET_RINGER_MODE,
    SET_AUDIO_SCENE,
    GET_AUDIO_SCENE,
    SET_CALLBACK,
    UNSET_CALLBACK,
    ACTIVATE_INTERRUPT,
    DEACTIVATE_INTERRUPT
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_TYPES_H
