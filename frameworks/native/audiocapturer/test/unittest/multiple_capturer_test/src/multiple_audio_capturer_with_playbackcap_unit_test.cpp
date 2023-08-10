#include "multiple_audio_capturer_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_101
* @tc.desc  : Create three recording sources in sequence: MIC1, MIC2 and PLAYBACK_CAPTURE.
*             Expected MIC1 source state is running, MIC2 source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_101, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_102
* @tc.desc  : Create three recording sources in sequence: MIC1, VOICE_RECOGNITION and PLAYBACK_CAPTURE.
*             Expected MIC1 source state is running, VOICE_RECOGNITION source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_102, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_103
* @tc.desc  : Create three recording sources in sequence: MIC1, VOICE_COMMUNICATION and PLAYBACK_CAPTURE.
*             Expected MIC1 source state is stoped, VOICE_COMMUNICATION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_103, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_104
* @tc.desc  : Create three recording sources in sequence: MIC1, ULTRASONIC and PLAYBACK_CAPTURE.
*             Expected MIC1 source state is running, ULTRASONIC source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_104, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_105
* @tc.desc  : Create three recording sources in sequence: MIC1, WAKEUP and PLAYBACK_CAPTURE.
*             Expected MIC1 source state is running, WAKEUP source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_105, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_106
* @tc.desc  : Create three recording sources in sequence: MIC1、PLAYBACK_CAPTURE、MIC2。
*             Expected MIC1 source state is running, PLAYBACK_CAPTURE source state is running,
*             MIC2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_106, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_107
* @tc.desc  : Create three recording sources in sequence: MIC、PLAYBACK_CAPTURE、VOICE_RECOGNITION。
*             Expected MIC source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_107, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_108
* @tc.desc  : Create three recording sources in sequence: MIC、PLAYBACK_CAPTURE、VOICE_COMMUNICATION。
*             Expected MIC source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_108, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_109
* @tc.desc  : Create three recording sources in sequence: MIC、PLAYBACK_CAPTURE、ULTRASONIC。
*             Expected MIC source state is running, PLAYBACK_CAPTURE source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_109, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_110
* @tc.desc  : Create three recording sources in sequence: MIC、PLAYBACK_CAPTURE、WAKEUP。
*             Expected MIC source state is running, PLAYBACK_CAPTURE source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_110, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_111
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION1, VOICE_RECOGNITION2 and PLAYBACK_CAPTURE.
*             Expected VOICE_RECOGNITION1 source state is running, VOICE_RECOGNITION2 source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_111, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_112
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION, MIC and PLAYBACK_CAPTURE.
*             Expected VOICE_RECOGNITION source state is running, MIC source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_112, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_113
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION, VOICE_COMMUNICATION and PLAYBACK_CAPTURE.
*             Expected VOICE_RECOGNITION source state is stoped, VOICE_COMMUNICATION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_113, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_114
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION, ULTRASONIC and PLAYBACK_CAPTURE.
*             Expected VOICE_RECOGNITION source state is running, ULTRASONIC source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_114, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_115
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION, WAKEUP and PLAYBACK_CAPTURE.
*             Expected VOICE_RECOGNITION source state is running, WAKEUP source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_115, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_116
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、MIC。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_116, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_117
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、VOICE_RECOGNITION。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_117, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_118
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、VOICE_COMMUNICATION。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_COMMUNICATION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_118, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_119
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、ULTRASONIC。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_119, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_120
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、WAKEUP。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_120, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_121
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、VOICE_COMMUNICATION2、PLAYBACK_CAPTURE.
*             Expected VOICE_COMMUNICATION1 source state is running, VOICE_COMMUNICATION2 source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_121, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_122
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION, MIC and PLAYBACK_CAPTURE.
*             Expected VOICE_COMMUNICATION source state is running, MIC source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_122, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_123
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION, VOICE_RECOGNITION and PLAYBACK_CAPTURE.
*             Expected VOICE_COMMUNICATION source state is running, VOICE_RECOGNITION source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_123, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_124
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION, ULTRASONIC and PLAYBACK_CAPTURE.
*             Expected VOICE_COMMUNICATION source state is running, ULTRASONIC source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_124, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_125
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION, WAKEUP and PLAYBACK_CAPTURE.
*             Expected VOICE_COMMUNICATION source state is running, WAKEUP source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_125, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_126
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、PLAYBACK_CAPTURE、MIC。
*             Expected VOICE_COMMUNICATION source state is running, PLAYBACK_CAPTURE source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_126, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_127
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、PLAYBACK_CAPTURE、VOICE_RECOGNITION。
*             Expected VOICE_COMMUNICATION source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_127, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_128
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION1、PLAYBACK_CAPTURE、VOICE_COMMUNICATION2。
*             Expected VOICE_COMMUNICATION1 source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_COMMUNICATION2 source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_128, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_129
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、PLAYBACK_CAPTURE、ULTRASONIC。
*             Expected VOICE_COMMUNICATION source state is running, PLAYBACK_CAPTURE source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_129, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_130
* @tc.desc  : Create three recording sources in sequence: VOICE_RECOGNITION、PLAYBACK_CAPTURE、WAKEUP。
*             Expected VOICE_RECOGNITION source state is running, PLAYBACK_CAPTURE source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_130, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_131
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、ULTRASONIC2、PLAYBACK_CAPTURE.
*             Expected ULTRASONIC1 source state is running, ULTRASONIC2 source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_131, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_132
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC, MIC and PLAYBACK_CAPTURE.
*             Expected ULTRASONIC source state is running, MIC source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_132, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_133
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC, VOICE_RECOGNITION and PLAYBACK_CAPTURE.
*             Expected ULTRASONIC source state is running, VOICE_RECOGNITION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_133, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_134
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC, VOICE_COMMUNICATION and PLAYBACK_CAPTURE.
*             Expected ULTRASONIC source state is running, VOICE_COMMUNICATION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_134, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_135
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC, WAKEUP and PLAYBACK_CAPTURE.
*             Expected ULTRASONIC source state is running, WAKEUP source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_135, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_136
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、PLAYBACK_CAPTURE、MIC。
*             Expected ULTRASONIC source state is running, PLAYBACK_CAPTURE source state is running,
*             MIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_136, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_137
* @tc.desc  : Create three recording sources in sequence: VOICE_COMMUNICATION、PLAYBACK_CAPTURE、VOICE_RECOGNITION。
*             Expected VOICE_COMMUNICATION source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_137, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_138
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、PLAYBACK_CAPTURE、VOICE_COMMUNICATION。
*             Expected ULTRASONIC source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_138, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_139
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC1、PLAYBACK_CAPTURE、ULTRASONIC2。
*             Expected ULTRASONIC1 source state is running, PLAYBACK_CAPTURE source state is running,
*             ULTRASONIC2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_139, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_140
* @tc.desc  : Create three recording sources in sequence: ULTRASONIC、PLAYBACK_CAPTURE、WAKEUP。
*             Expected ULTRASONIC source state is running, PLAYBACK_CAPTURE source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_140, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_141
* @tc.desc  : Create three recording sources in sequence: WAKEUP1、WAKEUP2、PLAYBACK_CAPTURE.
*             Expected WAKEUP1 source state is running, WAKEUP2 source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_141, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_142
* @tc.desc  : Create three recording sources in sequence: WAKEUP, MIC and PLAYBACK_CAPTURE.
*             Expected WAKEUP source state is running, MIC source state is stoped,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_142, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_143
* @tc.desc  : Create three recording sources in sequence: WAKEUP, VOICE_RECOGNITION and PLAYBACK_CAPTURE.
*             Expected WAKEUP source state is running, VOICE_RECOGNITION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_143, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_144
* @tc.desc  : Create three recording sources in sequence: WAKEUP, VOICE_COMMUNICATION and PLAYBACK_CAPTURE.
*             Expected WAKEUP source state is stoped, VOICE_COMMUNICATION source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_144, TestSize.Level1)
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
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_145
* @tc.desc  : Create three recording sources in sequence: WAKEUP, ULTRASONIC and PLAYBACK_CAPTURE.
*             Expected WAKEUP source state is running, ULTRASONIC source state is running,
*             PLAYBACK_CAPTURE source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_145, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_146
* @tc.desc  : Create three recording sources in sequence: WAKEUP、PLAYBACK_CAPTURE、MIC。
*             Expected WAKEUP source state is running, PLAYBACK_CAPTURE source state is running,
*             MIC source state is stop.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_146, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_147
* @tc.desc  : Create three recording sources in sequence: WAKEUP、PLAYBACK_CAPTURE、VOICE_RECOGNITION。
*             Expected WAKEUP source state is running, PLAYBACK_CAPTURE source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_147, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_148
* @tc.desc  : Create three recording sources in sequence: WAKEUP、PLAYBACK_CAPTURE、VOICE_COMMUNICATION。
*             Expected WAKEUP source state is stoped, PLAYBACK_CAPTURE source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_148, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb1->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_149
* @tc.desc  : Create three recording sources in sequence: WAKEUP、PLAYBACK_CAPTURE、ULTRASONIC。
*             Expected WAKEUP source state is running, PLAYBACK_CAPTURE source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_149, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_150
* @tc.desc  : Create three recording sources in sequence: WAKEUP、PLAYBACK_CAPTURE、WAKEUP。
*             Expected WAKEUP source state is running, PLAYBACK_CAPTURE source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_150, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}




