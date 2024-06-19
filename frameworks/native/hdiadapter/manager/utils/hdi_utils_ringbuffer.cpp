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

namespace OHOS {
namespace AudioStandard {

HdiRingBuffer::HdiRingBuffer()
{
    ringBuffer_.data = nullptr;
    ringBuffer_.length = 0;
    readIndex_ = 0;
    readFull_ = false;
    writeIndex_ = 0;;
    writeFull_ = false;
    maxBufferSize_ 0;
}
HdiRingBuffer::~HdiRingBuffer()
{
    std::lock_guard<std::mutex> lock(mtx_);
    if(ringBuffer_.data != nullptr){
        delete[] ringBuffer_.data;
        ringBuffer_.data = nullptr;
    }
}
HdiRingBuffer::Init(const int& channelNum, const int& frameLen, const int& frameNum)
{
    maxBufferSize_ = channelNum * frameLen * frameNum;
    ringBuffer_.data = new (std::nothrow)float[maxBufferSize_];
    if (ringBuffer_.data == nullptr){
          AUDIO_ERR_LOG("error: new ringBuffer_ data failed.");
    }
}
enum RINGBUFFER_STATE HdiRingBuffer::GetRingBufferStatus()
{
    if(readFull) {
        return RINGBUFFER_EMPTY;
    } else if(writeFull) {
        return RINGBUFFER_FULL;
    } else {
        return RINGBUFFER_HALFFULL;
    }
}
int32_t HdiRingBuffer::GetRingBufferDateLen()
{
    switch (expression) {
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
RingBuffer HdiRingBuffer::GetItem(const int32_t &length)
{
    RingBuffer item = {0};
    item.length = length;
    item.data = new float[length];
    if (item.data == nullptr) {
        AUDIO_ERR_LOG("error: new item data failed.");
    }
    return item;
}

int32_t HdiRingBuffer::Dequeue(RingBuffer& item)
{
   int32_t ret = 0;
   std::lock_guard<std::mutex> lock(mtx_);
   int32_t dataSize = GetRingBufferDateLen();
   int32_t readLength = (dataSize < item.length)? dataSize : item.length;
   if (readLength <= 0){
    return 0;
   }
   if ((maxBufferSize_ - readIndex_) > readLength) {
       ret = memmove_s(item.data, sizeof(float) * readLength, 
                       ringBuffer_.data + readIndex, sizeof(float) * readLength);
       CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_READ_BUFFER, "copy ringbuffer fail");
   } else {
       int32_t tailDataLength = maxBufferSize_ - readIndex_;
       ret = memmove_s(item.data, sizeof(float) * tailDataLength,
                       ringBuffer_.data + readIndex, sizeof(float) * tailDataLength);
       CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_READ_BUFFER, "copy ringbuffer tail fail");
       int32_t headDataLength = readLength - tailDataLength;
       ret = memmove_s(item.data + tailDataLength, sizeof(float) * headDataLength, 
                       ringBuffer_.data, sizeof(float) * headDataLength);
       CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_READ_BUFFER, "copy ringbuffer head fail");
       readLength = headDataLength;
   }
   AddReadIndex(readLength);
   return ret;
}

int32_t HdiRingBuffer::Enqueue(RingBuffer &item)
{
    int32_t ret = 0;
    std::lock_guard<std::mutex> lock(mtx_);
    int32_t leftSize = maxBufferSize_ - GeRingBufferDataLen();
    int32_t writeLength = (maxBufferSize_ < item.Length) ? maxBufferSize_ : item.Length;
    if (leftSize == 0) {
        ret = memmove_s(ringBuffer_.data, sizeof(float) * writeLength,
                        item.data, sizeof(float) * writeLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from begin fail");
    } else if (leftSize >= writeLength) {
        ret = memmove_s(ringBuffer_.data + writeIndex_, sizeof(float) * writeLength,
                        item.data, sizeof(float) * writeLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from writeIndex fail");
    } else if (maxBufferSize == writeLength) {
        ret = memmove_s(ringBuffer_.data + readIndex_, sizeof(float) * leftSize,
                        item.data, sizeof(float) * leftSize);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "copy ringbuffer from readIndex fail");
        int32_t headLength = writeLength - leftSize;
        ret = memmove_s(ringBuffer_.data, sizeof(float) * headLength,
                        item.data + leftSize, sizeof(float) * headLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from head fail");
        writeLength = headLength;
    } eles {
        ret = memmove_s(ringBuffer_.data + writeIndex_, sizeof(float) * leftSize,
                        item.data, sizeof(float) * leftSize);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from writeIndex fail");
        int32_t headLength = writeLength - leftSize;
        ret = memmove_s(ringBuffer_.data, sizeof(float) * headLength,
                        item.data + leftSize, sizeof(float) * headLength);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_WRITE_BUFFER, "write ringbuffer from head fail");
        writeLength = headLength;
    }
    AddWriteIndex(writeLength);
    return ret;
}

} // namespace AudioStandard
} // namespace OHOS
