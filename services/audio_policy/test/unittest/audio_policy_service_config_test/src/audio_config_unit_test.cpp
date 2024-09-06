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

#include "audio_config_unit_test.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioConfigConcurrencyParserUnitTest::SetUpTestCase(void) {}
void AudioConfigConcurrencyParserUnitTest::TearDownTestCase(void) {}
void AudioConfigConcurrencyParserUnitTest::SetUp(void) {}
void AudioConfigConcurrencyParserUnitTest::TearDown(void) {}


#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
* @tc.name  : Test AudioConfigAdapterInfoUnitTest.
* @tc.number: AudioAdapterInfo_001
* @tc.desc  : Test AudioAdapterInfo::GetPipeByName.
*/
HWTEST(AudioConfigAdapterInfoUnitTest, AudioAdapterInfo_001, TestSize.Level1)
{
    AudioAdapterInfo info;
    EXPECT_EQ(info.pipeInfos_.size(), 0);
    auto ret = info.GetPipeByName("test000");
    EXPECT_EQ(ret, nullptr);
    PipeInfo pInfo1;
    pInfo1.name_ = "test001";
    info.pipeInfos_.push_back(pInfo1);
    PipeInfo pInfo2;
    pInfo2.name_ = "test002";
    info.pipeInfos_.push_back(pInfo2);
    ret = info.GetPipeByName("test002");
    EXPECT_EQ(&info.pipeInfos_.back(), ret);
}

/**
* @tc.name  : Test AudioConfigAdapterInfoUnitTest.
* @tc.number: AudioAdapterInfo_002
* @tc.desc  : Test AudioAdapterInfo::GetDeviceInfoByDeviceType.
*/
HWTEST(AudioConfigAdapterInfoUnitTest, AudioAdapterInfo_002, TestSize.Level1)
{
    AudioAdapterInfo info;
    EXPECT_EQ(info.deviceInfos_.size(), 0);
    auto ret = info.GetDeviceInfoByDeviceType(DeviceType::DEVICE_TYPE_REMOTE_CAST);
    EXPECT_EQ(ret, nullptr);
    AudioPipeDeviceInfo dInfo1, dInfo2;
    info.deviceInfos_.push_back(dInfo1);
    info.deviceInfos_.push_back(dInfo2);
    ret = info.GetDeviceInfoByDeviceType(DeviceType::DEVICE_TYPE_REMOTE_CAST);
    EXPECT_EQ(ret, nullptr);
    ret = info.GetDeviceInfoByDeviceType(DeviceType::DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(&info.deviceInfos_.front(), ret);
}

/**
* @tc.name  : Test AudioConfigConcurrencyParserUnitTest.
* @tc.number: AudioConcurrencyParser_001
* @tc.desc  : Test AudioConcurrencyParser interfaces.
*/
HWTEST(AudioConfigConcurrencyParserUnitTest, AudioConcurrencyParser_001, TestSize.Level1)
{
    AudioConcurrencyParser parser;
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyMap;
    auto retStatus = parser.LoadConfig(concurrencyMap, "resource/no_such_file.xml");
    EXPECT_EQ(retStatus, ERR_OPERATION_FAILED);
    retStatus = parser.LoadConfig(concurrencyMap, "resource/invalid_xml_file.xml");
    EXPECT_EQ(retStatus, ERR_OPERATION_FAILED);
    retStatus = parser.LoadConfig(concurrencyMap, "resource/wrong_file.xml");
    EXPECT_EQ(retStatus, ERR_OPERATION_FAILED);
    retStatus = parser.LoadConfig(concurrencyMap, "resource/config_file_000.xml");
    EXPECT_EQ(retStatus, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
