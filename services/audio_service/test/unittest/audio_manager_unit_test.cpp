/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#define private public
#include "audio_stream_manager.h"
#undef private

#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_untils.h"
#include "audio_errors.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
class AudioManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

AudioSystemManager *audioSystemManager = AudioSystemManager::GetInstance();
void AudioManagerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioManagerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioManagerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioManagerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_001
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is EARPIECE.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_001, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::EARPIECE;
    bool flag = true;
    EXPECT_EQ(AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag),
        audioSystemManager->SetDeviceActive(deviceType, flag));
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_002
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is SPEAKER.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_002, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::SPEAKER;
    bool flag = true;
    EXPECT_EQ(AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag),
        audioSystemManager->SetDeviceActive(deviceType, flag));
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_003
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is BLUETOOTH_SCO.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_003, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::BLUETOOTH_SCO;
    bool flag = true;
    EXPECT_EQ(AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag),
        audioSystemManager->SetDeviceActive(deviceType, flag));
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_004
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is USB_HEADSET.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_004, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::USB_HEADSET;
    bool flag = true;
    EXPECT_EQ(AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag),
        audioSystemManager->SetDeviceActive(deviceType, flag));
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_005
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is FILE_SINK_DEVICE.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_005, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::FILE_SINK_DEVICE;
    bool flag = true;
    EXPECT_EQ(AudioPolicyManager::GetInstance().SetDeviceActive(static_cast<InternalDeviceType>(deviceType), flag),
        audioSystemManager->SetDeviceActive(deviceType, flag));
}

/**
* @tc.name  : Test SetDeviceActive
* @tc.number: SetDeviceActive_006
* @tc.desc  : Test function SetDeviceActive ,Test when deviceType is ACTIVE_DEVICE_TYPE_NONE.
*/
HWTEST_F(AudioManagerUnitTest, SetDeviceActive_006, TestSize.Level0)
{
    ActiveDeviceType deviceType = ActiveDeviceType::ACTIVE_DEVICE_TYPE_NONE;
    bool flag = true;
    EXPECT_EQ(ERR_NOT_SUPPORTED, audioSystemManager->SetDeviceActive(deviceType, flag));
}
}
}