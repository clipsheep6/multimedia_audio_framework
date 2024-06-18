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

#ifndef HDI_UTILS_RINGBUFFER_H
#define HDI_UTILS_RINGBUFFER_H

#include <mutex>
#include <string>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

enum RINGBUFFER_STATE {
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,
    RINGBUFFER_HALFFULL
};

typedef struct RingBuffer{
 float* data;
 int length;
} RingBuffer;

class HdiRingBuffer{
 public:
    HdiRingBuffer();
    ~HdiRingBuffer();

    void Init(const int32_t& channelNum, const int32_t& frameLen, const int32_t& frameNum = 5); //default 5 frames
    int32_t Dequeue(RingBuffer& item);
    int32_t Enqueue(RingBuffer& item);
    RingBuffer GetItem(const int32_t &length);
 private:
    enum RINGBUFFER_STATE GetRingBufferStatus();
    int32_t GetRingBufferDateLen();
    void AddWriteIndex(const int32_t& length);
    void AddReadIndex(const int32_t& length);

 private:
    RingBuffer ringBuffer_;
    int32_t readIndex_;
    bool readFull_;
    int32_t writeIndex_;
    bool writeFull_;
    int32_t maxBufferSize_;
    std::mutex mtx_;
}

} // namespace AudioStandard
} // namespace OHOS
#endif //HDI_UTILS_RINGBUFFER_H
