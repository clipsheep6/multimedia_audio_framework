/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCapturerSourceWakeup"

#include "audio_capturer_source_wakeup.h"

#include "securec.h"

#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

static inline void MemcpysAndCheck(void *dest, size_t destMax, const void *src, size_t count)
{
    if (memcpy_s(dest, destMax, src, count)) {
        AUDIO_ERR_LOG("memcpy_s error");
    }
}

AudioCapturerSourceWakeup::WakeupBuffer(size_t sizeMax = BUFFER_SIZE_MAX)
    : sizeMax_(sizeMax), buffer_(std::make_unique<char[]>(sizeMax))
{
}

int32_t AudioCapturerSourceWakeup::WakeupBuffer::Poll(char *frame, uint64_t requestBytes, uint64_t &replyBytes, uint64_t &noStart)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (noStart < headNum_) {
        noStart = headNum_;
    }

    if (noStart >= (headNum_ + size_)) {
        if (requestBytes > sizeMax_) {
            requestBytes = sizeMax_;
        }

        int32_t res = audioCapturerSource_.CaptureFrame(frame, requestBytes, replyBytes);
        Offer(frame, replyBytes);

        return res;
    }

    if (requestBytes > size_) { // size_!=0
        replyBytes = size_;
    } else {
        replyBytes = requestBytes;
    }

    uint64_t tail = (head_ + size_) % sizeMax_;

    if (tail > head_) {
        MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, replyBytes);
        headNum_ += replyBytes;
        size_ -= replyBytes;
        head_ = (head_ + replyBytes) % sizeMax_;
    } else {
        uint64_t copySize = min((sizeMax_ - head_), replyBytes);
        if (copySize != 0) {
            MemcpysAndCheck(frame, replyBytes, buffer_.get() + head_, copySize);
            headNum_ += copySize;
            size_ -= copySize;
            head_ = (head_ + copySize) % sizeMax_;
        }

        uint64_t remainCopySize = replyBytes - copySize;
        if (remainCopySize != 0) {
            MemcpysAndCheck(frame + copySize, remainCopySize, buffer_.get(), remainCopySize);
            headNum_ += remainCopySize;
            size_ -= remainCopySize;
            head_ = (head_ + remainCopySize) % sizeMax_;
        }
    }

    return SUCCESS;
}

void AudioCapturerSourceWakeup::WakeupBuffer::Offer(const char *frame, const uint64_t bufferBytes)
{
    if ((size_ + bufferBytes) > sizeMax_) { // head_ need shift
        u_int64_t shift = (size_ + bufferBytes) - sizeMax_; // 1 to sizeMax_
        headNum_ += shift;
        if (size_ > shift) {
            size_ -= shift;
            head_ = ((head_ + shift) % sizeMax_);
        } else {
            size_ = 0;
            head_ = 0;
        }
    }

    uint64_t tail = (head_ + size_) % sizeMax_;
    if (tail < head_) {
        MemcpysAndCheck((buffer_.get() + tail), bufferBytes, frame, bufferBytes);
    } else {
        uint64_t copySize = min(sizeMax_ - tail, bufferBytes);
        MemcpysAndCheck((buffer_.get() + tail), sizeMax_ - tail, frame, copySize);

        if (copySize < bufferBytes) {
            MemcpysAndCheck((buffer_.get()), bufferBytes - copySize, frame + copySize, bufferBytes - copySize);
        }
    }
    size_ += bufferBytes;
}

int32_t AudioCapturerSourceWakeup::Init(const IAudioSourceAttr &attr)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isInited) {
        return res;
    }
    noStart_ = 0;
    if (initCount == 0) {
        if (wakeupBuffer_ == nullptr) {
            wakeupBuffer_ = std::make_unique<WakeupBuffer>();
        }
        res = audioCapturerSource_.Init(attr);
    }
    if (res == SUCCESS) {
        isInited = true;
        initCount++;
    }
    return res;
}

