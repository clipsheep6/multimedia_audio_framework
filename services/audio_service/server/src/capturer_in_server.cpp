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

#include "capturer_in_server.h"
#include <cinttypes>
#include "securec.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "i_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
}

CapturerInServer::CapturerInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener)
{
    processConfig_ = processConfig;
    streamListener_ = streamListener; // LYH waiting for review
    int32_t ret = IStreamManager::GetRecorderManager().CreateCapturer(processConfig, stream_);
    AUDIO_INFO_LOG("Construct capturerInServer result: %{public}d", ret);
    streamIndex_ = stream_->GetStreamIndex();
    ConfigServerBuffer();
}

int32_t CapturerInServer::ConfigServerBuffer()
{
    if (audioServerBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }

    stream_->GetSpanSizePerFrame(spanSizeInFrame_);
    stream_->GetMinimumBufferSize(totalSizeInFrame_);
    stream_->GetByteSizePerFrame(byteSizePerFrame_);
    if (totalSizeInFrame_ == 0 || spanSizeInFrame_ == 0 || totalSizeInFrame_ % spanSizeInFrame_ != 0) {
        AUDIO_ERR_LOG("ConfigProcessBuffer: ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }

    AUDIO_INFO_LOG("ConfigProcessBuffer: totalSizeInFrame_: %{public}u, spanSizeInFrame_: %{public}u, byteSizePerFrame_: %{public}u",
        totalSizeInFrame_, spanSizeInFrame_, byteSizePerFrame_);

    // create OHAudioBuffer in server
    audioServerBuffer_ = OHAudioBuffer::CreateFormLocal(totalSizeInFrame_, spanSizeInFrame_, byteSizePerFrame_);
    CHECK_AND_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_OPERATION_FAILED, "Create oh audio buffer failed");

    // we need to clear data buffer to avoid dirty data.
    memset_s(audioServerBuffer_->GetDataBase(), audioServerBuffer_->GetDataSize(), 0,
        audioServerBuffer_->GetDataSize());
    int32_t ret = InitBufferStatus();
    AUDIO_DEBUG_LOG("Clear data buffer, ret:%{public}d", ret);
    isBufferConfiged_ = true;
    isInited_ = true;
    return SUCCESS;
}

int32_t CapturerInServer::InitBufferStatus()
{
    if (audioServerBuffer_ == nullptr) {
        AUDIO_ERR_LOG("InitBufferStatus failed, null buffer.");
        return ERR_ILLEGAL_STATE;
    }

    uint32_t spanCount = audioServerBuffer_->GetSpanCount();
    AUDIO_INFO_LOG("InitBufferStatus: spanCount %{public}u", spanCount);
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = audioServerBuffer_->GetSpanInfoByIndex(i);
        if (spanInfo == nullptr) {
            AUDIO_ERR_LOG("InitBufferStatus failed, null spaninfo");
            return ERR_ILLEGAL_STATE;
        }
        spanInfo->spanStatus = SPAN_READ_DONE;
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    return SUCCESS;
}

void CapturerInServer::RegisterStatusCallback()
{
    AUDIO_INFO_LOG("CapturerInServer::RegisterStatusCallback");
    stream_->RegisterStatusCallback(shared_from_this());
}

void CapturerInServer::RegisterReadCallback()
{
    AUDIO_INFO_LOG("CapturerInServer::RegisterReadCallback");
    stream_->RegisterReadCallback(shared_from_this());
}

void CapturerInServer::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("CapturerInServer::OnStatusUpdate operation: %{public}d", operation);
    operation_ = operation;
    std::lock_guard<std::mutex> lock(statusLock_);
    if (status_ == I_STATUS_RELEASED) {
        AUDIO_WARNING_LOG("Stream already released");
        return;
    }
    std::shared_ptr<CapturerListener> callback = testCallback_.lock();
    switch (operation) {
        case OPERATION_UNDERFLOW:
            underflowCount += 1;
            AUDIO_INFO_LOG("Underflow!! underflow count %{public}d", underflowCount);
            break;
        case OPERATION_STARTED:
            status_ = I_STATUS_STARTED;
            break;
        case OPERATION_PAUSED:
            status_ = I_STATUS_PAUSED;
            break;
        case OPERATION_STOPPED:
            status_ = I_STATUS_STOPPED;
            break;
        case OPERATION_FLUSHED:
            if (status_ == I_STATUS_FLUSHING_WHEN_STARTED) {
                status_ = I_STATUS_STARTED;
            } else if (status_ == I_STATUS_FLUSHING_WHEN_PAUSED) {
                status_ = I_STATUS_PAUSED;
            } else {
                AUDIO_WARNING_LOG("Invalid status before flusing");
            }
            break;
        case OPERATION_RELEASED:
            status_ = I_STATUS_RELEASED;
            break;
        default:
            AUDIO_INFO_LOG("Invalid operation %{public}u", operation);
            status_ = I_STATUS_INVALID;
    }
}

