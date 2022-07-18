/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


#include <sys/time.h>
#include <utility>

#include <cmath>
#include "securec.h"
#include "audio_log.h"
#include "audio_policy_manager.h"
#include "tone_player_private.h"


namespace OHOS {
namespace AudioStandard {
constexpr int32_t C20MS = 20;
constexpr int32_t C1000MS = 1000;
constexpr int32_t CMAXWAIT = 3;
constexpr int32_t CDOUBLE = 2;
constexpr int32_t AMPLITUDE = 16000;
constexpr int32_t BIT8 = 8;

#ifdef DUMPFILE
const char *g_tonePlayerTestFilePath = "/data/local/tmp/toneplayer_test.pcm";
#endif // DUMPFILE

std::shared_ptr<TonePlayer> TonePlayer::Create(const AudioRendererInfo &rendererInfo)
{
    return std::make_shared <TonePlayerPrivate>("", rendererInfo);
}

std::shared_ptr<TonePlayer> TonePlayer::Create(const std::string cachePath, const AudioRendererInfo &rendererInfo)
{
    return std::make_shared <TonePlayerPrivate>(cachePath, rendererInfo);
}

TonePlayerPrivate::TonePlayerPrivate(const std::string cachePath, const AudioRendererInfo &rendereInfo)
{
    tonePlayerState_ = TONE_PLAYER_IDLE;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
    rendererOptions.streamInfo.format = SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = MONO;
    rendererOptions.rendererInfo.contentType = rendereInfo.contentType; // contentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = rendereInfo.streamUsage; // streamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = rendereInfo.rendererFlags;
    supportedTones_ = AudioPolicyManager::GetInstance().GetSupportedTones();
    volume_ = TRACK_VOLUME;
    toneInfo_ = NULL;
    initialToneInfo_ = NULL;
    samplingRate_ = rendererOptions.streamInfo.samplingRate;
    if (!cachePath.empty()) {
        AUDIO_INFO_LOG("copy application cache path");
        cachePath_.assign(cachePath);
    }
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  volume=%{public}f",  volume_);
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  size=%{public}zu",  supportedTones_.size());
#ifdef DUMPFILE
    pfd = nullptr;
#endif // DUMPFILE
}


TonePlayer::~TonePlayer() = default;

TonePlayerPrivate::~TonePlayerPrivate()
{
    AUDIO_INFO_LOG("TonePlayerPrivate destructor");

    if (audioRenderer_ != nullptr) {
        StopTone();
        audioRenderer_->Clear();
    }
    toneDataGenLoop_.join();
    audioRenderer_ = nullptr;
}

bool TonePlayerPrivate::LoadEventStateHandler()
{
    if (tonePlayerState_ == TONE_PLAYER_IDLE) {
        AUDIO_INFO_LOG("Load Init AudioRenderer");
        if (!InitAudioRenderer()) {
            return false;
        }
        tonePlayerState_ = TONE_PLAYER_INIT;
    }
    if (InitToneWaveInfo() == false) {
        AUDIO_INFO_LOG("Wave Initialization Failed");
        tonePlayerState_ = TONE_PLAYER_IDLE;
        return false;
    }
    tonePlayerState_ = TONE_PLAYER_STARTING;
    toneDataState_ = TONE_DATA_LOADING;
    toneDataGenLoop_ = std::thread(&TonePlayerPrivate::AudioToneDataThreadFunc, this);
    return true;
}

bool TonePlayerPrivate::PlayEventStateHandler()
{
    status_t retStatus;
    mutexLock_.unlock();
    audioRenderer_->Start();
    mutexLock_.lock();
    if (tonePlayerState_ == TONE_PLAYER_STARTING) {
        mutexLock_.unlock();
        std::unique_lock<std::mutex> lock(cbkCondLock_);
        retStatus = waitAudioCbkCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
        AUDIO_INFO_LOG("Immediate start got notified");
        mutexLock_.lock();
        if (retStatus == std::cv_status::timeout) {
            AUDIO_ERR_LOG("--- Immediate start timed out, status %{public}d", retStatus);
            return false;
        }
    }
    return true;
}

bool TonePlayerPrivate::TonePlayerStateHandler(int16_t event)
{
    AUDIO_INFO_LOG("TonePlayerPrivate HandlePlayerState");
    switch (event) {
        case PLAYER_EVENT_LOAD:
            if (LoadEventStateHandler() == false) {
                return false;
            }
            break;
        case PLAYER_EVENT_PLAY:
            if (PlayEventStateHandler() == false) {
                tonePlayerState_ = TONE_PLAYER_IDLE;
                return false;
            }
            AUDIO_INFO_LOG("STARTTONE tonePlayerState_ %{public}d", tonePlayerState_);
            break;
        case PLAYER_EVENT_STOP:
            StopEventStateHandler();
            break;
    }
    return true;
}


bool TonePlayerPrivate::LoadTone(ToneType toneType)
{
    bool lRet = false;
    AUDIO_INFO_LOG("LoadTone tonePlayerState_ %{public}d", tonePlayerState_);
    if (toneType >= NUM_TONES) {
        return lRet;
    }
    mutexLock_.lock();
    if (tonePlayerState_ != TONE_PLAYER_IDLE && tonePlayerState_ != TONE_PLAYER_INIT) {
        mutexLock_.unlock();
        return lRet;
    }

    if (std::find(supportedTones_.begin(), supportedTones_.end(), (int32_t)toneType) == supportedTones_.end()) {
        mutexLock_.unlock();
        return lRet;
    }
    // Get descriptor for requested tone
    initialToneInfo_ = AudioPolicyManager::GetInstance().GetToneConfig(toneType);
    lRet = TonePlayerStateHandler(PLAYER_EVENT_LOAD);
    mutexLock_.unlock();
#ifdef DUMPFILE
    pfd = fopen(g_tonePlayerTestFilePath, "wb+");
    if (pfd == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
    }
#endif // DUMPFILE
    return true;
}

bool TonePlayerPrivate::StartTone()
{
    bool retVal = false;
    AUDIO_INFO_LOG("STARTTONE tonePlayerState_ %{public}d", tonePlayerState_);
    mutexLock_.lock();
    if (tonePlayerState_ == TONE_PLAYER_IDLE || tonePlayerState_ == TONE_PLAYER_INIT) {
        if (LoadEventStateHandler() == false) {
            mutexLock_.unlock();
            return false;
        }
    }
    retVal = TonePlayerStateHandler(PLAYER_EVENT_PLAY);
    mutexLock_.unlock();
    return retVal;
}

bool TonePlayerPrivate::Release()
{
    if (audioRenderer_ != nullptr) {
        StopTone();
        audioRenderer_->Clear();
        tonePlayerState_ = TONE_PLAYER_IDLE;
    }
    audioRenderer_ = nullptr;
    return true;
}

bool TonePlayerPrivate::StopTone()
{
    bool retVal = true;
    AUDIO_INFO_LOG("StopTone tonePlayerState_ %{public}d", tonePlayerState_);
    mutexLock_.lock();
    if (tonePlayerState_ == TONE_PLAYER_IDLE  || tonePlayerState_ == TONE_PLAYER_INIT) {
        AUDIO_INFO_LOG("-stop tone End");
        mutexLock_.unlock();
        return retVal;
    }

    retVal = TonePlayerStateHandler(PLAYER_EVENT_STOP);
    mutexLock_.unlock();
    audioRenderer_->Stop(); // Newly added
#ifdef DUMPFILE
    if (pfd) {
        fclose(pfd);
        pfd = nullptr;
    }
#endif // DUMPFILE
    return retVal;
}

bool TonePlayerPrivate::StopEventStateHandler()
{
    if (tonePlayerState_ == TONE_PLAYER_PLAYING || tonePlayerState_ == TONE_PLAYER_STARTING) {
        tonePlayerState_ = TONE_PLAYER_STOPPING;
    }
    AUDIO_INFO_LOG("WAITING wait_for cond");
    mutexLock_.unlock();
    std::unique_lock<std::mutex> lock(cbkCondLock_);
    status_t retStatus = waitAudioCbkCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
    mutexLock_.lock();
    if (retStatus != std::cv_status::timeout) {
        AUDIO_ERR_LOG("--- StopTone waiting wait_for cond got notified");
        tonePlayerState_ = TONE_PLAYER_INIT;
    } else {
        AUDIO_ERR_LOG("--- Stop timed out");
        tonePlayerState_ = TONE_PLAYER_IDLE;
    }
    return true;
}


bool TonePlayerPrivate::InitAudioRenderer()
{
    processSize_ = (rendererOptions.streamInfo.samplingRate * C20MS) / C1000MS;
    if (!cachePath_.empty()) {
        audioRenderer_ = AudioRenderer::Create(cachePath_, rendererOptions);
    } else {
        audioRenderer_ = AudioRenderer::Create(rendererOptions);
    }
    if (audioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("initAudioRenderer: Renderer create failed");
        return false;
    }

    AUDIO_INFO_LOG("initAudioRenderer: Playback renderer created");
    if (audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK)) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRenderMode failed");
        return false;
    }
    AUDIO_INFO_LOG("initAudioRenderer: SetRenderMode Sucessful");
    if (audioRenderer_->SetRendererWriteCallback(shared_from_this())) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRendererWriteCallback failed");
        return false;
    }
    AUDIO_INFO_LOG("initAudioRenderer: SetRendererWriteCallback Sucessful");
    if (audioRenderer_->SetRendererCallback(shared_from_this())) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRendererCallbackfailed");
        return false;
    }

    AUDIO_INFO_LOG("initAudioRenderer: SetRendererCallback Sucessful");
    audioRenderer_->SetVolume(volume_);
    AUDIO_INFO_LOG("initAudioRenderer: SetVolume Sucessful");
    return true;
}

