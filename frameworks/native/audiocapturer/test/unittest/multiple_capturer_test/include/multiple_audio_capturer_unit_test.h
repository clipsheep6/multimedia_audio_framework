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

#ifndef MULTIPLE_AUDIO_CAPTURER_UNIT_TEST_H
#define MULTIPLE_AUDIO_CAPTURER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_capturer.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
class MultipleAudioCapturerCallbackTest : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override
    {
        AUDIO_DEBUG_LOG("MultipleAudioCapturerCallbackTest: OnInterrupt Hint: %{public}d eventType: %{public}d,\
            forceType: %{public}d", interruptEvent.hintType, interruptEvent.eventType, interruptEvent.forceType);
        currInterruptEvent = interruptEvent;
    }
    void OnStateChange(const CapturerState state) override {}
    InterruptEvent GetInterruptEvent() const
    {
        return currInterruptEvent;
    }

private:
    InterruptEvent currInterruptEvent;
};

class MultipleAudioCapturerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);

    static void AudioCapturerUnitTestFunc(std::unique_ptr<AudioCapturer> &audioCapturer,
        std::shared_ptr<MultipleAudioCapturerCallbackTest> &cb, SourceType sourceType, bool isStarted = true);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // MULTIPLE_AUDIO_CAPTURER_UNIT_TEST_H
