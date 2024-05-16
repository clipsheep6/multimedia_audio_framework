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

#include "audio_renderer_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_stream.h"
#include "audio_renderer_private.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t RENDERER_FLAG = 0;
constexpr int32_t SLEEP_TIME = 2;
} // namespace

InterruptEvent AudioRendererUnitTest::interruptEventTest_ = {};
void AudioRendererUnitTest::SetUpTestCase(void) {}
void AudioRendererUnitTest::TearDownTestCase(void) {}
void AudioRendererUnitTest::SetUp(void) {}
void AudioRendererUnitTest::TearDown(void) {}

void AudioRendererCallbackTest::OnInterrupt(const InterruptEvent &interruptEvent)
{
    AudioRendererUnitTest::interruptEventTest_.hintType = interruptEvent.hintType;
}

AudioRendererOptions AudioRendererUnitTest::UTCreateAudioRenderer(StreamUsage streamUsage)
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererOptions.rendererInfo.streamUsage = streamUsage;
    rendererOptions.rendererInfo.rendererFlags = RENDERER_FLAG;
    return rendererOptions;
}

void AudioRendererUnitTest::AudioInterruptProcessIncoming(StreamUsage streamUsageExist,
    StreamUsage streamUsageIncoming, InterruptHint hintType)
{
    AudioRendererOptions rendererOptionsExist = UTCreateAudioRenderer(streamUsageExist);
    unique_ptr<AudioRenderer> audioRendererExist = AudioRenderer::Create(rendererOptionsExist);
    ASSERT_NE(nullptr, audioRendererExist);
    audioRendererExist->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererExistStarted = audioRendererExist->Start();
    EXPECT_EQ(true, isRendererExistStarted);

    AudioRendererOptions rendererOptionsIncoming = UTCreateAudioRenderer(streamUsageIncoming);
    unique_ptr<AudioRenderer> audioRendererIncoming = AudioRenderer::Create(rendererOptionsIncoming);
    ASSERT_NE(nullptr, audioRendererIncoming);
    audioRendererIncoming->SetInterruptMode(INDEPENDENT_MODE);
    shared_ptr<AudioRendererCallbackTest> audioCapturerCB = make_shared<AudioRendererCallbackTest>();
    int32_t ret = audioRendererIncoming->SetRendererCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    bool isRendererIncomingStarted = audioRendererIncoming->Start();
    EXPECT_EQ(true, isRendererIncomingStarted);

    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    EXPECT_EQ(AudioRendererUnitTest::interruptEventTest_.hintType, hintType);
    AudioRendererUnitTest::interruptEventTest_.hintType = INTERRUPT_HINT_NONE;

    audioRendererIncoming->Release();
    audioRendererExist->Release();
}

void AudioRendererUnitTest::AudioInterruptProcessExisting(StreamUsage streamUsageExist,
    StreamUsage streamUsageIncoming, InterruptHint hintType)
{
    AudioRendererOptions rendererOptionsExist = UTCreateAudioRenderer(streamUsageExist);
    unique_ptr<AudioRenderer> audioRendererExist = AudioRenderer::Create(rendererOptionsExist);
    ASSERT_NE(nullptr, audioRendererExist);
    audioRendererExist->SetInterruptMode(INDEPENDENT_MODE);
    shared_ptr<AudioRendererCallbackTest> audioCapturerCB = make_shared<AudioRendererCallbackTest>();
    int32_t ret = audioRendererExist->SetRendererCallback(audioCapturerCB);
    EXPECT_EQ(SUCCESS, ret);
    bool isRendererExistStarted = audioRendererExist->Start();
    EXPECT_EQ(true, isRendererExistStarted);

    AudioRendererOptions rendererOptionsIncoming = UTCreateAudioRenderer(streamUsageIncoming);
    unique_ptr<AudioRenderer> audioRendererIncoming = AudioRenderer::Create(rendererOptionsIncoming);
    ASSERT_NE(nullptr, audioRendererIncoming);
    audioRendererIncoming->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererIncomingStarted = audioRendererIncoming->Start();
    EXPECT_EQ(true, isRendererIncomingStarted);

    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    EXPECT_EQ(AudioRendererUnitTest::interruptEventTest_.hintType, hintType);
    AudioRendererUnitTest::interruptEventTest_.hintType = INTERRUPT_HINT_NONE;

    audioRendererIncoming->Release();
    audioRendererExist->Release();
}