bool AudioCapturerSourceWakeup::IsInited(void)
{
    return isInited;
}

void AudioCapturerSourceWakeup::DeInit(void)
{
    AudioXCollie wakeupXCollie("AudioCapturerSourceWakeup::DeInit", DEINIT_TIME_OUT_SECONDS);
    AUDIO_INFO_LOG("Start deinit of source wakeup");
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    if (!isInited) {
        return;
    }
    isInited = false;
    initCount--;
    if (initCount == 0) {
        wakeupBuffer_.reset();
        audioCapturerSource_.DeInit();
    }
}

int32_t AudioCapturerSourceWakeup::Start(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (isStarted) {
        return res;
    }
    if (startCount == 0) {
        res = audioCapturerSource_.Start();
    }
    if (res == SUCCESS) {
        isStarted = true;
        startCount++;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Stop(void)
{
    std::lock_guard<std::mutex> lock(wakeupMutex_);
    int32_t res = SUCCESS;
    if (!isStarted) {
        return res;
    }
    if (startCount == 1) {
        res = audioCapturerSource_.Stop();
    }
    if (res == SUCCESS) {
        isStarted = false;
        startCount--;
    }
    return res;
}

int32_t AudioCapturerSourceWakeup::Flush(void)
{
    return audioCapturerSource_.Flush();
}

int32_t AudioCapturerSourceWakeup::Reset(void)
{
    return audioCapturerSource_.Reset();
}

int32_t AudioCapturerSourceWakeup::Pause(void)
{
    return audioCapturerSource_.Pause();
}

int32_t AudioCapturerSourceWakeup::Resume(void)
{
    return audioCapturerSource_.Resume();
}

int32_t AudioCapturerSourceWakeup::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t res = wakeupBuffer_->Poll(frame, requestBytes, replyBytes, noStart_);
    noStart_ += replyBytes;
    return res;
}

int32_t AudioCapturerSourceWakeup::SetVolume(float left, float right)
{
    return audioCapturerSource_.SetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::GetVolume(float &left, float &right)
{
    return audioCapturerSource_.GetVolume(left, right);
}

int32_t AudioCapturerSourceWakeup::SetMute(bool isMute)
{
    return audioCapturerSource_.SetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::GetMute(bool &isMute)
{
    return audioCapturerSource_.GetMute(isMute);
}

int32_t AudioCapturerSourceWakeup::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    return audioCapturerSource_.SetAudioScene(audioScene, activeDevice);
}

int32_t AudioCapturerSourceWakeup::SetInputRoute(DeviceType inputDevice)
{
    return audioCapturerSource_.SetInputRoute(inputDevice);
}

uint64_t AudioCapturerSourceWakeup::GetTransactionId()
{
    return audioCapturerSource_.GetTransactionId();
}

int32_t AudioCapturerSourceWakeup::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    return audioCapturerSource_.GetPresentationPosition(frames, timeSec, timeNanoSec);
}

std::string AudioCapturerSourceWakeup::GetAudioParameter(const AudioParamKey key,
                                                         const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

void AudioCapturerSourceWakeup::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    audioCapturerSource_.RegisterWakeupCloseCallback(callback);
}

void AudioCapturerSourceWakeup::RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback)
{
    audioCapturerSource_.RegisterAudioCapturerSourceCallback(std::move(callback));
}

void AudioCapturerSourceWakeup::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("AudioCapturerSourceWakeup: RegisterParameterCallback is not supported!");
}

float AudioCapturerSourceWakeup::GetMaxAmplitude()
{
    return audioCapturerSource_.GetMaxAmplitude();
}

int32_t AudioCapturerSourceWakeup::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    return audioCapturerSource_.UpdateAppsUid(appsUid, size);
}

int32_t AudioCapturerSourceWakeup::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return audioCapturerSource_.UpdateAppsUid(appsUid);
}

} // namespace AudioStandard
} // namesapce OHOS
