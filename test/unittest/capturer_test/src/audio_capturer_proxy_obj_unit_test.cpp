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

#include "audio_capturer_proxy_obj_unit_test.h"
//#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr float DEFAULT_VOLUME = 1.0;
} // namespace

void AudioCapturerProxyObjUnitTest::SetUpTestCase(void) {}
void AudioCapturerProxyObjUnitTest::TearDownTestCase(void) {}
void AudioCapturerProxyObjUnitTest::SetUp(void) {}
void AudioCapturerProxyObjUnitTest::TearDown(void) {}

/**
* @tc.name  : Test PausedStreamImpl API
* @tc.number: Audio_Capturer_Proxy_Obj_PausedStreamImpl_001
* @tc.desc  : Test PausedStreamImpl interface.
*/
HWTEST(AudioCapturerProxyObjUnitTest, Audio_Capturer_Proxy_Obj_PausedStreamImpl_001, TestSize.Level0)
{
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    EXPECT_NE(nullptr, capturerProxyObj_);
    StreamSetStateEventInternal sreamSetStateEventInternal = {};
    capturerProxyObj_->PausedStreamImpl(sreamSetStateEventInternal);
}

/**
* @tc.name  : Test ResumeStreamImpl API
* @tc.number: Audio_Capturer_Proxy_Obj_ResumeStreamImpl_001
* @tc.desc  : Test ResumeStreamImpl interface.
*/
HWTEST(AudioCapturerProxyObjUnitTest, Audio_Capturer_Proxy_Obj_ResumeStreamImpl_001, TestSize.Level0)
{
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    EXPECT_NE(nullptr, capturerProxyObj_);
    StreamSetStateEventInternal sreamSetStateEventInternal = {};
    capturerProxyObj_->ResumeStreamImpl(sreamSetStateEventInternal);
}

/**
* @tc.name  : Test SetLowPowerVolumeImpl API
* @tc.number: Audio_Capturer_Proxy_Obj_SetLowPowerVolumeImpl_001
* @tc.desc  : Test SetLowPowerVolumeImpl interface.
*/
HWTEST(AudioCapturerProxyObjUnitTest, Audio_Capturer_Proxy_Obj_SetLowPowerVolumeImpl_001, TestSize.Level0)
{
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    EXPECT_NE(nullptr, capturerProxyObj_);
    float volume = 0;
    capturerProxyObj_->SetLowPowerVolumeImpl(volume);
}

/**
* @tc.name  : Test GetLowPowerVolumeImpl API
* @tc.number: Audio_Capturer_Proxy_Obj_GetLowPowerVolumeImpl_001
* @tc.desc  : Test GetLowPowerVolumeImpl interface.
*/
HWTEST(AudioCapturerProxyObjUnitTest, Audio_Capturer_Proxy_Obj_GetLowPowerVolumeImpl_001, TestSize.Level0)
{
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    EXPECT_NE(nullptr, capturerProxyObj_);
    float volume;
    capturerProxyObj_->GetLowPowerVolumeImpl(volume);
    EXPECT_EQ(DEFAULT_VOLUME, volume);
}

/**
* @tc.name  : Test GetSingleStreamVolumeImpl API
* @tc.number: Audio_Capturer_Proxy_Obj_GetSingleStreamVolumeImpl_001
* @tc.desc  : Test GetSingleStreamVolumeImpl interface.
*/
HWTEST(AudioCapturerProxyObjUnitTest, Audio_Capturer_Proxy_Obj_GetSingleStreamVolumeImpl_001, TestSize.Level0)
{
    std::shared_ptr<AudioCapturerProxyObj> capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    EXPECT_NE(nullptr, capturerProxyObj_);
    float volume;
    capturerProxyObj_->GetSingleStreamVolumeImpl(volume);
    EXPECT_EQ(0, volume);
}
} // namespace AudioStandard
} // namespace OHOS
