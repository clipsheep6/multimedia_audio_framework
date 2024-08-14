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

#include "audio_capturer_unit_test.h"

#include <thread>
#include <memory>
#include <vector>

#define protected public
#define private public
#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "fast_audio_stream.h"
#undef protected
#undef private

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_system_manager.h"


using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class CapturerPositionCallbackTestStub : public CapturerPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class CapturerPeriodPositionCallbackTestStub : public CapturerPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class AudioCapturerCallbackTestStub : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override
    {
        state_ = interruptEvent.hintType;
    }
    void OnStateChange(const CapturerState state) override {}
public:
    InterruptHint state_;
};

class RendererPositionCallbackTestStub : public RendererPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class RendererPeriodPositionCallbackTestStub : public RendererPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class CapturerPolicyServiceDiedCallbackTestStub : public CapturerPolicyServiceDiedCallback {
};

static int g_writeOverflowNum = 1000;
class TestAudioStremStub : public FastAudioStream {
public:
    TestAudioStremStub() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    int32_t RemoveRendererOrCapturerPolicyServiceDiedCB() override { return ERROR; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner) override { return true; }

    State state_ = State::RUNNING;
};

/**
* @tc.name  : Test MISCELLANEOUS classes of module audio capturer.
* @tc.number: Audio_Capturer_MISCELLANEOUS_001.
* @tc.desc  : Test AudioCapturerInterruptCallbackImpl. Functions should
*             work without throwing any exception.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MISCELLANEOUS_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    AudioStreamParams tempParams = {};

    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
        
    auto interruptCallback = std::make_unique<AudioCapturerInterruptCallbackImpl>(nullptr);
    interruptCallback->UpdateAudioStream(audioStream);
    EXPECT_EQ(interruptCallback->audioStream_, audioStream);

    capturer->audioStream_ = audioStream;
    capturer->SetApplicationCachePath("/a");
    EXPECT_EQ(capturer->cachePath_, "/a");
    EXPECT_EQ(static_cast<FastAudioStream*>(audioStream.get())->cachePath_, "/a");

    InterruptEvent interruptEventResume {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_SHARE, InterruptHint::INTERRUPT_HINT_RESUME};
    InterruptEventInternal interruptEventInternalResume {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_SHARE, InterruptHint::INTERRUPT_HINT_RESUME, 0.0};
    InterruptEventInternal interruptEventInternalPause {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_SHARE, InterruptHint::INTERRUPT_HINT_PAUSE, 0.0};
    InterruptEventInternal interruptEventInternalStop {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_SHARE, InterruptHint::INTERRUPT_HINT_STOP, 0.0};
    InterruptEventInternal interruptEventInternalNone {InterruptType::INTERRUPT_TYPE_BEGIN,
        InterruptForceType::INTERRUPT_FORCE, InterruptHint::INTERRUPT_HINT_NONE, 0.0};

    interruptCallback->NotifyEvent(interruptEventResume);

    shared_ptr<AudioCapturerCallbackTestStub> cb = make_shared<AudioCapturerCallbackTestStub>();
    interruptCallback->SaveCallback(cb);
    interruptCallback->OnInterrupt(interruptEventInternalResume);
    interruptCallback->NotifyEvent(interruptEventResume);

    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_RESUME);

    auto testAudioStremStub = std::make_shared<TestAudioStremStub>();
    interruptCallback->audioStream_ = testAudioStremStub;
    interruptCallback->NotifyForcePausedToResume(interruptEventInternalResume);
    testAudioStremStub->state_ = State::PAUSED;
    interruptCallback->isForcePaused_ = true;
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalResume);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_RESUME);

    testAudioStremStub->state_ = State::RUNNING;
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalPause);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_PAUSE);

    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalStop);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_STOP);
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalNone);
    EXPECT_EQ(cb->state_, InterruptHint::INTERRUPT_HINT_NONE);
    interruptCallback->OnInterrupt(interruptEventInternalResume);
    interruptCallback->OnInterrupt(interruptEventInternalNone);
    EXPECT_NE(cb, nullptr);
}

/**
* @tc.name  : Test MISCELLANEOUS classes of module audio capturer.
* @tc.number: Audio_Capturer_MISCELLANEOUS_002.
* @tc.desc  : Test AudioCapturerPrivate. Functions should
*             work without throwing any exception.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MISCELLANEOUS_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    AudioStreamParams tempParams = {};
    capturer->audioStream_ = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM,
        tempParams, STREAM_MUSIC, getpid());

    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VIRTUAL_CAPTURE);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_WAKEUP);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_VOICE_CALL);
    capturer->FindStreamTypeBySourceType(SourceType::SOURCE_TYPE_INVALID);

    std::vector<SourceType> targetSources;
    capturer->SetAudioSourceConcurrency(targetSources);
    targetSources.push_back(SourceType::SOURCE_TYPE_VOICE_COMMUNICATION);
    auto status = capturer->SetAudioSourceConcurrency(targetSources);
    EXPECT_EQ(status, SUCCESS);

    AudioInterrupt audioInterrupt;
    capturer->GetAudioInterrupt(audioInterrupt);

    capturer->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    status = capturer->SetCaptureMode(AudioCaptureMode::CAPTURE_MODE_CALLBACK);

    EXPECT_EQ(status, ERR_ILLEGAL_STATE);
}
} // namespace AudioStandard
} // namespace OHOS
