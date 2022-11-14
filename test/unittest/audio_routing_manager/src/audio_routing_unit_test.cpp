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

#include "audio_capturer_unit_test.h"

#include <thread>

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const string AUDIO_CAPTURE_FILE1 = "/data/audiocapturetest_blocking.pcm";
    const string AUDIO_CAPTURE_FILE2 = "/data/audiocapturetest_nonblocking.pcm";
    const string AUDIO_TIME_STABILITY_TEST_FILE = "/data/audiocapture_getaudiotime_stability_test.pcm";
    const string AUDIO_FLUSH_STABILITY_TEST_FILE = "/data/audiocapture_flush_stability_test.pcm";
    const int32_t READ_BUFFERS_COUNT = 128;
    const int32_t VALUE_NEGATIVE = -1;
    const int32_t VALUE_ZERO = 0;
    const int32_t VALUE_HUNDRED = 100;
    const int32_t VALUE_THOUSAND = 1000;
    const int32_t CAPTURER_FLAG = 0;

    constexpr uint64_t BUFFER_DURATION_FIVE = 5;
    constexpr uint64_t BUFFER_DURATION_TEN = 10;
    constexpr uint64_t BUFFER_DURATION_FIFTEEN = 15;
    constexpr uint64_t BUFFER_DURATION_TWENTY = 20;
} // namespace

void AudioServiceUnitTest::SetUpTestCase(void) {}
void AudioServiceUnitTest::TearDownTestCase(void) {}
void AudioServiceUnitTest::SetUp(void) {}
void AudioServiceUnitTest::TearDown(void) {}



/**
* @tc.name  : Test GetSupportedFormats API
* @tc.number: Audio_Capturer_GetSupportedFormats_001
* @tc.desc  : Test GetSupportedFormats interface. Returns supported Formats on success.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_GetSupportedFormats_001, TestSize.Level0)
{
    vector<AudioSampleFormat> supportedFormatList = AudioCapturer::GetSupportedFormats();
    EXPECT_EQ(AUDIO_SUPPORTED_FORMATS.size(), supportedFormatList.size());
}
} // namespace AudioStandard
} // namespace OHOS
