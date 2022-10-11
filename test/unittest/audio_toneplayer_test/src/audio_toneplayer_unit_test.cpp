/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "audio_toneplayer_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;


namespace OHOS {
namespace AudioStandard {
namespace {
    unique_ptr<TonePlayer> tonePlayer;
}

void TonePlayerUnitTest::SetUpTestCase(void) {}
void TonePlayerUnitTest::TearDownTestCase(void) {}
void TonePlayerUnitTest::SetUp(void) {}
void TonePlayerUnitTest::TearDown(void) {}

/*
 * @tc.name : TonePlayer_Create_001
 * @tc.desc : Test create interface take only one input parameter.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_Create_001, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_Create_002
 * @tc.desc : Test create interface with cachePath input parameter.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_Create_002, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    string cachePath = "/data/storage/el2/base/haps/entry/files";

    tonePlayer = TonePlayer::Create(cachePath.c_str(), rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_LoadTone_001
 * @tc.desc : Test LoadTone interface with tonetype 0.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_LoadTone_001, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    bool lstate = tonePlayer->LoadTone(0);
    EXPECT_EQ(true, lstate);

    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_LoadTone_001
 * @tc.desc : Test LoadTone interface with tonetype 1.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_LoadTone_002, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    bool lstate = tonePlayer->LoadTone(1);
    EXPECT_EQ(true, lstate);
    
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_StartTone_001
 * @tc.desc : Test StartTone interface.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_StartTone_001, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    bool ret = false;
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    ret = tonePlayer->LoadTone(1);
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StartTone();
    EXPECT_EQ(true, ret);
    
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_StopTone_001
 * @tc.desc : Test StopTone interface.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_StopTone_001, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    bool ret = false;
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    ret = tonePlayer->LoadTone(1);
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StartTone();
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StopTone();
    EXPECT_EQ(true, ret);
    
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_StopTone_002
 * @tc.desc : Test StopTone interface.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_StopTone_002, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    bool ret = false;
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    ret = tonePlayer->LoadTone(1);
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StopTone();
    EXPECT_EQ(true, ret);
    
    tonePlayer->Release();
}

/*
 * @tc.name : TonePlayer_Release_001
 * @tc.desc : Test create interface.
 * @tc.type : FUNC
 * @tc.require : issue
 */
HWTEST(TonePlayerUnitTest, TonePlayer_Release_001, TestSize.Level0)
{
    AudioRendererInfo rendererInfo = {};
    bool ret = false;
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;

    unique_ptr<TonePlayer> tonePlayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, tonePlayer);
    ret = tonePlayer->LoadTone(1);
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StartTone();
    EXPECT_EQ(true, ret);
    ret = TonePlayer->StopTone();
    EXPECT_EQ(true, ret);    
    ret = tonePlayer->Release();
    EXPECT_EQ(true, ret);  
}
}
}