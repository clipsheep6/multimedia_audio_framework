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

#include <gtest/gtest.h>
#include <audio_info.h>
#include "tone_player_private.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
constexpr uint32_t SLEEP_TIME = 30000;
static size_t g_reqBufLen = 0;
class AudioToneplayerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioToneplayerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioToneplayerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioToneplayerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioToneplayerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

class AudioRendererCallbackTest : public AudioRendererCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override {}
    void OnStateChange(const RendererState state, const StateChangeCmdType cmdType) override {}
};

class AudioToneplayerCallbackTest : public AudioRendererCallback {
public:
    void OnWriteData(size_t length);
};

void AudioToneplayerCallbackTest::OnWriteData(size_t length)
{
    g_reqBufLen = length;
}

/**
 * @tc.name  : Test Create API
 * @tc.type  : FUNC
 * @tc.number: Create_001
 * @tc.desc  : Test Create interface.
 */
HWTEST(AudioToneplayerUnitTest, Create_001, TestSize.Level1)
{
    std::shared_ptr<TonePlayer> toneplayer = nullptr;
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);
    
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    toneplayer->Release();
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_001
 * @tc.desc  : Test create->LoadTone->StartTone->StopTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_1);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    usleep(SLEEP_TIME); // 30ms sleep time
    bool stopRet = toneplayer->StopTone();
    EXPECT_EQ(true, stopRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_002
 * @tc.desc  : Test create->StartTone->StopTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_002, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_2);
    EXPECT_EQ(true, ret);

    usleep(SLEEP_TIME); // 30ms sleep time
    bool stopRet = toneplayer->StopTone();
    EXPECT_EQ(false, stopRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_003
 * @tc.desc  : Test create->StartTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_003, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(false, startRet);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_004
 * @tc.desc  : Test create->LoadTone->StartTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_004, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(TONE_TYPE_DIAL_6);
    EXPECT_EQ(true, ret);

    bool startRet = toneplayer->StartTone();
    EXPECT_EQ(true, startRet);

    usleep(SLEEP_TIME); // 30ms sleep time

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_005
 * @tc.desc  : Test create->LoadTone->Release interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_005, TestSize.Level1)
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_DTMF;
    rendererInfo.rendererFlags = 0;
    std::shared_ptr<TonePlayer> toneplayer = TonePlayer::Create(rendererInfo);
    ASSERT_NE(nullptr, toneplayer);

    bool ret = toneplayer->LoadTone(NUM_TONES);
    EXPECT_EQ(false, ret);

    bool releaseRet = toneplayer->Release();
    EXPECT_EQ(true, releaseRet);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_006
 * @tc.desc  : Test PlayEventStateHandler interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_006, TestSize.Level1)
{
    string cachePath = "";
    AudioRendererInfo rendereInfo;
    std::unique_ptr<TonePlayerPrivate> tonePlayerPrivate =
        std::make_unique<TonePlayerPrivate>(cachePath.c_str(), rendereInfo)
    ASSERT_NE(nullptr, tonePlayerPrivate);

    bool ret = tonePlayerPrivate->PlayEventStateHandler();
    EXPECT_EQ(true, ret);

    ret = tonePlayerPrivate->TonePlayerStateHandler(TonePlayerPrivate::PLAYER_EVENT_PLAY);
    EXPECT_EQ(true, ret);

    ret = tonePlayerPrivate->TonePlayerStateHandler(TonePlayerPrivate::PLAYER_EVENT_STOP);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_007
 * @tc.desc  : Test CheckToneStopped interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_007, TestSize.Level1)
{
    string cachePath = "";
    AudioRendererInfo rendereInfo;
    std::unique_ptr<TonePlayerPrivate> tonePlayerPrivate =
        std::make_unique<TonePlayerPrivate>(cachePath.c_str(), rendereInfo)
    ASSERT_NE(nullptr, tonePlayerPrivate);

    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_STOPPED;
    bool ret = tonePlayerPrivate->CheckToneStopped();
    EXPECT_EQ(true, ret);

    ret = false;
    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_STOPPING;
    ret = tonePlayerPrivate->CheckToneStopped();
    EXPECT_EQ(true, ret);

    ret = false;
    tonePlayerPrivate->toneInfo_->segments[tonePlayerPrivate->currSegment_].duration = 0;
    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_PLAYING;
    ret = tonePlayerPrivate->CheckToneStopped();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_008
 * @tc.desc  : Test CheckToneStarted interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_008, TestSize.Level1)
{
    string cachePath = "";
    AudioRendererInfo rendereInfo;
    std::unique_ptr<TonePlayerPrivate> tonePlayerPrivate =
        std::make_unique<TonePlayerPrivate>(cachePath.c_str(), rendereInfo)
    ASSERT_NE(nullptr, tonePlayerPrivate);

    uint32_t reqSample = 100;
    tonePlayerPrivate->currSegment_ = 0;
    tonePlayerPrivate->toneInfo_ = std::make_shared<ToneInfo>();
    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_STARTING;
    tonePlayerPrivate->toneInfo_->segments[tonePlayerPrivate->currSegment_].duration = 100;
    tonePlayerPrivate->toneInfo_->segments[tonePlayerPrivate->currSegment_].waveFreq = 400;
    int8_t audioBuffer[100];

    uint32_t reqSample = 100;
    bool ret = tonePlayerPrivate->CheckToneStarted(reqSample, audioBuffer);
    EXPECT_EQ(true, ret);

    tonePlayerState_ = TONE_PLAYER_PLAYING;
    ret = tonePlayerPrivate->CheckToneStarted(reqSample, audioBuffer);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_009
 * @tc.desc  : Test CheckToneContinuity interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_009, TestSize.Level1)
{
    string cachePath = "";
    AudioRendererInfo rendereInfo;
    std::unique_ptr<TonePlayerPrivate> tonePlayerPrivate =
        std::make_unique<TonePlayerPrivate>(cachePath.c_str(), rendereInfo)
    ASSERT_NE(nullptr, tonePlayerPrivate);

    tonePlayerPrivate->toneInfo_ = std::make_shared<ToneInfo>();
    tonePlayerPrivate->toneInfo_->segments[currSegment_].duration = 100;
    tonePlayerPrivate->currCount_ = 0;
    tonePlayerPrivate->currSegment_ = 0;
    tonePlayerPrivate->toneInfo_->repeatCnt = 2;
    tonePlayerPrivate->toneInfo_->repeatSegment = 0;

    bool ret = tonePlayerPrivate->CheckToneContinuity();
    EXPECT_EQ(true, ret);

    tonePlayerPrivate->toneInfo_->segments[currSegment_].duration = 0;
    tonePlayerPrivate->currCount_ = 2;

    ret -false;
    ret = tonePlayerPrivate->CheckToneContinuity();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test toneplayer loading API
 * @tc.type  : FUNC
 * @tc.number: Toneplayer_010
 * @tc.desc  : Test ContinueToneplay interface.
 */
HWTEST(AudioToneplayerUnitTest, Toneplayer_010, TestSize.Level1)
{
    string cachePath = "";
    AudioRendererInfo rendereInfo;
    std::unique_ptr<TonePlayerPrivate> tonePlayerPrivate =
        std::make_unique<TonePlayerPrivate>(cachePath.c_str(), rendereInfo)
    ASSERT_NE(nullptr, tonePlayerPrivate);

    int8_t audioBuffer[100];
    uint32_t reqSample = 100;
    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_STOPPED;
    bool ret = tonePlayerPrivate->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(false, ret);

    tonePlayerPrivate->totalSample_ = 50;
    tonePlayerPrivate->nextSegSample_ = 100;
    tonePlayerPrivate->tonePlayerState_ = tonePlayerPrivate::TONE_PLAYER_PLAYING;

    ret = tonePlayerPrivate->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(true, ret);

    ret = fasle;
    tonePlayerPrivate->totalSample_ = 150;
    tonePlayerPrivate->toneInfo_->segments[currSegment_].duration = 50;
    tonePlayerPrivate->toneInfo_->segments[tonePlayerPrivate->currSegment_].waveFreq = 440;
    ret = tonePlayerPrivate->ContinueToneplay(reqSample, audioBuffer);
    EXPECT_EQ(true, ret);

} // namespace AudioStandard
} // namespace OHOS