void TonePlayerPrivate::OnInterrupt(const InterruptEvent &interruptEvent)
{
}

void TonePlayerPrivate::OnWriteData(size_t length)
{
    AUDIO_INFO_LOG("OnWriteData Cbk : %{public}zu", length);
    AudioToneRendererCallback();
}

void TonePlayerPrivate::OnStateChange(const RendererState state)
{
    AUDIO_INFO_LOG("OnStateChange Cbk : %{public}d not calling", state);
    if (state == RENDERER_RUNNING || state == RENDERER_STOPPED) {
    }
}

bool TonePlayerPrivate::CheckToneStopped()
{
    std::shared_ptr<ToneInfo> toneDesc = toneInfo_;
    AUDIO_INFO_LOG("CheckToneStopped state : %{public}d tot : %{public}d max : %{public}d not calling",
        tonePlayerState_, totalSample_, maxSample_);
    if (tonePlayerState_ == TONE_PLAYER_STOPPED) {
        return true;
    }
    if (toneDesc->segments[currSegment_].duration == 0 ||
        totalSample_ > maxSample_ || tonePlayerState_ == TONE_PLAYER_STOPPING) {
        if (tonePlayerState_ == TONE_PLAYER_PLAYING) {
            tonePlayerState_ = TONE_PLAYER_STOPPING;
            AUDIO_INFO_LOG("Audicallback move playing to stoping");
        }
        return true;
    }
    return false;
}

