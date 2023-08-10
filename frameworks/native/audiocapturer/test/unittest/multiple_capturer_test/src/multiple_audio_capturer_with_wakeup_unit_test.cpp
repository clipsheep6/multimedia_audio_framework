#include "multiple_audio_capturer_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_049
* @tc.desc  : Create three recording sources in sequence: MIC1, MIC2, and WAKEUP.
*             Expected MIC1 source state is running, MIC2 source state is stoped, WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_049, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_050
* @tc.desc  : Create three recording sources in sequence: MIC、VOICE_RECOGNATION、WAKEUP。
*             Expected MIC source state is running, VOICE_RECOGNATION source state is stoped,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_050, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_051
* @tc.desc  : Create three recording sources in sequence: MIC、VOICE_COMMUNICATION、WAKEUP。
*             Expected MIC source state is stoped, VOICE_COMMUNICATION source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_051, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_052
* @tc.desc  : Create three recording sources in sequence: MIC、ULTRASONIC、WAKEUP。
*             Expected MIC source state is running, ULTRASONIC source state is running, WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_052, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_053
* @tc.desc  : Create three recording sources in sequence: MIC1、WAKEUP、MIC2。
*             Expected MIC source state is running, WAKEUP source state is stoped, MIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_053, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_054
* @tc.desc  : Create three recording sources in sequence: MIC、WAKEUP、VOICE_RECOGNITION。
*             Expected MIC source state is running, WAKEUP source state is stoped,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_054, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_055
* @tc.desc  : Create three recording sources in sequence: MIC、WAKEUP、VOICE_COMMUNICATION.
*             Expected MIC source state is running, WAKEUP source state is stoped,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_055, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_056
* @tc.desc  : Create three recording sources in sequence: MIC、WAKEUP、ULTRASONIC。
*             Expected MIC source state is running, WAKEUP source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_056, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_057
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、VOICE_RECOGNITION2、WAKEUP。
*             Expected VOICE_RECOGNITION1 source state is running, VOICE_RECOGNITION2 source state is stoped,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_057, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_058
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、MIC、WAKEUP。
*             Expected VOICE_RECOGNITION source state is running, MIC source state is stoped,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_058, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_059
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、VOICE_COMMUNICATION、_WAKEUP。
*             Expected VOICE_RECOGNITION source state is stoped, VOICE_COMMUNICATION source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_059, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_060
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、ULTRASONIC、WAKEUP。
*             Expected VOICE_RECOGNITION1 source state is stoped, ULTRASONIC source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_060, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_061
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、WAKEUP、MIC。
*             Expected VOICE_RECOGNITION source state is running, WAKEUP source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_061, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_062
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1、WAKEUP、VOICE_RECOGNITION2。
*             Expected VOICE_RECOGNITION1 source state is running, WAKEUP source state is running,
*             VOICE_RECOGNITION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_062, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_063
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、WAKEUP、VOICE_COMMUNICATION。
*             Expected VOICE_RECOGNITION source state is stoped, WAKEUP source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_063, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_064
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、WAKEUP、ULTRASONIC。
*             Expected VOICE_RECOGNITION source state is running, WAKEUP source state is running,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_064, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_065
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_COMMUNICATION2、WAKEUP。
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_COMMUNICATION2 source state is stoped,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_065, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_066
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、MIC、WAKEUP。
*             Expected VOICE_COMMUNICATION source state is running, MIC source state is stoped,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_066, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_067
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、VOICE_RECOGNITION、WAKEUP。
*             Expected VOICE_COMMUNICATION source state is running, VOICE_RECOGNITION source state is stoped,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_067, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2,
        SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_068
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、ULTRASONIC、WAKEUP。
*             Expected VOICE_COMMUNICATION source state is running, ULTRASONIC source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_068, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_069
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、WAKEUP、 MIC。
*             Expected VOICE_COMMUNICATION source state is running, WAKEUP source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_069, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_070
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、WAKEUP、VOICE_RECOGNITION。
*             Expected VOICE_COMMUNICATION source state is running, WAKEUP source state is stoped,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_070, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_071
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、WAKEUP、VOICE_COMMUNICATION2。
*             Expected VOICE_COMMUNICATION1 source state is running, WAKEUP source state is stoped,
*             VOICE_COMMUNICATION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_071, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_072
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、WAKEUP、ULTRASONIC。
*             Expected VOICE_COMMUNICATION source state is running, WAKEUP source state is stoped,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_072, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_073
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、ULTRASONIC2、WAKEUP。
*             Expected ULTRASONIC1 source state is running, ULTRASONIC2 source state is stoped,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_073, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_074
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、MIC、WAKEUP。
*             Expected ULTRASONIC source state is running, MIC source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_074, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_075
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_RECOGNITION、WAKEUP。
*             Expected ULTRASONIC source state is running, VOICE_RECOGNITION source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_075, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);;

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_076
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、VOICE_COMMUNICATION、WAKEUP。
*             Expected ULTRASONIC source state is running, VOICE_COMMUNICATION source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_076, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_077
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、WAKEUP、MIC。
*             Expected ULTRASONIC source state is running, WAKEUP source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_077, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_078
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、WAKEUP、VOICE_RECOGNITION。
*             Expected ULTRASONIC source state is running, WAKEUP source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_078, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_079
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、WAKEUP、VOICE_COMMUNICATION。
*             Expected ULTRASONIC source state is running, WAKEUP source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_079, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_080
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、WAKEUP、ULTRASONIC。
*             Expected ULTRASONIC source state is running, WAKEUP source state is running,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_080, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_080
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、WAKEUP、ULTRASONIC。
*             Expected ULTRASONIC source state is running, WAKEUP source state is running,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_080, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_081
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、WAKEUP2、MIC。
*             Expected WAKEUP1 source state is running, WAKEUP2 source state is stoped,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_081, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_082
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、WAKEUP2、VOICE_RECOGNITION。
*             Expected WAKEUP1 source state is running, WAKEUP2 source state is stoped,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_082, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_083
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、WAKEUP2、MIC。
*             Expected WAKEUP1 source state is running, WAKEUP2 source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_083, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_084
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、WAKEUP2、ULTRASONIC。
*             Expected WAKEUP1 source state is running, WAKEUP2 source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_084, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_085
* @tc.desc  : Create three recording sources in sequence: WAKEUP、MIC、VOICE_RECOGNITION。
*             Expected WAKEUP1 source state is running, MIC source state is stoped,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_085, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_086
* @tc.desc  : Create three recording sources in sequence: WAKEUP、MIC、VOICE_COMMUNICATION。
*             Expected WAKEUP1 source state is stoped, MIC source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_086, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_087
* @tc.desc  : Create three recording sources in sequence: WAKEUP、MIC、ULTRASONIC。
*             Expected WAKEUP1 source state is running, MIC source state is stoped,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_087, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_088
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、MIC、WAKEUP2。
*             Expected WAKEUP1 source state is running, MIC source state is stoped,
*             WAKEUP2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_088, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_089
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_RECOGNITION、MIC。
*             Expected WAKEUP source state is running, VOICE_RECOGNITION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_089, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_090
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_RECOGNITION、VOICE_COMMUNICATION。
*             Expected WAKEUP source state is running, VOICE_RECOGNITION source state is running,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_090, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_091
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_RECOGNITION、ULTRASONIC。
*             Expected WAKEUP source state is running, VOICE_RECOGNITION source state is running,
*             ULTRASONIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_091, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_092
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、VOICE_RECOGNITION、WAKEUP2.
*             Expected WAKEUP1 source state is running, VOICE_RECOGNITION source state is running,
*             WAKEUP2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_092, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_093
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_COMMUNICATION、MIC。
*             Expected WAKEUP source state is stoped, VOICE_COMMUNICATION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_093, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_094
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_COMMUNICATION、VOICE_RECOGNITION。
*             Expected WAKEUP source state is stoped, VOICE_COMMUNICATION source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_094, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_095
* @tc.desc  : Create three recording sources in sequence: WAKEUP、VOICE_COMMUNICATION、ULTRASONIC。
*             Expected WAKEUP source state is stoped, VOICE_COMMUNICATION source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_095, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_096
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、VOICE_COMMUNICATION、WAKEUP2。
*             Expected WAKEUP1 source state is stoped, VOICE_COMMUNICATION source state is running,
*             WAKEUP2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_096, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_097
* @tc.desc  : Create three recording sources in sequence: WAKEUP、ULTRASONIC、MIC。
*             Expected WAKEUP source state is stoped, ULTRASONIC source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_097, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_098
* @tc.desc  : Create three recording sources in sequence: WAKEUP、ULTRASONIC、VOICE_RECOGNITION。
*             Expected WAKEUP source state is running, ULTRASONIC source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_098, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_099
* @tc.desc  : Create three recording sources in sequence: WAKEUP、ULTRASONIC、VOICE_COMMUNICATION。
*             Expected WAKEUP source state is stoped, ULTRASONIC source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_099, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_100
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、ULTRASONIC、WAKEUP2。
*             Expected WAKEUP source state is running, ULTRASONIC source state is running,
*             WAKEUP2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_100, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

} // namespace AudioStandard
} // namespace OHOS