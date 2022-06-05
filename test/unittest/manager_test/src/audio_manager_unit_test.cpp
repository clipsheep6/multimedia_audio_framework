/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr uint32_t MIN_DEVICE_COUNT = 2;
    constexpr uint32_t MIN_INPUT_DEVICE_COUNT = 1;
    constexpr uint32_t MIN_OUTPUT_DEVICE_COUNT = 1;
    constexpr uint32_t CONTENT_TYPE_UPPER_INVALID = 6;
    constexpr uint32_t STREAM_USAGE_UPPER_INVALID = 7;
    constexpr uint32_t STREAM_TYPE_UPPER_INVALID = 100;
    constexpr uint32_t CONTENT_TYPE_LOWER_INVALID = -1;
    constexpr uint32_t STREAM_USAGE_LOWER_INVALID = -1;
    constexpr uint32_t STREAM_TYPE_LOWER_INVALID = -1;
    constexpr int32_t MAX_VOL = 15;
    constexpr int32_t MIN_VOL = 0;
}

void AudioManagerUnitTest::SetUpTestCase(void) {}
void AudioManagerUnitTest::TearDownTestCase(void) {}
void AudioManagerUnitTest::SetUp(void) {}
void AudioManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test GetDevices API
* @tc.number: GetConnectedDevicesList_001
* @tc.desc  : Test GetDevices interface. Returns list of all input and output devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_001, TestSize.Level0)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    auto deviceCount = audioDeviceDescriptors.size();
    EXPECT_GE(deviceCount, MIN_DEVICE_COUNT);
}