bool TonePlayerPrivate::CheckToneStarted(uint32_t reqSample, int8_t *audioBuffer)
{
    std::shared_ptr<ToneInfo> toneDesc = toneInfo_;
    if (tonePlayerState_ == TONE_PLAYER_STARTING) {
            tonePlayerState_ = TONE_PLAYER_PLAYING;
            AUDIO_INFO_LOG("Audicallback move to playing");
        if (toneDesc->segments[currSegment_].duration != 0) {
            sampleCount_ = 0;
            GetSamples(toneDesc->segments[currSegment_].waveFreq, audioBuffer, reqSample);
        }
        AUDIO_INFO_LOG("CheckToneStarted GenerateStartWave to currseg: %{public}d ", currSegment_);
        return true;
    }
    return false;
}
void TonePlayerPrivate::GetCurrentSegmentUpdated(std::shared_ptr<ToneInfo> toneDesc)
{
    if (toneDesc->segments[currSegment_].loopCnt) {
        if (loopCounter_ < toneDesc->segments[currSegment_].loopCnt) {
            currSegment_ = toneDesc->segments[currSegment_].loopIndx;
            ++loopCounter_;
        } else {
         // completed loop. go to next segment
            loopCounter_ = 0;
            currSegment_++;
        }
    } else {
        // no looping required , go to next segment
        currSegment_++;
    }
    AUDIO_INFO_LOG("GetCurrentSegmentUpdated: %{public}d, %{public}d",
        loopCounter_, currSegment_);
}

