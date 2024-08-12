/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#undef protected
#undef private

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_system_manager.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
} // namespace



/**
* @tc.name  : Test Flush API.
* @tc.number: Audio_Capturer_MIX_001
* @tc.desc  : Test SetApplicationCachePath. Returns true, if the flush is successful.
*/
HWTEST(AudioCapturerUnitTest, Audio_Capturer_MIX_001, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);
    capturer->SetApplicationCachePath("");

    AudioStreamParams tempParams = {};

    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC,
            getpid());
        
    auto interruptCallback = std::make_unique<AudioCapturerInterruptCallbackImpl>(nullptr);
    interruptCallback->UpdateAudioStream(audioStream);

    InterruptEvent interruptEventResume {InterruptType::INTERRUPT_TYPE_BEGIN, InterruptForceType::INTERRUPT_SHARE,
                                         InterruptHint::INTERRUPT_HINT_RESUME};
    
    interruptCallback->NotifyEvent(interruptEventResume);

    shared_ptr<AudioCapturerCallback> cb = make_shared<AudioCapturerCallbackTest>();
    interruptCallback->SaveCallback(cb);
    interruptCallback->NotifyEvent(interruptEventResume);

    InterruptEventInternal interruptEventInternalResume {InterruptType::INTERRUPT_TYPE_BEGIN, InterruptForceType::INTERRUPT_SHARE,
                                         InterruptHint::INTERRUPT_HINT_RESUME, 0.0};
    InterruptEventInternal interruptEventInternalPause {InterruptType::INTERRUPT_TYPE_BEGIN, InterruptForceType::INTERRUPT_SHARE,
                                         InterruptHint::INTERRUPT_HINT_PAUSE, 0.0};
    InterruptEventInternal interruptEventInternalStop {InterruptType::INTERRUPT_TYPE_BEGIN, InterruptForceType::INTERRUPT_SHARE,
                                         InterruptHint::INTERRUPT_HINT_STOP, 0.0};
    InterruptEventInternal interruptEventInternalNone {InterruptType::INTERRUPT_TYPE_BEGIN, InterruptForceType::INTERRUPT_FORCE,
                                         InterruptHint::INTERRUPT_HINT_NONE, 0.0};

    interruptCallback->NotifyForcePausedToResume(interruptEventInternalResume);

    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalResume);
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalPause);
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalStop);
    interruptCallback->HandleAndNotifyForcedEvent(interruptEventInternalNone);

    interruptCallback->OnInterrupt(interruptEventInternalResume);
    interruptCallback->OnInterrupt(interruptEventInternalNone);
    EXPECT_NE(cb, nullptr);
}

HWTEST(AudioCapturerUnitTest, Audio_Capturer_MIX_002, TestSize.Level1)
{
    AppInfo appInfo = {};
    auto capturer = std::make_unique<AudioCapturerPrivate>(STREAM_MUSIC, appInfo, false);

    AudioStreamParams tempParams = {};
    capturer->audioStream_ = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, STREAM_MUSIC,
            getpid());
    
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
