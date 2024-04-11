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

#ifndef RENDERER_CAPTURE_INTERRUPT_UNIT_TEST_H
#define RENDERER_CAPTURE_INTERRUPT_UNIT_TEST_H

#include <functional>
#include "gtest/gtest.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioCapturerCallbackTest : public AudioCapturerCallback {
public:
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const CapturerState state) override {}
};

class AudioRendererCapturerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
    // Init Renderer
    static AudioRendererOptions UTCreateAudioRenderer(StreamUsage streamUsage);
    // Init Capturer
    static AudioCapturerOptions UTCreateAudioCapturer(SourceType sourceType);
    // Define interrupt func
    static void AudioInterruptUnitTestFunc(StreamUsage streamUsage, SourceType sourceType);
    static InterruptEvent CaptureinterruptEventTest_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // RENDERER_CAPTURE_INTERRUPT_UNIT_TEST_H