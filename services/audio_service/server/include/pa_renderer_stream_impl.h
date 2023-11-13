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

#ifndef PA_RENDERER_STREAM_IMPL_H
#define PA_RENDERER_STREAM_IMPL_H

#include <pulse/pulseaudio.h>
#include "i_renderer_stream.h"

namespace OHOS {
namespace AudioStandard {
class PaRendererStreamImpl : public IRendererStream {
public:
    PaRendererStreamImpl(pa_stream *paStream, AudioStreamParams params, pa_threaded_mainloop *mainloop);
    
    int32_t Start() override;
    int32_t Pause() override;
    int32_t Flush() override;
    int32_t Drain() override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t CorkStream();
    void RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback) override;
    void RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback) override;
    BufferDesc DequeueBuffer(size_t length) override;
    int32_t EnqueueBuffer(const BufferDesc &bufferDesc) override;
    int32_t GetMinimumBufferSize(size_t &minBufferSize) const override;
    uint32_t GetStreamIndex() override;
    void AbortCallback(int32_t abortTimes) override;

private:
    static void PAStreamWriteCb(pa_stream *stream, size_t length, void *userdata);
    static void PAStreamMovedCb(pa_stream *stream, void *userdata);
    static void PAStreamUnderFlowCb(pa_stream *stream, void *userdata);
    static void PAStreamSetStartedCb(pa_stream *stream, void *userdata);
    static void PAStreamStartSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamPauseSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamFlushSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamDrainSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamDrainInStopCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamAsyncStopSuccessCb(pa_stream *stream, int32_t success, void *userdata);

    pa_stream_success_cb_t PAStreamCorkSuccessCb;
    pa_stream *paStream_ = nullptr;
    AudioStreamParams params_;
    std::weak_ptr<IStatusCallback> statusCallback_;
    std::weak_ptr<IWriteCallback> writeCallback_;
    int32_t streamCmdStatus_;
    int32_t streamDrainStatus_;
    int32_t streamFlushStatus_;
    State state_;
    uint32_t underFlowCount_;
    bool isDrain_ = false;
    pa_threaded_mainloop *mainloop_;

    // Only for debug
    int32_t abortFlag_ = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PA_RENDERER_STREAM_IMPL_H
