/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef AUDIO_INTERRUPT_TEST_H
#define AUDIO_INTERRUPT_TEST_H

#include <thread>

#include "audio_renderer.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
class AudioInterruptTest : public AudioRendererCallback, public std::enable_shared_from_this<AudioInterruptTest> {
public:
    std::shared_ptr<AudioInterruptTest> GetPtr()
    {
        return shared_from_this();
    }

    int32_t TestPlayback();
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state) override;
    void SaveStreamInfo(ContentType contentType, StreamUsage streamUsage);
    FILE *wavFile_ = nullptr;
private:
    bool InitRender();
    bool StartRender();
    bool GetBufferLen(size_t &bufferLen) const;
    void WriteBuffer();

    std::unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    bool isRenderPaused_ = false;
    bool isStopInProgress_ = false;
    bool isRenderStopped_ = false;
    bool isRenderingCompleted_ = false;
    std::unique_ptr<std::thread> writeThread_ = nullptr;
    ContentType contentType_ = ContentType::CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage_ = StreamUsage::STREAM_USAGE_MEDIA;
};
} // AudioStandard
} // OHOS
#endif // AUDIO_INTERRUPT_TEST_H