void AudioRendererUnitTest::AudioInterruptDenyIncoming(StreamUsage streamUsageExist, StreamUsage streamUsageIncoming,
    bool isStart)
{
    AudioRendererOptions rendererOptionsExist = UTCreateAudioRenderer(streamUsageExist);
    unique_ptr<AudioRenderer> audioRendererExist = AudioRenderer::Create(rendererOptionsExist);
    ASSERT_NE(nullptr, audioRendererExist);
    audioRendererExist->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererExistStarted = audioRendererExist->Start();
    EXPECT_EQ(true, isRendererExistStarted);

    AudioRendererOptions rendererOptionsIncoming = UTCreateAudioRenderer(streamUsageIncoming);
    unique_ptr<AudioRenderer> audioRendererIncoming = AudioRenderer::Create(rendererOptionsIncoming);
    ASSERT_NE(nullptr, audioRendererIncoming);
    audioRendererIncoming->SetInterruptMode(INDEPENDENT_MODE);
    bool isRendererIncomingStarted = audioRendererIncoming->Start();
    EXPECT_EQ(isStart, isRendererIncomingStarted);

    audioRendererIncoming->Release();
    audioRendererExist->Release();
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_001
 * @tc.desc  : When voip comes after voiceCall, voip will start fail(deny by voiceCall)
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_001, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptDenyIncoming(STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
        STREAM_USAGE_VOICE_COMMUNICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_002
 * @tc.desc  : When voip comes after voiceMessage, voiceMessage will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_002, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_VOICE_MESSAGE,
        STREAM_USAGE_VOICE_COMMUNICATION, INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_003
 * @tc.desc  : When voip comes after system, system will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_003, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_SYSTEM, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_004
 * @tc.desc  : When voip comes after ring, ring will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_004, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_RINGTONE, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_005
 * @tc.desc  : When voip comes after music, music will pause
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_005, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_MUSIC, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_PAUSE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_006
 * @tc.desc  : When voip comes after movie, movie will pause
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_006, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_MOVIE, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_PAUSE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_007
 * @tc.desc  : When voip comes after game, game will pause
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_007, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_GAME, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_PAUSE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_008
 * @tc.desc  : When voip comes after speech, speech will pause
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_008, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_AUDIOBOOK, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_PAUSE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_009
 * @tc.desc  : When voip comes after navigation, navigation will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_009, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_NAVIGATION, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_010
 * @tc.desc  : When voip comes after alarm, alarm will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_010, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_ALARM, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_011
 * @tc.desc  : When voip comes after notification, alarm will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_011, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_NOTIFICATION, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_012
 * @tc.desc  : When voip comes after system enforced, play both
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_012, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_ENFORCED_TONE, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_NONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_013
 * @tc.desc  : When voip comes after dtmf, dtmf will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_013, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_014
 * @tc.desc  : When voip comes after voice assistant, voice assistant will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_014, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_VOICE_ASSISTANT,
        STREAM_USAGE_VOICE_COMMUNICATION, INTERRUPT_HINT_STOP);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_015
 * @tc.desc  : When voip comes after accessibility, voip will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_015, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_ACCESSIBILITY, STREAM_USAGE_VOICE_COMMUNICATION,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_016
 * @tc.desc  : When ring comes after voip, ring will start fail(deny by voip)
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_016, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptDenyIncoming(STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_RINGTONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_017
 * @tc.desc  : When notification comes after voip, notification will start fail(deny by voip)
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_017, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptDenyIncoming(STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_NOTIFICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_018
 * @tc.desc  : When voice assistant comes after voip, voice assistant will start fail(deny by voip)
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_018, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptDenyIncoming(STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_VOICE_ASSISTANT);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_019
 * @tc.desc  : When voip2 comes after voip, voip2 will start fail(deny by voip)
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_019, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptDenyIncoming(STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_VOICE_COMMUNICATION);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_020
 * @tc.desc  : When voice message comes after voip, voice message will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_020, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_VOICE_MESSAGE,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_021
 * @tc.desc  : When voice message comes after voip, voice message will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_021, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_VOICE_MESSAGE,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_022
 * @tc.desc  : When system comes after voip, system will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_022, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_SYSTEM,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_023
 * @tc.desc  : When music comes after voip, music will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_023, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_MUSIC,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_024
 * @tc.desc  : When movie comes after voip, movie will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_024, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_MOVIE,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_025
 * @tc.desc  : When game comes after voip, game will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_025, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_GAME,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_026
 * @tc.desc  : When speech comes after voip, speech will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_026, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_AUDIOBOOK,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_027
 * @tc.desc  : When navigation comes after voip, navigation will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_027, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_NAVIGATION,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_028
 * @tc.desc  : When alarm comes after voip, alarm will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_028, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_ALARM,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_029
 * @tc.desc  : When system enforced comes after voip, play both
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_029, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_ENFORCED_TONE,
        INTERRUPT_HINT_NONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_030
 * @tc.desc  : When dtmf comes after voip, play both
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_030, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessIncoming(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_DTMF,
        INTERRUPT_HINT_NONE);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_031
 * @tc.desc  : When accessibility comes after voip, voip will duck
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_031, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_ACCESSIBILITY,
        INTERRUPT_HINT_DUCK);
}

/**
 * @tc.name  : Test Audio Interrupt rule
 * @tc.number: SetAudioInterrupt_032
 * @tc.desc  : When voice call comes after voip, voip will stop
 */
HWTEST(AudioRendererUnitTest, SetAudioInterrupt_032, TestSize.Level1)
{
    AudioRendererUnitTest::AudioInterruptProcessExisting(STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_USAGE_VOICE_MODEM_COMMUNICATION, INTERRUPT_HINT_STOP);
}
} // namespace AudioStandard
} // namespace OHOS