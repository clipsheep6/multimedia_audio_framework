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

#include "audio_group_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    // constexpr uint32_t MIN_DEVICE_COUNT = 2;
    // constexpr uint32_t MIN_INPUT_DEVICE_COUNT = 1;
    // constexpr uint32_t MIN_OUTPUT_DEVICE_COUNT = 1;
    // constexpr uint32_t CONTENT_TYPE_UPPER_INVALID = 6;
    // constexpr uint32_t STREAM_USAGE_UPPER_INVALID = 7;
    // constexpr uint32_t STREAM_TYPE_UPPER_INVALID = 100;
    // constexpr uint32_t CONTENT_TYPE_LOWER_INVALID = -1;
    // constexpr uint32_t STREAM_USAGE_LOWER_INVALID = -1;
    // constexpr uint32_t STREAM_TYPE_LOWER_INVALID = -1;
    constexpr int32_t MAX_VOL = 15;
    constexpr int32_t MIN_VOL = 0;
    int32_t groupId =1;
    // constexpr int32_t INV_CHANNEL = -1;
    // constexpr int32_t CHANNEL_10 = 10;
}

void AudioGroupManagerUnitTest::SetUpTestCase(void) {}
void AudioGroupManagerUnitTest::TearDownTestCase(void) {}
void AudioGroupManagerUnitTest::SetUp(void) {}
void AudioGroupManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioVolume API
* @tc.number: AudioVolume_001
* @tc.desc  : Test AudioVolume manager interface multiple requests
*/
HWTEST(AudioGroupManagerUnitTest, AudioVolume_001, TestSize.Level1)
{
    int32_t volume = 0;
    bool mute = true;
    auto audioGroupMngr_ = new AudioGroupManager(groupId);
    auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_ALL, volume);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(volume, ret);

    ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_ALL, mute);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(SUCCESS, ret);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_001
* @tc.desc  : Test setting volume of ringtone stream with max volume
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_001, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);
    auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MAX_VOL, volume);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_002
* @tc.desc  : Test setting volume of ringtone stream with min volume
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_002, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);
    auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_RING, MIN_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, volume);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetVolume API
* @tc.number: SetVolumeTest_003
* @tc.desc  : Test setting volume of media stream with max volume
*/
HWTEST(AudioGroupManagerUnitTest, SetVolumeTest_003, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);
    auto ret = audioGroupMngr_->SetVolume(AudioVolumeType::STREAM_MUSIC, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t mediaVol = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(MAX_VOL, mediaVol);

    int32_t ringVolume = audioGroupMngr_->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, ringVolume);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test GetMaxVolume API
* @tc.number: GetMaxVolumeTest_001
* @tc.desc  : Test GetMaxVolume  of media stream
*/
HWTEST(AudioGroupManagerUnitTest, GetMaxVolumeTest_001, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    int32_t mediaVol = audioGroupMngr_->GetMaxVolume(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(MAX_VOL, mediaVol);

    int32_t ringVolume = audioGroupMngr_->GetMaxVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MAX_VOL, ringVolume);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test GetMaxVolume API
* @tc.number: GetMinVolumeTest_001
* @tc.desc  : Test GetMaxVolume  of media stream
*/
HWTEST(AudioGroupManagerUnitTest, GetMinVolumeTest_001, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    int32_t mediaVol = audioGroupMngr_->GetMinVolume(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(MIN_VOL, mediaVol);

    int32_t ringVolume = audioGroupMngr_->GetMinVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, ringVolume);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetMute API
* @tc.number: SetMute_001
* @tc.desc  : Test mute functionality of ringtone stream
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_001, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_RING, true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(true, ret);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_002
* @tc.desc  : Test unmute functionality of ringtone stream
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_002, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_RING, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(false, ret);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_003
* @tc.desc  : Test mute functionality of media stream
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_003, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_MUSIC, true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(true, ret);

    delete audioGroupMngr_;
}

/**
* @tc.name  : Test SetMute IsStreamMute API
* @tc.number: SetMute_004
* @tc.desc  : Test unmute functionality of media stream
*/
HWTEST(AudioGroupManagerUnitTest, SetMute_004, TestSize.Level0)
{
    auto audioGroupMngr_ = new AudioGroupManager(groupId);

    auto ret = audioGroupMngr_->SetMute(AudioVolumeType::STREAM_MUSIC, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioGroupMngr_->IsStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(false, ret);

    delete audioGroupMngr_;
}
} // namespace AudioStandard
} // namespace OHOS
