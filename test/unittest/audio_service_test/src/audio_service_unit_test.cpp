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

#include "audio_stream_unit_test.h"

#include <thread>

#include "audio_stream.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void AudioStreamUnitTest::SetUpTestCase(void) {
    AppInfo appInfo_ = {};
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    
    audioStream_ = std::make_shared<AudioStream>(STREAM_NOTIFICATION, AUDIO_MODE_PLAYBACK, appInfo_.appUid);
    if (audioStream_) {
        AUDIO_DEBUG_LOG("AudioRendererPrivate::Audio stream created");
    }
}
void AudioStreamUnitTest::TearDownTestCase(void) {}
void AudioStreamUnitTest::SetUp(void) {}
void AudioStreamUnitTest::TearDown(void) {}

/**
* @tc.name  : Test Audio_Stream_WriteCbTheadLoop_001 via legal state
* @tc.number: Audio_Stream_WriteCbTheadLoop_001
* @tc.desc  : Test WriteCbTheadLoop interface. Returns success.
*/
HWTEST(AudioStreamUnitTest, Audio_Stream_WriteCbTheadLoop_001, TestSize.Level1)
{
    audioStream_->WriteCbTheadLoop();
}

/**
* @tc.name  : Test Audio_Stream_WriteCbTheadLoop_001 via legal state
* @tc.number: Audio_Stream_WriteCbTheadLoop_001
* @tc.desc  : Test WriteCbTheadLoop interface. Returns success.
*/
HWTEST(AudioStreamUnitTest, Audio_Stream_WriteCbTheadLoop_001, TestSize.Level1)
{
    int32_t ret = -1;

    ret = AudioStreamUnitTest::InitializeStream();
    EXPECT_EQ(SUCCESS, ret);

    uint32_t samplingRate = audioStream_->GetSamplingRate();
    EXPECT_EQ(DEFAULT_SAMPLING_RATE, ret);

    uint8_t channelCount = audioStream_->GetChannelCount();
    EXPECT_EQ(DEFAULT_CHANNEL_COUNT, ret);

    uint8_t sampleSize = audioStream_->GetSampleSize();
    EXPECT_EQ(DEFAULT_SAMPLE_SIZE, ret);

    uint32_t sessionID = 0;
    uint8_t sampleSize = audioStream_->GetStreamVolume(sessionID);
    EXPECT_EQ(DEFAULT_STREAM_VOLUME, ret);
}
x'x'x'x'x
} // namespace AudioStandard
} // namespace OHOS
