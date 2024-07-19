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
#undef LOG_TAG
#define LOG_TAG "HdiUtilsRingBuffer"

#include "hdi_utils_ringbuffer.h"

#include "securec.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

HdiRingBuffer::HdiRingBuffer()
{
    ringBuffer_.data = nullptr;
    ringBuffer_.length = 0;
    outputBuffer_.data = nullptr;
    outputBuffer_.length = 0;
    inputBuffer_.data = nullptr;
    inputBuffer_.length = 0;
    readIndex_ = 0;
    readFull_ = false;
    writeIndex_ = 0;
    writeFull_ = false;
    maxBufferSize_ = 0;
    perFrameLength_ = 0;
}
HdiRingBuffer::~HdiRingBuffer()
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (ringBuffer_.data != nullptr) {
        delete [] ringBuffer_.data;
        ringBuffer_.data = nullptr;
    }
    if (outputBuffer_.data != nullptr) {
        delete [] outputBuffer_.data;
        outputBuffer_.data = nullptr;
    }
    if (inputBuffer_.data != nullptr) {
        delete [] inputBuffer_.data;
        inputBuffer_.data = nullptr;
    }
}

void HdiRingBuffer::Init(const int32_t sampleRate, const int32_t channelCount, const int32_t formatBytes,
    const int32_t onceFrameNum, const int32_t maxFrameNum)
{ 
    perFrameLength_ = ((sampleRate * onceFrameNum) / 100) * channelCount * formatBytes;
    maxBufferSize_ = perFrameLength_ * maxFrameNum;
    ringBuffer_.data = new (std::nothrow) uint8_t[maxBufferSize_];
    if (ringBuffer_.data != nullptr) {
        memset_s(static_cast<void *>(ringBuffer_.data), sizeof(uint8_t) * maxBufferSize_,
                 0, sizeof(uint8_t) * maxBufferSize_);
    } else {
        AUDIO_ERR_LOG("error: new ringBuffer_ data failed.");
    }
    outputBuffer_.data = new (std::nothrow) uint8_t[perFrameLength_];
    if (outputBuffer_.data != nullptr) {
        memset_s(static_cast<void *>(outputBuffer_.data), sizeof(uint8_t) * perFrameLength_,
                 0, sizeof(uint8_t) * perFrameLength_);
    } else {
        AUDIO_ERR_LOG("error: new outputBuffer_ data failed.");
    }
    inputBuffer_.data = new (std::nothrow) uint8_t[perFrameLength_];
    if (inputBuffer_.data != nullptr) {
        memset_s(static_cast<void *>(inputBuffer_.data), sizeof(uint8_t) * perFrameLength_,
                 0, sizeof(uint8_t) * perFrameLength_);
    } else {
        AUDIO_ERR_LOG("error: new inputBuffer_ data failed.");
    }
}
enum RingBufferState HdiRingBuffer::GetRingBufferStatus()
{
    if (readFull_) {
        return RINGBUFFER_EMPTY;
    } else if (writeFull_) {
        return RINGBUFFER_FULL;
    } else {
        return RINGBUFFER_HALFFULL;
    }
}
int32_t HdiRingBuffer::GetRingBufferDataLen()
{
    switch (GetRingBufferStatus()) {
    case RINGBUFFER_EMPTY: {
            return 0;
        }
    case RINGBUFFER_FULL: {
            return maxBufferSize_;
        }
    case RINGBUFFER_HALFFULL:
    default: {
            if (writeIndex_ > readIndex_){
               return writeIndex_ - readIndex_;
            } else {
               return maxBufferSize_ - (readIndex_ - writeIndex_);
            }
        }
    }
}
void HdiRingBuffer::AddWriteIndex(const int32_t &length)
{
    if ((writeIndex_ + length) == maxBufferSize_) {
        writeFull_ = true;
        writeIndex_ = 0;
    } else {
        writeIndex_ += length;
        writeFull_ = false;
    }
    readFull_ = false;
}
void HdiRingBuffer::AddReadIndex(const int32_t &length)
{
    if ((readIndex_ + length) == maxBufferSize_) {
        readFull_ = true;
        readIndex_ = 0;
    } else {
        readIndex_ += length;
        readFull_ = false;
    }
}
RingBuffer HdiRingBuffer::DequeueInputBuffer()
{
    std::lock_guard<std::mutex> lock(mtx_);
    inputBuffer_.length = perFrameLength_;
    if (inputBuffer_.data != nullptr) {
         memset_s(static_cast<void *>(inputBuffer_.data), sizeof(uint8_t) * perFrameLength_,
                 0, sizeof(uint8_t) * perFrameLength_);
    } else {
       AUDIO_ERR_LOG("error: Dequeue InputBuffer failed.");
    }
    return inputBuffer_;
}

RingBuffer HdiRingBuffer::AcquireOutputBuffer()
{
    std::lock_guard<std::mutex> lock(mtx_);
    outputBuffer_.length = perFrameLength_;
    if (outputBuffer_.data != nullptr) {
         memset_s(static_cast<void *>(outputBuffer_.data), sizeof(uint8_t) * perFrameLength_,
                 0, sizeof(uint8_t) * perFrameLength_);
    } else {
       AUDIO_ERR_LOG("error: Acquire outpurtBuffer failed.");
    }
    return outputBuffer_;
}

int32_t HdiRingBuffer::ReleaseOutputBuffer(RingBuffer &item)
{
    int32_t ret = 0;
    std::lock_guard<std::mutex> lock(mtx_);
    // Onlyfixed-length(perFrameLength_) data is processed。
    if ((maxBufferSize_ - readIndex_) >= perFrameLength_ && GetRingBufferStatus() != RINGBUFFER_EMPTY) {
        ret = memmove_s(item.data, sizeof(uint8_t) * perFrameLength_,
            ringBuffer_.data + readIndex_, sizeof(uint8_t) * perFrameLength_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_READ_BUFFER, "copy ringbuffer fail");
    } else {
        AUDIO_ERR_LOG("error: Not enough data to return.");
    }
    AddReadIndex(perFrameLength_);
    return ret;
}

int32_t HdiRingBuffer::EnqueueInputBuffer(RingBuffer &item)
{
    int32_t ret = 0;
    std::lock_guard<std::mutex> lock(mtx_);
    int32_t leftSize = maxBufferSize_ - GetRingBufferDataLen();
    if (leftSize == 0) {
        ret = memmove_s(ringBuffer_.data, sizeof(uint8_t) * perFrameLength_,
                        item.data, sizeof(uint8_t) * perFrameLength_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from begin fail");
    } else if (leftSize >= perFrameLength_) {
        ret = memmove_s(ringBuffer_.data + writeIndex_, sizeof(uint8_t) * perFrameLength_,
                        item.data, sizeof(uint8_t) * perFrameLength_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from writeIndex fail");
    } else {
        AUDIO_ERR_LOG("error: Not enough data to write.");
    }
    AddWriteIndex(perFrameLength_);
    return ret;
};

} // namespace AudioStandard
} // namespace OHOS