/**
* @tc.name  : Test GetDevices API
* @tc.number: GetConnectedDevicesList_002
* @tc.desc  : Test GetDevices interface. Returns list of input devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_002, TestSize.Level0)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto deviceCount = audioDeviceDescriptors.size();
    EXPECT_GE(deviceCount, MIN_INPUT_DEVICE_COUNT);

    for (const auto &device : audioDeviceDescriptors) {
        EXPECT_EQ(device->deviceRole_, DeviceRole::INPUT_DEVICE);
    }
}

/**
* @tc.name  : Test GetDevices API
* @tc.number: GetConnectedDevicesList_003
* @tc.desc  : Test GetDevices interface. Returns list of output devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_003, TestSize.Level0)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto deviceCount = audioDeviceDescriptors.size();
    EXPECT_GE(deviceCount, MIN_OUTPUT_DEVICE_COUNT);

    for (const auto &device : audioDeviceDescriptors) {
        EXPECT_EQ(device->deviceRole_, DeviceRole::OUTPUT_DEVICE);
    }
}

/**
* @tc.name  : Test SetDeviceActive API
* @tc.number: SetDeviceActive_001
* @tc.desc  : Test SetDeviceActive interface. Activate bluetooth sco device by deactivating speaker
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_001, TestSize.Level0)
{
    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::SPEAKER);
    EXPECT_TRUE(isActive);

    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(ActiveDeviceType::BLUETOOTH_SCO, true);
    EXPECT_EQ(SUCCESS, ret);

    isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::BLUETOOTH_SCO);
    EXPECT_TRUE(isActive);
}

/**
* @tc.name  : Test SetDeviceActive API
* @tc.number: SetDeviceActive_002
* @tc.desc  : Test SetDeviceActive interface. Activate speaker device
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_002, TestSize.Level0)
{
    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::BLUETOOTH_SCO);
    EXPECT_TRUE(isActive);

    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(ActiveDeviceType::SPEAKER, true);
    EXPECT_EQ(SUCCESS, ret);

    isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::SPEAKER);
    EXPECT_TRUE(isActive);
}

/**
* @tc.name  : Test SetDeviceActive API
* @tc.number: SetDeviceActive_003
* @tc.desc  : Test SetDeviceActive interface. Switch between SPEAKER and BT SCO automatically
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_003, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(ActiveDeviceType::SPEAKER, false);
    EXPECT_EQ(SUCCESS, ret);

    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::SPEAKER);
    EXPECT_FALSE(isActive);

    isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::BLUETOOTH_SCO);
    EXPECT_TRUE(isActive);

    ret = AudioSystemManager::GetInstance()->SetDeviceActive(ActiveDeviceType::BLUETOOTH_SCO, false);
    EXPECT_EQ(SUCCESS, ret);

    isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::SPEAKER);
    EXPECT_TRUE(isActive);

    isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::BLUETOOTH_SCO);
    EXPECT_FALSE(isActive);
}

/**
* @tc.name  : Test SetAudioManagerInterruptCallback API
* @tc.number: SetAudioManagerInterruptCallback_001
* @tc.desc  : Test SetAudioManagerInterruptCallback interface with valid parameters
*/
HWTEST(AudioManagerUnitTest, SetAudioManagerInterruptCallback_001, TestSize.Level0)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetAudioManagerInterruptCallback API
* @tc.number: SetAudioManagerInterruptCallback_002
* @tc.desc  : Test SetAudioManagerInterruptCallback interface with null callback pointer as parameter
*/
HWTEST(AudioManagerUnitTest, SetAudioManagerInterruptCallback_002, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test SetAudioManagerInterruptCallback API
* @tc.number: SetAudioManagerInterruptCallback_003
* @tc.desc  : Test SetAudioManagerInterruptCallback interface with Multiple Set
*/
HWTEST(AudioManagerUnitTest, SetAudioManagerInterruptCallback_003, TestSize.Level0)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UnsetAudioManagerInterruptCallback API
* @tc.number: UnsetAudioManagerInterruptCallback_001
* @tc.desc  : Test UnsetAudioManagerInterruptCallback interface with Set and Unset callback
*/
HWTEST(AudioManagerUnitTest, UnsetAudioManagerInterruptCallback_001, TestSize.Level0)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UnsetAudioManagerInterruptCallback API
* @tc.number: UnsetAudioManagerInterruptCallback_002
* @tc.desc  : Test UnsetAudioManagerInterruptCallback interface with Multiple Unset
*/
HWTEST(AudioManagerUnitTest, UnsetAudioManagerInterruptCallback_002, TestSize.Level0)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test UnsetAudioManagerInterruptCallback API
* @tc.number: UnsetAudioManagerInterruptCallback_003
* @tc.desc  : Test UnsetAudioManagerInterruptCallback interface without set interrupt call
*/
HWTEST(AudioManagerUnitTest, UnsetAudioManagerInterruptCallback_003, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test RequestAudioFocus API
* @tc.number: RequestAudioFocus_001
* @tc.desc  : Test RequestAudioFocus interface with valid parameters
*/
HWTEST(AudioManagerUnitTest, RequestAudioFocus_001, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test RequestAudioFocus API
* @tc.number: RequestAudioFocus_002
* @tc.desc  : Test RequestAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerUnitTest, RequestAudioFocus_002, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    constexpr int32_t INVALID_CONTENT_TYPE = 10;
    audioInterrupt.contentType = static_cast<ContentType>(INVALID_CONTENT_TYPE);
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test RequestAudioFocus API
* @tc.number: RequestAudioFocus_003
* @tc.desc  : Test RequestAudioFocus interface with boundary values for content type, stream usage
*             and stream type
*/
HWTEST(AudioManagerUnitTest, RequestAudioFocus_003, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_UPPER_INVALID);
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.streamType = STREAM_VOICE_CALL;

    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = static_cast<AudioStreamType>(STREAM_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamType = static_cast<AudioStreamType>(STREAM_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_UNKNOWN;
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test RequestAudioFocus API
* @tc.number: RequestAudioFocus_004
* @tc.desc  : Test RequestAudioFocus interface with back to back requests
*/
HWTEST(AudioManagerUnitTest, RequestAudioFocus_004, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AbandonAudioFocus API
* @tc.number: AbandonAudioFocus_001
* @tc.desc  : Test AbandonAudioFocus interface with valid parameters
*/
HWTEST(AudioManagerUnitTest, AbandonAudioFocus_001, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AbandonAudioFocus API
* @tc.number: AbandonAudioFocus_002
* @tc.desc  : Test AbandonAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerUnitTest, AbandonAudioFocus_002, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    constexpr int32_t INVALID_CONTENT_TYPE = 10;
    audioInterrupt.contentType = static_cast<ContentType>(INVALID_CONTENT_TYPE);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test AbandonAudioFocus API
* @tc.number: AbandonAudioFocus_003
* @tc.desc  : Test AbandonAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerUnitTest, AbandonAudioFocus_003, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;

    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = static_cast<AudioStreamType>(STREAM_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamType = static_cast<AudioStreamType>(STREAM_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);


    audioInterrupt.contentType = CONTENT_TYPE_UNKNOWN;
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AbandonAudioFocus API
* @tc.number: AbandonAudioFocus_004
* @tc.desc  : Test AbandonAudioFocus interface multiple requests
*/
HWTEST(AudioManagerUnitTest, AbandonAudioFocus_004, TestSize.Level0)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_001
* @tc.desc  : Test AudioVolume manager interface multiple requests
*/
HWTEST(AudioManagerUnitTest, AudioVolume_001, TestSize.Level1)
{
    int32_t volume = 10;
    bool mute = true;
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioSystemManager::AudioVolumeType::STREAM_ALL, volume);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->GetVolume(AudioSystemManager::AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(volume, ret);
    ret = AudioSystemManager::GetInstance()->SetMute(AudioSystemManager::AudioVolumeType::STREAM_ALL, mute);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->IsStreamMute(AudioSystemManager::AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_001
* @tc.desc  : Test setting volume of ringtone stream with max volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_001, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioSystemManager::AudioVolumeType::STREAM_RING, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = AudioSystemManager::GetInstance()->GetVolume(AudioSystemManager::AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MAX_VOL, volume);
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_002
* @tc.desc  : Test setting volume of ringtone stream with min volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_002, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioSystemManager::AudioVolumeType::STREAM_RING, MIN_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = AudioSystemManager::GetInstance()->GetVolume(AudioSystemManager::AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, volume);
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_003
* @tc.desc  : Test setting volume of media stream with max volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_003, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioSystemManager::AudioVolumeType::STREAM_MUSIC, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t mediaVol = AudioSystemManager::GetInstance()->GetVolume(AudioSystemManager::AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(MAX_VOL, mediaVol);

    int32_t ringVolume = AudioSystemManager::GetInstance()->GetVolume(AudioSystemManager::AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, ringVolume);
}

/**
* @tc.name  : Test SetRingerMode API
* @tc.number: SetRingerModeTest_001
* @tc.desc  : Test setting of ringer mode to SILENT
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_001, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_SILENT);
}

/**
* @tc.name  : Test SetRingerMode API
* @tc.number: SetRingerModeTest_002
* @tc.desc  : Test setting of ringer mode to NORMAL
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_002, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_NORMAL);
}

/**
* @tc.name  : Test SetRingerMode API
* @tc.number: SetRingerModeTest_003
* @tc.desc  : Test setting of ringer mode to VIBRATE
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_003, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_VIBRATE);
}

/**
* @tc.name  : Test SetMicrophoneMute API
* @tc.number: SetMicrophoneMute_001
* @tc.desc  : Test muting of microphone to true
*/
HWTEST(AudioManagerUnitTest, SetMicrophoneMute_001, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMicrophoneMute(true);
    EXPECT_EQ(SUCCESS, ret);

    bool isMicrophoneMuted = AudioSystemManager::GetInstance()->IsMicrophoneMute();
    EXPECT_EQ(isMicrophoneMuted, true);
}

/**
* @tc.name  : Test SetMicrophoneMute API
* @tc.number: SetMicrophoneMute_002
* @tc.desc  : Test muting of microphone to false
*/
HWTEST(AudioManagerUnitTest, SetMicrophoneMute_002, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMicrophoneMute(false);
    EXPECT_EQ(SUCCESS, ret);

    bool isMicrophoneMuted = AudioSystemManager::GetInstance()->IsMicrophoneMute();
    EXPECT_EQ(isMicrophoneMuted, false);
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test mute functionality of ringtone stream
*/
HWTEST(AudioManagerUnitTest, SetMute_001, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioSystemManager::AudioVolumeType::STREAM_RING, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test unmute functionality of ringtone stream
*/
HWTEST(AudioManagerUnitTest, SetMute_002, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioSystemManager::AudioVolumeType::STREAM_RING, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test mute functionality of media stream
*/
HWTEST(AudioManagerUnitTest, SetMute_003, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioSystemManager::AudioVolumeType::STREAM_MUSIC, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test unmute functionality of media stream
*/
HWTEST(AudioManagerUnitTest, SetMute_004, TestSize.Level0)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioSystemManager::AudioVolumeType::STREAM_MUSIC, false);
    EXPECT_EQ(SUCCESS, ret);
}

} // namespace AudioStandard
} // namespace OHOS
