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

#include "multiple_audio_capturer_unit_test.h"
#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void MultipleAudioCapturerUnitTest::SetUpTestCase(void) {}
void MultipleAudioCapturerUnitTest::TearDownTestCase(void) {}
void MultipleAudioCapturerUnitTest::SetUp(void) {}
void MultipleAudioCapturerUnitTest::TearDown(void) {}

void MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(unique_ptr<AudioCapturer> &audioCapturer,
    shared_ptr<MultipleAudioCapturerCallbackTest> &cb, SourceType sourceType, bool isStarted)
{
    AudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = sourceType;
    audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    cb = make_shared<MultipleAudioCapturerCallbackTest>();
    ASSERT_NE(nullptr, cb);
    int32_t ret = audioCapturer->SetCapturerCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    bool startOK = audioCapturer->Start();
    EXPECT_EQ(isStarted, startOK);

    return;
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_001
* @tc.desc  : Create three recording sources in sequence: MIC1, MIC2, and VOICE_RECOGNATION.
*             Expected MIC1 source state is running, MIC2 source state is stoped,
*             VOICE_RECOGNATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_001, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_002
* @tc.desc  : Create three recording sources in sequence: MIC, MIC2, and VOICE_COMMUNICATION.
*             Expected MIC1 source state is stop, MIC2 source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_002, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_003
* @tc.desc  : Create three recording sources in sequence: MIC1, MIC2, and ULTRASONIC.
*             Expected MIC1 source state is running, MIC2 source state is stoped, ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_003, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_004
* @tc.desc  : Create three recording sources in sequence: MIC1, VOICE_RECOGNATION, and MIC2.
*             Expected MIC source state is running, VOICE_RECOGNATION source state is stoped,
*             MIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_004, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_005
* @tc.desc  : Create three recording sources in sequence: MIC、 VOICE_RECOGNATION、VOICE_COMMUNICATION。
*             Expected MIC source state is stoped, VOICE_RECOGNATION source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_005, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_006
* @tc.desc  : Create three recording sources in sequence: MIC、VOICE_RECOGNATION、ULTRASONIC。
*             Expected MIC source state is running, VOICE_RECOGNATION source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_006, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_007
* @tc.desc  : Create three recording sources in sequence: MIC1、VOICE_COMMUNICATION、MIC2.
*             Expected MIC1 source state is stoped, VOICE_COMMUNICATION source state is running,
*             MIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_007, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_008
* @tc.desc  : Create three recording sources in sequence: MIC、VOICE_COMMUNICATION、VOICE_RECOGNITION.
*             Expected MIC source state is running, VOICE_COMMUNICATION source state is stoped,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_008, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_009
* @tc.desc  : Create three recording sources in sequence: MIC、VOICE_COMMUNICATION、ULTRASONIC。
*             Expected MIC source state is stoped, VOICE_COMMUNICATION source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_009, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_010
* @tc.desc  : Create three recording sources in sequence: MIC1、ULTRASONIC、MIC2。
*             Expected MIC1 source state is running, ULTRASONIC source state is running,
*             MIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_010, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_011
* @tc.desc  : Create three recording sources in sequence: MIC、ULTRASONIC、VOICE_RECOGNITION。
*             Expected MIC source state is running, ULTRASONIC source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_011, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_012
* @tc.desc  : Create three recording sources in sequence: MIC、ULTRASONIC、VOICE_COMMUNICATION。
*             Expected MIC source state is running, ULTRASONIC source state is running,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_012, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_013
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、VOICE_RECOGNITION2、MIC。
*             Expected VOICE_RECOGNITION1 source state is running, VOICE_RECOGNITION2 source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_013, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_014
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、VOICE_RECOGNITION2、VOICE_COMMUNICATION。
*             Expected VOICE_RECOGNITION1 source state is stoped, VOICE_RECOGNITION2 source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_014, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_015
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、VOICE_RECOGNITION2、ULTRASONIC。
*             Expected VOICE_RECOGNITION1 source state is running, VOICE_RECOGNITION2 source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_015, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_016
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、MIC、VOICE_RECOGNITION2。
*             Expected VOICE_RECOGNITION1 source state is running, MIC source state is stoped,
*             VOICE_RECOGNITION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_016, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_017
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、MIC、VOICE_COMMUNICATION。
*             Expected VOICE_RECOGNITION source state is stoped, MIC source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_017, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_018
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、MIC、ULTRASONIC。
*             Expected VOICE_RECOGNITION source state is running, MIC source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_018, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_019
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、VOICE_COMMUNICATION、MIC。
*             Expected VOICE_RECOGNITION source state is stoped, VOICE_COMMUNICATION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_019, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_020
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、VOICE_COMMUNICATION、VOICE_RECOGNITION。
*             Expected VOICE_RECOGNITION source state is stoped, VOICE_COMMUNICATION source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_020, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_021
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、VOICE_COMMUNICATION、MIC。
*             Expected VOICE_RECOGNITION source state is stoped, VOICE_COMMUNICATION source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_021, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_022
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、ULTRASONIC、MIC。
*             Expected VOICE_RECOGNITION source state is running, ULTRASONIC source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_022, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_023
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、ULTRASONIC、VOICE_RECOGNITION2。
*             Expected VOICE_RECOGNITION1 source state is running, ULTRASONIC source state is running,
*             VOICE_RECOGNITION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_023, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_024
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、ULTRASONIC、VOICE_COMMUNICATION。
*             Expected VOICE_RECOGNITION1 source state is stoped, ULTRASONIC source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_024, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_025
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_COMMUNICATION2、MIC。
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_COMMUNICATION2 source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_025, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_026
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_COMMUNICATION2、MIC。
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_COMMUNICATION2 source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_026, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_027
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_COMMUNICATION2、ULTRASONIC。
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_COMMUNICATION2 source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_027, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_028
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、MIC、VOICE_RECOGNITION。
*             Expected VOICE_COMMUNICATION source state is running, MIC source state is stoped,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_028, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_029
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、MIC、VOICE_COMMUNICATION2。
*             Expected VOICE_COMMUNICATION1 source state is running, MIC source state is stoped,
*             VOICE_COMMUNICATION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_029, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_030
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、MIC、ULTRASONIC。
*             Expected VOICE_COMMUNICATION source state is running, MIC source state is stoped,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_030, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_031
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、VOICE_RECOGNITION、MIC。
*             Expected VOICE_COMMUNICATION source state is running, VOICE_RECOGNITION source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_031, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_032
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_RECOGNITION、VOICE_COMMUNICATION2。
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_RECOGNITION source state is stoped,
*             VOICE_COMMUNICATION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_032, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_033
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、VOICE_RECOGNITION、ULTRASONIC。
*             Expected VOICE_COMMUNICATION source state is running, VOICE_RECOGNITION source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_033, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_034
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、ULTRASONIC、MIC。
*             Expected VOICE_COMMUNICATION source state is running, ULTRASONIC source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_034, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_035
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、ULTRASONIC、VOICE_RECOGNITION。
*             Expected VOICE_COMMUNICATION source state is running, ULTRASONIC source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_035, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_036
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、ULTRASONIC、VOICE_COMMUNICATION2。
*             Expected VOICE_COMMUNICATION1 source state is running, ULTRASONIC source state is running,
*             VOICE_COMMUNICATION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_036, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_037
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、ULTRASONIC2、MIC。
*             Expected ULTRASONIC1 source state is running, ULTRASONIC2 source state is stoped,
*             MIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_037, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_038
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、ULTRASONIC2、VOICE_RECOGNITION。
*             Expected ULTRASONIC1 source state is running, ULTRASONIC2 source state is stoped,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_038, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_039
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、ULTRASONIC2、VOICE_COMMUNICATION。
*             Expected ULTRASONIC1 source state is running, ULTRASONIC2 source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_039, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_040
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、MIC、VOICE_RECOGNITION。
*             Expected ULTRASONIC source state is running, MIC source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_040, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_041
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、MIC、VOICE_COMMUNICATION。
*             Expected ULTRASONIC source state is running, MIC source state is running,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_041, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_042
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、MIC、ULTRASONIC2。
*             Expected ULTRASONIC1 source state is running, MIC source state is running,
*             ULTRASONIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_042, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_043
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_RECOGNITION、MIC。
*             Expected ULTRASONIC source state is running, VOICE_RECOGNITION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_043, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_044
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_RECOGNITION、VOICE_COMMUNICATION。
*             Expected ULTRASONIC source state is running, VOICE_RECOGNITION source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_044, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_045
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、VOICE_RECOGNITION、ULTRASONIC2。
*             Expected ULTRASONIC1 source state is running, VOICE_RECOGNITION source state is running,
*             ULTRASONIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_045, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_046
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_COMMUNICATION、MIC。
*             Expected ULTRASONIC source state is running, VOICE_COMMUNICATION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_046, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_047
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_COMMUNICATION、VOICE_RECOGNITION。
*             Expected ULTRASONIC source state is running, VOICE_COMMUNICATION source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_047, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_048
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、VOICE_COMMUNICATION、ULTRASONIC2。
*             Expected ULTRASONIC1 source state is running, VOICE_COMMUNICATION source state is running,
*             ULTRASONIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_048, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapturerUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}
} // namespace AudioStandard
} // namespace OHOS
