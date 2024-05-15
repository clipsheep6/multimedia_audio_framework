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

#include "audio_errors.h"
#include "audio_manager_listener_stub.h"
#include "audio_manager_proxy.h"
#include "audio_log.h"
#include "audio_process_in_client.h"
#include "audio_process_proxy.h"
#include "audio_stream.h"
#include "audio_system_manager.h"
#include <gtest/gtest.h>
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

class AudioDeviceDescriptorUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioParameterCallbackTest : public AudioParameterCallback {
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) {}
};

void AudioDeviceDescriptorUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioDeviceDescriptorUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioDeviceDescriptorUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioDeviceDescriptorUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioDeviceDescriptor API
 * @tc.type  : FUNC
 * @tc.number: AudioDeviceDescriptor_001
 * @tc.desc  : Test AudioDeviceDescriptor interface.
 */
HWTEST(AudioDeviceDescriptorUnitTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    DeviceType type = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole role = DeviceRole::OUTPUT_DEVICE;
    int32_t interruptGroupId = 1;
    int32_t volumeGroupId = 1;
    std::string networkId = "LocalDevice";
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptor =
        std::make_unique<AudioDeviceDescriptor>(type, role, interruptGroupId, volumeGroupId, networkId);
    EXPECT_NE(audioDeviceDescriptor, nullptr);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = type;
    deviceDescriptor.deviceRole_ = role;
    deviceDescriptor.channelIndexMasks_ = 1;
    audioDeviceDescriptor = std::make_unique<AudioDeviceDescriptor>(deviceDescriptor);
    EXPECT_NE(audioDeviceDescriptor, nullptr);
    EXPECT_EQ(audioDeviceDescriptor->channelIndexMasks_, 1);

    std::string deviceName = "";
    std::string macAddress = "";
    audioDeviceDescriptor->SetDeviceInfo(deviceName, macAddress);

    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.channels = STEREO;
    audioStreamInfo.encoding = ENCODING_PCM;
    audioStreamInfo.format = SAMPLE_S16LE;
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    int32_t channelMask = 1;
    int32_t channelIndexMasks = 2;
    audioDeviceDescriptor->SetDeviceCapability(audioStreamInfo, channelMask, channelIndexMasks);
    EXPECT_EQ(audioDeviceDescriptor->channelIndexMasks_, channelIndexMasks);
}
} // namespace AudioStandard
} // namespace OHOS