bool TonePlayerPrivate::CheckToneContinuity ()
{
    bool retVal = false;
    AUDIO_INFO_LOG("CheckToneContinuity Entry: %{public}d, %{public}d",
        loopCounter_, currSegment_);
    std::shared_ptr<ToneInfo> toneDesc = toneInfo_;
    GetCurrentSegmentUpdated(toneDesc);
    // Handle loop if last segment reached
    if (toneDesc->segments[currSegment_].duration == 0) {
        AUDIO_INFO_LOG("Last Seg: %{public}d", currSegment_);
        if (currCount_ < toneDesc->repeatCnt) {
            AUDIO_INFO_LOG("Repeating Count: %{public}d", currCount_);
            currSegment_ = toneDesc->repeatSegment;
            ++currCount_;
            retVal = true;
        } else {
            retVal = false;
        }
    } else {
        retVal = true;
    }
    AUDIO_INFO_LOG("CheckToneContinuity End: loopCounter_ %{public}d, currSegment_ %{public}d currCount_ %{public}d",
        loopCounter_, currSegment_, currCount_);
    return retVal;
}

bool TonePlayerPrivate::ContinueToneplay(uint32_t reqSample, int8_t *audioBuffer)
{
    std::shared_ptr<ToneInfo> toneDesc = toneInfo_;
    if (tonePlayerState_ != TONE_PLAYER_PLAYING) {
        return false;
    }
    if (totalSample_ <= nextSegSample_) {
        AUDIO_INFO_LOG("ContinueToneplay Continue Playing Tone");
        if (toneDesc->segments[currSegment_].duration != 0) {
            GetSamples(toneDesc->segments[currSegment_].waveFreq, audioBuffer, reqSample);
        }
        return true;
    }
    AUDIO_INFO_LOG(" Current Seg Last minute Playing Tone");
    if (toneDesc->segments[currSegment_].duration != 0) {
        GetSamples(toneDesc->segments[currSegment_].waveFreq, audioBuffer, reqSample);
    }

    if (CheckToneContinuity()) {
        if (toneDesc->segments[currSegment_].duration != 0) {
            sampleCount_ = 0;
            GetSamples(toneDesc->segments[currSegment_].waveFreq, audioBuffer, reqSample);
        }
    }
    nextSegSample_
        += (toneDesc->segments[currSegment_].duration * samplingRate_) / C1000MS;
    AUDIO_INFO_LOG("ContinueToneplay nextSegSample_ %{public}d", nextSegSample_);
    return true;
}

void TonePlayerPrivate::AudioToneRendererCallback()
{
    std::unique_lock<std::mutex> lock(dataCondLock_);
    AUDIO_INFO_LOG("AudioToneRendererCallback Entered");
    if (toneDataState_ == TONE_DATA_LOADED) {
        AUDIO_INFO_LOG("Notifing Data AudioRendererDataThreadFunc");
        waitToneDataCond_.notify_all();
    }
    if (toneDataState_ == TONE_DATA_LOADING) {
        toneDataState_ = TONE_DATA_REQUESTED;
        waitToneDataCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
    }
    AUDIO_INFO_LOG("AudioToneRendererCallback Exited");
}

bool TonePlayerPrivate::AudioToneSequenceGen(BufferDesc &bufDesc)
{
    int8_t *audioBuffer = (int8_t *)bufDesc.buffer;
    uint32_t totalBufAvailable = bufDesc.bufLength /sizeof(int16_t);
    while (totalBufAvailable) {
        uint32_t reqSamples = totalBufAvailable < processSize_* CDOUBLE ? totalBufAvailable : processSize_;
        bool lSignal = false;
        AUDIO_INFO_LOG("AudioToneDataThreadFunc, lReqSmp : %{public}d totalBufAvailable : %{public}d",
            reqSamples, totalBufAvailable);
        mutexLock_.lock();
        // Update pcm frame count and end time (current time at the end of this process)
        totalSample_ += reqSamples;
        if (CheckToneStopped()) {
            if (tonePlayerState_ == TONE_PLAYER_STOPPING) {
                tonePlayerState_ = TONE_PLAYER_STOPPED;
                mutexLock_.unlock();
                break;
            }
            if (tonePlayerState_ == TONE_PLAYER_STOPPED) {
                tonePlayerState_ = TONE_PLAYER_INIT;
                totalBufAvailable = 0;
                bufDesc.dataLength = 0;
                AUDIO_INFO_LOG("Notifing all the STOP");
                mutexLock_.unlock();
                waitAudioCbkCond_.notify_all();
                audioRenderer_->Stop();
                return false;
            }
        } else if (CheckToneStarted(reqSamples, audioBuffer)) {
            bufDesc.dataLength += reqSamples*sizeof(int16_t);
            lSignal = true;
        } else {
            if (ContinueToneplay(reqSamples, audioBuffer)) {
                bufDesc.dataLength += reqSamples*sizeof(int16_t);
            }
        }
        totalBufAvailable -= reqSamples;
        audioBuffer += reqSamples*sizeof(int16_t);
        mutexLock_.unlock();
        if (lSignal) {
            AUDIO_INFO_LOG("Notifing all the");
            waitAudioCbkCond_.notify_all();
        }
    }
    return true;
}