BufferDesc CapturerInServer::DequeueBuffer(size_t length)
{
    return stream_->DequeueBuffer(length);
}

void CapturerInServer::ReadData(size_t length)
{
    
    std::shared_ptr<CapturerListener> callback = testCallback_.lock();
    if (callback == nullptr) {
        AUDIO_ERR_LOG("Callback from test is nullptr");
        return ;
    }
    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    AUDIO_INFO_LOG("Current write frame: %{public}" PRIu64 ", read frame: %{public}" PRIu64 ", avaliable frame:%{public}d, spanSizeInFrame:%{public}u", currentWriteFrame, currentReadFrame,
        audioServerBuffer_->GetAvailableDataFrames(), spanSizeInFrame_);
    if (audioServerBuffer_->GetAvailableDataFrames() <= spanSizeInFrame_) {
        AUDIO_INFO_LOG("OverFlow!!!");
        stream_->DequeueBuffer(length);
        callback->OnReadEvent();
        return;
    }
    BufferDesc srcBuffer = stream_->DequeueBuffer(length);
    {
        BufferDesc dstBuffer = {nullptr, 0, 0};
        uint64_t curWritePos = audioServerBuffer_->GetCurWriteFrame();
        int32_t ret = audioServerBuffer_->GetWriteBuffer(curWritePos, dstBuffer);
        if (ret < 0) {
            callback->OnReadEvent();
            return ;
        }
        memcpy_s(dstBuffer.buffer, totalSizeInFrame_, srcBuffer.buffer, totalSizeInFrame_);

        uint64_t nextWriteFrame = currentWriteFrame + spanSizeInFrame_;
        AUDIO_INFO_LOG("Read data, current write frame: %{public}" PRIu64 ", next write frame: %{public}" PRIu64 "",
            currentWriteFrame, nextWriteFrame);
        audioServerBuffer_->SetCurWriteFrame(nextWriteFrame);
        callback->OnReadEvent();
    }
    stream_->EnqueueBuffer(srcBuffer);
}

int32_t CapturerInServer::OnReadData(size_t length)
{
    AUDIO_INFO_LOG("CapturerInServer::OnReadData, length: %{public}zu", length);
    std::shared_ptr<CapturerListener> callback = testCallback_.lock();
    if (callback == nullptr) {
        AUDIO_ERR_LOG("Callback from test is nullptr");
        return -1;
    }

    ReadData(length);
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::UpdateReadIndex()
{
    AUDIO_INFO_LOG("UpdateReadIndex: audioServerBuffer_->GetAvailableDataFrames(): %{public}d, needStart: %{public}d",
        audioServerBuffer_->GetAvailableDataFrames(), needStart);
    return 0;
}

int32_t CapturerInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    buffer = audioServerBuffer_;
    return 0;
}

int32_t CapturerInServer::GetSessionId(uint32_t &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "GetSessionId failed, stream_ is null");
    sessionId = streamIndex_;
    CHECK_AND_RETURN_RET_LOG(sessionId < INT32_MAX, ERR_OPERATION_FAILED, "GetSessionId failed, sessionId:%{public}d",
        sessionId);

    return SUCCESS;
}