/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_151
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、PLAYBACK_CAPTURE2、MIC.
*             Expected PLAYBACK_CAPTURE1 source state is running, PLAYBACK_CAPTURE2 source state is running,
*             MIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_151, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_152
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、PLAYBACK_CAPTURE2、VOICE_RECOGNITION.
*             Expected PLAYBACK_CAPTURE1 source state is running, PLAYBACK_CAPTURE2 source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_152, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_153
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、PLAYBACK_CAPTURE2 and VOICE_COMMUNICATION.
*             Expected PLAYBACK_CAPTURE1 source state is running, PLAYBACK_CAPTURE2 source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_153, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_154
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、PLAYBACK_CAPTURE2 and ULTRASONIC.
*             Expected PLAYBACK_CAPTURE1 source state is running, PLAYBACK_CAPTURE2 source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_154, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_155
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、PLAYBACK_CAPTURE2 and WAKEUP.
*             Expected PLAYBACK_CAPTURE1 source state is running, PLAYBACK_CAPTURE2 source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_155, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_156
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、MIC、VOICE_RECOGNITION。
*             Expected PLAYBACK_CAPTURE source state is running, MIC source state is running,
*             VOICE_RECOGNITION source state is stop.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_156, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_157
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、MIC、VOICE_COMMUNICATION。
*             Expected PLAYBACK_CAPTURE source state is running, MIC source state is stop,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_157, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_158
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、MIC、ULTRASONIC。
*             Expected PLAYBACK_CAPTURE source state is running, MIC source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_158, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_159
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、MIC、WAKEUP。
*             Expected PLAYBACK_CAPTURE source state is running, MIC source state is running,
*             WAKEUP source state is stop.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_159, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
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
* @tc.number: Multiple_Audio_Capturer_160
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、MIC、PLAYBACK_CAPTURE2。
*             Expected PLAYBACK_CAPTURE1 source state is running, MIC source state is running,
*             PLAYBACK_CAPTURE2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_160, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_MIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_161
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_RECOGNITION、MIC。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_RECOGNITION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_161, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_162
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_RECOGNITION、VOICE_COMMUNICATION。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_RECOGNITION source state is stoped,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_162, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb2->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_163
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_RECOGNITION、ULTRASONIC。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_RECOGNITION source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_163, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_164
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_RECOGNITION、WAKEUP。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_RECOGNITION source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_164, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_165
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、VOICE_RECOGNITION、PLAYBACK_CAPTURE2。
*             Expected PLAYBACK_CAPTURE1 source state is running, VOICE_RECOGNITION source state is running,
*             PLAYBACK_CAPTURE2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_165, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_RECOGNITION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_166
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_COMMUNICATION、MIC。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_COMMUNICATION source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_166, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC, false);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_167
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_COMMUNICATION、VOICE_RECOGNITION。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_COMMUNICATION source state is running,
*             VOICE_RECOGNITION source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_167, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);
    usleep(200000);
    EXPECT_EQ(INTERRUPT_HINT_STOP, cb3->GetInterruptEvent().hintType);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_168
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_COMMUNICATION、ULTRASONIC。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_COMMUNICATION source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_168, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_169
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、VOICE_COMMUNICATION、WAKEUP。
*             Expected PLAYBACK_CAPTURE source state is running, VOICE_COMMUNICATION source state is running,
*             WAKEUP source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_169, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
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
* @tc.number: Multiple_Audio_Capturer_170
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、VOICE_COMMUNICATION、PLAYBACK_CAPTURE2。
*             Expected PLAYBACK_CAPTURE1 source state is running, VOICE_COMMUNICATION source state is running,
*             PLAYBACK_CAPTURE2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_170, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_VOICE_COMMUNICATION);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_171
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、ULTRASONIC、MIC。
*             Expected PLAYBACK_CAPTURE source state is running, ULTRASONIC source state is running,
*             MIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_171, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_MIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_172
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、ULTRASONIC、VOICE_RECOGNITION。
*             Expected PLAYBACK_CAPTURE source state is running, ULTRASONIC source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_172, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_173
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、ULTRASONIC、VOICE_COMMUNICATION。
*             Expected PLAYBACK_CAPTURE source state is running, ULTRASONIC source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_173, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_174
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、ULTRASONIC、WAKEUP。
*             Expected PLAYBACK_CAPTURE source state is running, ULTRASONIC source state is running,
*             WAKEUP source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_174, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_WAKEUP);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_175
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、ULTRASONIC、PLAYBACK_CAPTURE2。
*             Expected PLAYBACK_CAPTURE1 source state is running, ULTRASONIC source state is running,
*             PLAYBACK_CAPTURE2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_175, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_ULTRASONIC);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_176
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、WAKEUP、MIC。
*             Expected PLAYBACK_CAPTURE source state is running, WAKEUP source state is running,
*             MIC source state is stoped.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_176, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
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
* @tc.number: Multiple_Audio_Capturer_177
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、WAKEUP、VOICE_RECOGNITION。
*             Expected PLAYBACK_CAPTURE source state is running, WAKEUP source state is running,
*             VOICE_RECOGNITION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_177, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_178
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、WAKEUP、VOICE_COMMUNICATION。
*             Expected PLAYBACK_CAPTURE source state is running, WAKEUP source state is running,
*             VOICE_COMMUNICATION source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_178, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_179
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE、WAKEUP、ULTRASONIC。
*             Expected PLAYBACK_CAPTURE source state is running, WAKEUP source state is running,
*             ULTRASONIC source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_179, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_ULTRASONIC);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}

