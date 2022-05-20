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

#ifndef AUDIO_STREAM_CHANGE_LISTNER_UNIT_TEST_H
#define AUDIO_STREAM_CHANGE_LISTNER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_renderer.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioRendererStateChangeCallbackTest : public AudioRendererStateChangeCallback {
public:
    explicit AudioRendererStateChangeCallbackTest(const std::string &testCaseName);
    ~AudioRendererStateChangeCallbackTest() = default;

    void OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;
private:
    std::string testCaseName_;
};

/*
class AudioRendererStateChangeCallbackTest : public AudioRendererStateChangeCallback {
public:

};*/

class RendererPositionCallbackTest : public RendererPositionCallback {
public:
    void OnMarkReached(const int64_t &framePosition) override {}
};

class RendererPeriodPositionCallbackTest : public RendererPeriodPositionCallback {
public:
    void OnPeriodReached(const int64_t &frameNumber) override {}
};

class AudioRenderModeCallbackTest : public AudioRendererWriteCallback {
public:
    void OnWriteData(size_t length) override;
};

class AudioStreamChangeListnerUnitTest : public testing::Test {
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
    static int32_t InitializeRenderer(std::unique_ptr<AudioRenderer> &audioRenderer);
    // Init Renderer Options
    static void InitializeRendererOptions(AudioRendererOptions &rendererOptions);
    static void WaitForCallback();
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_STREAM_CHANGE_LISTNER_UNIT_TEST_H
