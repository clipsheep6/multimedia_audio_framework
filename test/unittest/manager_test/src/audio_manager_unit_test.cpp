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

#include "audio_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_system_manager.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const uint32_t MIN_DEVICE_COUNT = 2;
    const uint32_t MIN_INPUT_DEVICE_COUNT = 1;
    const uint32_t MIN_OUTPUT_DEVICE_COUNT = 1;
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
* @tc.desc  : Test SetDeviceActive interface. Deactivate default speaker should not be success
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_001, TestSize.Level0)
{
    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(ActiveDeviceType::SPEAKER, false);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test IsDeviceActive API
* @tc.number: IsDeviceActive_001
* @tc.desc  : Test IsDeviceActive interface. Returns whether the mentioned device is currently active
*/
HWTEST(AudioManagerUnitTest, IsDeviceActive_001, TestSize.Level0)
{
    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(ActiveDeviceType::SPEAKER);
    EXPECT_TRUE(isActive);
}
} // namespace AudioStandard
} // namespace OHOS