int32_t CapturerInServer::Start()
{
    needStart = 0;
    std::unique_lock<std::mutex> lock(statusLock_);

    if (status_ != I_STATUS_IDLE && status_ != I_STATUS_PAUSED && status_ != I_STATUS_STOPPED) {
        AUDIO_ERR_LOG("CapturerInServer::Start failed, Illegal state: %{public}u", status_);
        return -1;
    }
    status_ = I_STATUS_STARTING;
    int ret = stream_->Start();
    if (ret < 0) {
        AUDIO_ERR_LOG("Start stream failed, reason: %{public}d", ret);
        IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    resetTime_ = true;
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::Pause()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED) {
        AUDIO_ERR_LOG("CapturerInServer::Pause failed, Illegal state: %{public}u", status_);
        return -1;
    }
    status_ = I_STATUS_PAUSING;
    int ret = stream_->Pause();
    if (ret < 0) {
        AUDIO_ERR_LOG("Pause stream failed, reason: %{public}d", ret);
        IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::Flush()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED) {
        status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    } else if (status_ != I_STATUS_PAUSED) {
        status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    } else {
        AUDIO_ERR_LOG("CapturerInServer::Flush failed, Illegal state: %{public}u", status_);
        return -1;
    }

    // Flush buffer of audio server
    uint64_t writeFrame = audioServerBuffer_->GetCurWriteFrame();
    uint64_t readFrame = audioServerBuffer_->GetCurReadFrame();

    while (readFrame < writeFrame) {
        BufferDesc bufferDesc = {nullptr, 0, 0};
        int32_t readResult = audioServerBuffer_->GetReadbuffer(readFrame, bufferDesc);
        if (readResult != 0) {
            return -1;
        }
        memset_s(bufferDesc.buffer, bufferDesc.bufLength, 0, bufferDesc.bufLength);
        readFrame += spanSizeInFrame_;
        AUDIO_INFO_LOG("On flush, write frame: %{public}" PRIu64 ", nextReadFrame: %{public}u, readFrame: %{public}" PRIu64 "", writeFrame, spanSizeInFrame_, readFrame);
        audioServerBuffer_->SetCurReadFrame(readFrame);
    }

    int ret = stream_->Flush();
    if (ret < 0) {
        AUDIO_ERR_LOG("Flush stream failed, reason: %{public}d", ret);
        IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::DrainAudioBuffer()
{
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::Stop()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED && status_ != I_STATUS_PAUSED) {
        AUDIO_ERR_LOG("CapturerInServer::Stop failed, Illegal state: %{public}u", status_);
        return -1;
    }
    status_ = I_STATUS_STOPPING;
    int ret = stream_->Stop();
    if (ret < 0) {
        AUDIO_ERR_LOG("Stop stream failed, reason: %{public}d", ret);
        IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::Release()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    status_ = I_STATUS_RELEASED;

    int32_t ret = IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
    stream_ = nullptr;
    if (ret < 0) {
        AUDIO_ERR_LOG("Release stream failed, reason: %{public}d", ret);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    return PA_ADAPTER_SUCCESS;
}

int32_t CapturerInServer::GetAudioTime(uint64_t &framePos, uint64_t &timeStamp)
{
    if (status_ == I_STATUS_STOPPED) {
        AUDIO_WARNING_LOG("Current status is stopped");
        return -1;
    }
    stream_->GetStreamFramesRead(framePos);
    stream_->GetCurrentTimeStamp(timeStamp);
    if (resetTime_) {
        resetTime_ = false;
        resetTimestamp_ = timeStamp;
    }
    return 0;
}

int32_t CapturerInServer::GetLatency(uint64_t &latency)
{
    return stream_->GetLatency(latency);
}


void CapturerInServer::RegisterTestCallback(const std::weak_ptr<CapturerListener> &callback)
{
    testCallback_ = callback;
}

int32_t CapturerInServer::ReadOneFrame()
{
    // if buffer ready, get buffer, else, return
    size_t minBufferSize = 0;
    if (stream_->GetMinimumBufferSize(minBufferSize) < 0) {
        AUDIO_ERR_LOG("Get min buffer size err");
        return -1;
    }
    BufferDesc bufferDesc = stream_->DequeueBuffer(minBufferSize);
    stream_->EnqueueBuffer(bufferDesc);
    return 0;
}

int32_t CapturerInServer::AbortOneCallback()
{
    std::lock_guard<std::mutex> lock(statusLock_);
    stream_->AbortCallback(1);
    return 0;
}

int32_t CapturerInServer::AbortAllCallback()
{
    std::lock_guard<std::mutex> lock(statusLock_);
    stream_->AbortCallback(5);
    return 0;
}

std::shared_ptr<OHAudioBuffer> CapturerInServer::GetOHSharedBuffer()
{
    return audioServerBuffer_;
}
} // namespace AudioStandard
} // namespace OHOS