/**
* @tc.name  : Test Multiple AudioCapturer Interrupt.
* @tc.number: Multiple_Audio_Capturer_180
* @tc.desc  : Create three recording sources in sequence: PLAYBACK_CAPTURE1、WAKEUP、PLAYBACK_CAPTURE2。
*             Expected PLAYBACK_CAPTURE1 source state is running, WAKEUP source state is running,
*             PLAYBACK_CAPTURE2 source state is running.
*/
HWTEST(MultipleAudioCapturerUnitTest, Multiple_Audio_Capturer_180, TestSize.Level1)
{
    unique_ptr<AudioCapturer> audioCapturer1 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb1 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer2 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb2 = nullptr;
    unique_ptr<AudioCapturer> audioCapturer3 = nullptr;
    shared_ptr<MultipleAudioCapturerCallbackTest> cb3 = nullptr;

    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer1, cb1, SOURCE_TYPE_PLAYBACK_CAPTURE);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer2, cb2, SOURCE_TYPE_WAKEUP);
    MultipleAudioCapturerUnitTest::AudioCapUnitTestFunc(audioCapturer3, cb3, SOURCE_TYPE_PLAYBACK_CAPTURE);

    audioCapturer1->Release();
    audioCapturer2->Release();
    audioCapturer3->Release();
}
} // namespace AudioStandard
} // namespace OHOS