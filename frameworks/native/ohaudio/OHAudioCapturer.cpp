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

#include "OHAudioCapturer.h"
#include "audio_errors.h"

using OHOS::AudioStandard::Timestamp;

static OHOS::AudioStandard::OHAudioCapturer *convertCapturer(OH_AudioCapturer* capturer)
{
    return (OHOS::AudioStandard::OHAudioCapturer*) capturer;
}

OH_AudioStream_Result OH_AudioCapturer_Release(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    if (audioCapturer->Release()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Start(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    if (audioCapturer->Start()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Pause(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Pause()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Stop(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Stop()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_Flush(OH_AudioCapturer* capturer)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    if (audioCapturer->Flush()) {
        return AUDIOSTREAM_SUCCESS;
    } else {
        return AUDIOSTREAM_ERROR_ILLEGAL_STATE;
    }
}

OH_AudioStream_Result OH_AudioCapturer_GetLatencyMode(OH_AudioCapturer* capturer,
    OH_AudioStream_LatencyMode* latencyMode)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *latencyMode = (OH_AudioStream_LatencyMode)capturerInfo.capturerFlags;
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetStreamId(OH_AudioCapturer* capturer, uint32_t* streamId)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    audioCapturer->GetStreamId(*streamId);
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetChannelCount(OH_AudioCapturer* capturer, int32_t* channelCount)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *channelCount = audioCapturer->GetChannelCount();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSamplingRate(OH_AudioCapturer* capturer, int32_t* rate)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");

    *rate = audioCapturer->GetSamplingRate();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetSampleFormat(OH_AudioCapturer* capturer,
    OH_AudioStream_SampleFormat* sampleFormat)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *sampleFormat = (OH_AudioStream_SampleFormat)audioCapturer->GetSampleFormat();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetEncodingType(OH_AudioCapturer* capturer,
    OH_AudioStream_EncodingType* encodingType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    *encodingType = (OH_AudioStream_EncodingType)audioCapturer->GetEncodingType();
    return AUDIOSTREAM_SUCCESS;
}

OH_AudioStream_Result OH_AudioCapturer_GetCapturerInfo(OH_AudioCapturer* capturer,
    OH_AudioStream_SourceType* sourceType)
{
    OHOS::AudioStandard::OHAudioCapturer *audioCapturer = convertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(audioCapturer != nullptr, AUDIOSTREAM_ERROR_INVALID_PARAM, "convert capturer failed");
    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    audioCapturer->GetCapturerInfo(capturerInfo);
    *sourceType = (OH_AudioStream_SourceType)capturerInfo.sourceType;
    return AUDIOSTREAM_SUCCESS;
}

namespace OHOS {
namespace AudioStandard {
OHAudioCapturer::OHAudioCapturer()
{
    AUDIO_INFO_LOG("OHAudioCapturer created!");
}

OHAudioCapturer::~OHAudioCapturer()
{
    AUDIO_INFO_LOG("OHAudioCapturer destroyed!");
}

bool OHAudioCapturer::Initialize(const AudioCapturerOptions& capturerOptions)
{
    audioCapturer_ = AudioCapturer::Create(capturerOptions);
    if (audioCapturer_ == nullptr) {
        return false;
    }
    return true;
}

bool OHAudioCapturer::Start()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Start();
}
    
bool OHAudioCapturer::Pause()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Pause();
}

bool OHAudioCapturer::Stop()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Stop();
}

bool OHAudioCapturer::Flush()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Flush();
}

bool OHAudioCapturer::Release()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Release();
}

void OHAudioCapturer::GetStreamId(uint32_t &streamId)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->GetAudioStreamId(streamId);
}

AudioChannel OHAudioCapturer::GetChannelCount()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioChannel;
}

int32_t OHAudioCapturer::GetSamplingRate()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, MONO, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.samplingRate;
}

AudioEncodingType OHAudioCapturer::GetEncodingType()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ENCODING_INVALID, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioEncoding;
}

void OHAudioCapturer::GetCapturerInfo(AudioCapturerInfo& capturerInfo)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->GetCapturerInfo(capturerInfo);
}

AudioSampleFormat OHAudioCapturer::GetSampleFormat()
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, INVALID_WIDTH, "capturer client is nullptr");
    AudioCapturerParams params;
    audioCapturer_->GetParams(params);
    return params.audioSampleFormat;
}

void OHAudioCapturer::SetCapturerReadCallback(OH_AudioCapturer_Callbacks callbacks, void* userData)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    if (callbacks.OH_AudioCapturer_OnReadData != nullptr) {
        std::shared_ptr<AudioCapturerReadCallback> callback = std::make_shared<OHAudioCapturerModeCallback>(callbacks,
            (OH_AudioCapturer*)this, userData);
        audioCapturer_->SetCapturerReadCallback(callback);
    } else {
        AUDIO_ERR_LOG("read callback is nullptr");
    }
}

void OHAudioCapturer::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_LOG(audioCapturer_ != nullptr, "capturer client is nullptr");
    audioCapturer_->GetAudioTime(timestamp, base);
}

int32_t OHAudioCapturer::GetBufferDesc(BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->GetBufferDesc(bufDesc);
}

int32_t OHAudioCapturer::Enqueue(const BufferDesc &bufDesc) const
{
    CHECK_AND_RETURN_RET_LOG(audioCapturer_ != nullptr, ERROR, "capturer client is nullptr");
    return audioCapturer_->Enqueue(bufDesc);
}

void OHAudioCapturerModeCallback::OnReadData(size_t length)
{
    OHAudioCapturer* audioCapturer = (OHAudioCapturer*)ohAudioCapturer_;
    CHECK_AND_RETURN_LOG(audioCapturer != nullptr, "capturer client is nullptr");
    CHECK_AND_RETURN_LOG(callbacks_.OH_AudioCapturer_OnReadData != nullptr, "pointer to the fuction is nullptr");
    BufferDesc bufDesc;
    audioCapturer->GetBufferDesc(bufDesc);
    callbacks_.OH_AudioCapturer_OnReadData(ohAudioCapturer_,
        userData_,
        (void*)bufDesc.buffer,
        bufDesc.bufLength);
    audioCapturer->Enqueue(bufDesc);
}
}  // namespace AudioStandard
}  // namespace OHOS