void TonePlayerPrivate::AudioToneDataThreadFunc()
{
    while (true) {
        BufferDesc bufDesc = {};
        audioRenderer_->GetBufferDesc(bufDesc);
        std::shared_ptr<ToneInfo> lpToneDesc = toneInfo_;
        bufDesc.dataLength = 0;
        AUDIO_INFO_LOG("AudioToneDataThread, buflen: %{public}zu ", bufDesc.bufLength);
        AUDIO_INFO_LOG("AudioToneDataThreadFunc, tonePlayerState_ : %{public}d", tonePlayerState_);
        if (bufDesc.bufLength == 0) return;
        // Clear output buffer: WaveGenerator accumulates into audioBuffer buffer
        memset_s(bufDesc.buffer, bufDesc.bufLength, 0, bufDesc.bufLength);
        if (AudioToneSequenceGen(bufDesc) == false) {
            return;
        }
        AUDIO_INFO_LOG("Exiting the AudioToneDataThreadFunc %{public}zu,tonePlayerState_ %{public}d ",
            bufDesc.dataLength, tonePlayerState_);
        if (bufDesc.dataLength) {
#ifdef DUMPFILE
            size_t writeResult = fwrite((void*)bufDesc.buffer, 1, bufDesc.dataLength, pfd);
            if (writeResult != bufDesc.dataLength) {
                AUDIO_ERR_LOG("Failed to write the file.");
            }
#endif // DUMPFILE
            std::unique_lock<std::mutex> lock(dataCondLock_);
            if (toneDataState_ == TONE_DATA_LOADING) {
                toneDataState_ = TONE_DATA_LOADED;
                waitToneDataCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
            }
            audioRenderer_->Enqueue(bufDesc);
            if (toneDataState_ == TONE_DATA_REQUESTED) {
                AUDIO_INFO_LOG("Notifing Data AudioRendererCallback");
                waitToneDataCond_.notify_all();
            }
            toneDataState_ = TONE_DATA_LOADING;
        }
    }
}

bool TonePlayerPrivate::InitToneWaveInfo()
{
    AUDIO_INFO_LOG("InitToneWaveInfo, Entered");
    if (initialToneInfo_ == NULL) {
        return false;
    }
    toneInfo_ = initialToneInfo_;
    maxSample_ = TONEINFO_INF;
    // Initialize tone sequencer
    totalSample_ = 0;
    currSegment_ = 0;
    currCount_ = 0;
    loopCounter_ = 0;
    if (toneInfo_->segments[0].duration == TONEINFO_INF) {
        nextSegSample_ = TONEINFO_INF;
    } else {
        nextSegSample_ = (toneInfo_->segments[0].duration * samplingRate_) / C1000MS;
    }
    AUDIO_INFO_LOG("Prepare wave, nextSegSample_ %{public}d", nextSegSample_);
    return true;
}

int32_t TonePlayerPrivate::GetSamples(uint16_t *freqs, int8_t* buffer, uint32_t samples)
{
    uint32_t index;
    uint8_t* data;
    uint16_t freqVal;
    for (uint32_t i = 0; i <= TONEINFO_MAX_WAVES; i++) {
        if (freqs[i] == 0) {
            break;
        }
        freqVal = freqs[i];
        AUDIO_INFO_LOG("GetSamples: Freq %{public}d  sampleCount_ %{public}d", freqVal, sampleCount_);
        index = sampleCount_;
        data = (uint8_t*)buffer;
        double factor = freqVal * 2 * 3.1428 / samplingRate_;
        for (uint32_t idx = 0; idx < samples; idx++) {
            int16_t a1 = AMPLITUDE * sin(factor * index);
            int16_t adata = (int16_t) a1;
            uint16_t b1;
            if (i == 0) {
                b1 = (adata & 0xFF);
                *data = b1;
                data++;
                *data = ((adata & 0xFF00) >> BIT8);
                data++;
            } else {
                b1 = *data + (adata & 0xFF);
                *data = b1 & 0xFF;
                data++;
                *data += (b1 >> BIT8) + ((adata & 0xFF00) >> BIT8);
                data++;
            }
            index++;
        }
    }
    sampleCount_ += samples;
    return 0;
}
} // end namespace AudioStandard
} // end OHOS
