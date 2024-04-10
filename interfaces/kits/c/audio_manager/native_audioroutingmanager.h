/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

/**
 * @addtogroup OHAudio
 * @{
 *
 * @brief Provide the definition of the C interface for the audio module.
 *
 * @syscap SystemCapability.Multimedia.Audio.Core
 *
 * @since 12
 * @version 1.0
 */

/**
 * @file native_audioroutingmanager.h
 *
 * @brief Declare audio routing manager related interfaces.
 *
 * @syscap SystemCapability.Multimedia.Audio.Core
 * @since 12
 * @version 1.0
 */

#ifndef NATIVE_AUDIOROUTINGMANAGER_H
#define NATIVE_AUDIOROUTINGMANAGER_H

#include <time.h>
#include "native_audiodevice_base.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Declaring the audio routing manager.
 * The handle of audio routing manager used for routing and device related function.
 *
 * @since 12
 */
typedef struct OH_AudioRoutingManagerStruct OH_AudioRoutingManager;

/**
 * This function pointer will point to the callback function that
 * is used to return the chaning audio device descriptors.
 * There may be more than one audio device descriptor returned.
 *
 * @since 12
 */

typedef int32_t (*OH_AudioRoutingManager_OnDeviceChangedCallback) (
    OH_AudioDevice_ChangeType type,
    OH_AudioDeviceDescriptorArray *AudioDeviceDescriptorArray
);

/**
 * Query the audio routing manager handle which need to be set as the firest parameter in the routing related functions.
 *
 * @since 12
 */
OH_AudioCommon_Result OH_AudioManager_GetAudioRoutingManager(OH_AudioRoutingManager **audioRoutingManager);

/**
 * Query the available devices according to the input deviceFlag.
 *
 * @since 12
 *
 * @param audio routing manager handle returned by OH_AudioManager_GetAudioRoutingManager;
 * @param deviceFlag which is used as the filter parameter for selecting the target devices
 * @param audio device descriptors array pointer variable which will be set the audio device descriptors value.
 * @param audio device descriptors size pointer variable which will be set the audio device descriptors size.
 * @return {@link #AUDIOCOMMON_SUCCESS} or an undesired error.
 */
OH_AudioCommon_Result OH_AudioRoutingManager_GetDevices(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioDevice_DeviceFlag deviceFlag,
    OH_AudioDeviceDescriptorArray *AudioDeviceDescriptorArray);

/**
 * Register the device change callback of the audio routing manager.
 *
 * @since 12
 *
 * @param audio routing manager handle returned by OH_AudioManager_GetAudioRoutingManager.
 * @param callbacks Callbacks to the functions that will process the device changed event.
 * @return {@link #AUDIOCOMMON_SUCCESS} or an undesired error.
 */
OH_AudioCommon_Result OH_AudioRoutingManager_RegisterDeviceChangeCallback(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioRoutingManager_OnDeviceChangedCallback callback);

/**
 * Unregister the device change callback of the audio routing manager.
 *
 * @since 12
 * @param audio routing manager handle returned by OH_AudioManager_GetAudioRoutingManager;
 * @param callbacks Callbacks to the functions that will process the device changed event.
 * @return {@link #AUDIOCOMMON_SUCCESS} or an undesired error.
 */
OH_AudioCommon_Result OH_AudioRoutingManager_UnregisterDeviceChangeCallback(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioRoutingManager_OnDeviceChangedCallback callback);

/**
 * Release the audio device descriptor array object.
 *
 * @since 12
 * @param audio routing manager handle returned by OH_AudioManager_GetAudioRoutingManager;
 * @param audioDeviceDescriptorArray Audio device descriptors should be released.
 * @return {@link #AUDIOCOMMON_SUCCESS} or an undesired error.
 */
OH_AudioCommon_Result OH_AudioRoutingManager_ReleaseDevices(
    OH_AudioRoutingManager *audioRoutingManager,
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray);
#ifdef __cplusplus
}
#endif
#endif // NATIVE_AUDIOROUTINGMANAGER_H
