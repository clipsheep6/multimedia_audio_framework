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
#include "audio_toneplayer_private.h"


namespace OHOS {
namespace AudioStandard {
constexpr int32_t C20MS = 20;
constexpr int32_t C1000MS = 1000;
constexpr int32_t CMAXWAIT = 3;
constexpr int32_t CNANOSEC = 1000000000;
constexpr int32_t CMICROSEC = 1000000;
constexpr int32_t CDOUBLE = 2;
constexpr float CINTMAX = 32767.0;
constexpr int32_t C32500 = 32500;
constexpr int32_t C16BIT = 16;

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
    mState = TONE_PLAYER_IDLE;
    mRendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    mRendererOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
    mRendererOptions.streamInfo.format = SAMPLE_S16LE;
    mRendererOptions.streamInfo.channels = MONO;
    mRendererOptions.rendererInfo.contentType = rendereInfo.contentType; // contentType::CONTENT_TYPE_MUSIC;
    mRendererOptions.rendererInfo.streamUsage = rendereInfo.streamUsage; // streamUsage::STREAM_USAGE_MEDIA;
    mRendererOptions.rendererInfo.rendererFlags = rendereInfo.rendererFlags;
    supportedTones_ = AudioPolicyManager::GetInstance().GetSupportedTones();
    volume_ = TRACK_VOLUME;
    pToneInfo_ = NULL;
    pNewToneInfo_ = NULL;
    samplingRate_ = mRendererOptions.streamInfo.samplingRate;
    if (!cachePath.empty()) {
        AUDIO_INFO_LOG("copy application cache path");
        cachePath_.assign(cachePath);
    }
    // Generate tone by chunks of 20 ms to keep cadencing precision
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  volume=%{public}f",  volume_);
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  size=%{public}zu",  supportedTones_.size());
}


TonePlayer::~TonePlayer() = default;

TonePlayerPrivate::~TonePlayerPrivate()
{
    AUDIO_INFO_LOG("TonePlayerPrivate destructor");

    if (pAudioRenderer_ != 0) {
        StopTone();
        AUDIO_INFO_LOG("Delete Track: %p", pAudioRenderer_.get());
        pAudioRenderer_->Clear();
    }
    clearWaveGenInfos();
}


bool TonePlayerPrivate::LoadTone(ToneType toneType)
{
    bool lResult = false;

    if (toneType >= NUM_TONES) {
        return lResult;
    }
    if (mState != TONE_PLAYER_IDLE && mState != TONE_PLAYER_INIT) {
        return lResult;
    }
    if (std::find(supportedTones_.begin(), supportedTones_.end(), (int32_t)toneType) == supportedTones_.end()) {
        return lResult;
    }
    mutexLock_.lock();
    if (mState == TONE_PLAYER_IDLE) {
        AUDIO_DEBUG_LOG("startTone: try to re-init AudioTrack");
        if (!InitAudioRenderer()) {
            mutexLock_.unlock();
            return lResult;
        }
    }
    // Get descriptor for requested tone
    pNewToneInfo_ = AudioPolicyManager::GetInstance().GetToneConfig(toneType);
    mutexLock_.unlock();

    return true;
}

bool TonePlayerPrivate::StartTone()
{
    bool lRet = false;
    struct timeval tv = {0};
    status_t lStatus;
    mutexLock_.lock();
    if (mState != TONE_PLAYER_INIT && mState != TONE_PLAYER_IDLE) {
        mutexLock_.unlock(); 
        return lRet;
    }

    if (mState == TONE_PLAYER_IDLE) {
        AUDIO_DEBUG_LOG("startTone: try to re-init AudioTrack");
        if (!InitAudioRenderer()) {
            mutexLock_.unlock();
            return lRet;
        }
    }
    AUDIO_INFO_LOG("STARTTONE mstate %{public}d", mState);
    durationMs_ = -1;
    if (InitToneWaveInfo()) {
        lRet = true;
        mState = TONE_PLAYER_STARTING;
        if (clock_gettime(CLOCK_MONOTONIC, &startTime_) != 0) {
            startTime_.tv_sec = 0;
        }
        AUDIO_INFO_LOG("DEBUG Immediate start, time %{public}d", (unsigned int)tv.tv_sec);
        mutexLock_.unlock(); // Newly added
        pAudioRenderer_->Start();
        mutexLock_.lock(); // Newly added
        if (mState == TONE_PLAYER_STARTING) {
            mutexLock_.unlock(); // Newly added
            std::unique_lock<std::mutex> lock(cbkCondLock_);
            lStatus = waitAudioCbkCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
            AUDIO_INFO_LOG("Immediate start got notified");
            mutexLock_.lock(); // Newly added
            if (lStatus == std::cv_status::timeout) {
                AUDIO_ERR_LOG("--- Immediate start timed out, status %{public}d", lStatus);
                mState = TONE_PLAYER_IDLE;
                lRet = false;
            }
        }
    } else {
        AUDIO_INFO_LOG("Prepare Wave Return ERR");
        mState = TONE_PLAYER_IDLE;
    }
    mutexLock_.unlock();
    gettimeofday(&tv, NULL);

    CHECK_AND_RETURN_RET_LOG(lRet == false, lRet, "Tone started, time %{public}d", (unsigned int)(tv.tv_sec));
    CHECK_AND_RETURN_RET_LOG(!lRet == true, lRet, "Tone start failed!!!, time %{public}d",
        (unsigned int)(tv.tv_sec));

    return lRet;
}

bool TonePlayerPrivate::Release()
{
    if (pAudioRenderer_ != nullptr) {
        StopTone();
        AUDIO_DEBUG_LOG("Delete Track: %p", pAudioRenderer_.get());
        pAudioRenderer_->Clear();
    }
    clearWaveGenInfos();
    pAudioRenderer_ = nullptr;
    return true;
}

bool TonePlayerPrivate::StopTone()
{
    struct timeval tv = {0};
    bool lResult = true;
    AUDIO_DEBUG_LOG("stopTone");

    mutexLock_.lock();
    if (mState == TONE_PLAYER_IDLE  || mState == TONE_PLAYER_INIT) {
        AUDIO_INFO_LOG("-stop tone End");
        mutexLock_.unlock();
        return lResult;
    }
    if (mState == TONE_PLAYER_PLAYING || mState == TONE_PLAYER_STARTING || mState == TONE_PLAYER_RESTARTING) {
        struct timespec stopTime;
        // If the start time is valid, make sure that the number of audio samples produced
        // corresponds at least to the time between the start and stop commands.
        // This is needed in case of cold start of the output stream.
        if ((startTime_.tv_sec != 0) && (clock_gettime(CLOCK_MONOTONIC, &stopTime) == 0)) {
            time_t sec = stopTime.tv_sec - startTime_.tv_sec;
            auto nsec = stopTime.tv_nsec - startTime_.tv_nsec;
            if (nsec < 0) {
                --sec;
                nsec += CNANOSEC;
            }
            if ((sec + 1) > ((time_t)(INT_MAX / samplingRate_))) {
                maxSample_ = sec * samplingRate_;
            } else {
                // samplingRate_ is always > 1000
                sec = sec * C1000MS + nsec / CMICROSEC; // duration in milliseconds
                maxSample_ = (unsigned int)(((int64_t)sec * samplingRate_) / C1000MS);
            }
            AUDIO_INFO_LOG("stopTone() forcing maxSample to %{public}d, total for far %{public}d",
                maxSample_,  totalSample_);
        } else {
            mState = TONE_PLAYER_STOPPING;
        }
    }
    AUDIO_INFO_LOG("WAITING wait_for cond");
    mutexLock_.unlock();
    std::unique_lock<std::mutex> lock(cbkCondLock_);
    status_t lStatus = waitAudioCbkCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
    mutexLock_.lock();
    if (lStatus != std::cv_status::timeout) {
        AUDIO_ERR_LOG("--- StopTone waiting wait_for cond got notified");
        gettimeofday(&tv, NULL);
        AUDIO_INFO_LOG("track stop complete, time %{public}d", (unsigned int)(tv.tv_sec));
    } else {
        AUDIO_ERR_LOG("--- Stop timed out");
        mState = TONE_PLAYER_IDLE;
    }
    // pAudioRenderer_->Stop(); clearwaves not called
    AUDIO_INFO_LOG("-stop tone clear waves");
    clearWaveGenInfos();
    mutexLock_.unlock();
    pAudioRenderer_->Stop(); // Newly added
    return lResult;
}


bool TonePlayerPrivate::InitAudioRenderer()
{
    processSize_ = (mRendererOptions.streamInfo.samplingRate * C20MS) / C1000MS;
    if (!cachePath_.empty()) {
        pAudioRenderer_ = AudioRenderer::Create(cachePath_, mRendererOptions);
    } else {
        pAudioRenderer_ = AudioRenderer::Create(mRendererOptions);
    }
    if (pAudioRenderer_ == nullptr) {
        AUDIO_ERR_LOG("initAudioRenderer: Renderer create failed");
        return false;
    }

    AUDIO_INFO_LOG("initAudioRenderer: Playback renderer created");
    if (pAudioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK)) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRenderMode failed");
        return false;
    }
    AUDIO_INFO_LOG("initAudioRenderer: SetRenderMode Sucessful");
    if (pAudioRenderer_->SetRendererWriteCallback(shared_from_this())) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRendererWriteCallback failed");
        return false;
    }
    AUDIO_INFO_LOG("initAudioRenderer: SetRendererWriteCallback Sucessful");
    if (pAudioRenderer_->SetRendererCallback(shared_from_this())) {
        AUDIO_ERR_LOG("initAudioRenderer: SetRendererCallbackfailed");
        return false;
    }

    AUDIO_INFO_LOG("initAudioRenderer: SetRendererCallback Sucessful");
    pAudioRenderer_->SetVolume(volume_);
    AUDIO_INFO_LOG("initAudioRenderer: SetVolume Sucessful");

    mState = TONE_PLAYER_INIT;
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

bool TonePlayerPrivate::CheckToneStopped(uint32_t lReqSmp, int16_t * lpAudioBuf)
{
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;
    AUDIO_INFO_LOG("CheckToneStopped state : %{public}d tot : %{public}d max : %{public}d not calling", 
        mState, totalSample_, maxSample_);
    if(mState == TONE_PLAYER_STOPPED) {
        return true;
    }
    if (lpToneDesc->segments[currSegment_].duration == 0 ||
        totalSample_ > maxSample_ || mState == TONE_PLAYER_STOPPING) {
        if (mState == TONE_PLAYER_PLAYING) {
            mState = TONE_PLAYER_STOPPING;
            AUDIO_INFO_LOG("Audicallback move playing to stoping");
        }
        if (lpToneDesc->segments[currSegment_].duration != 0) {
            uint16_t lIndex = 0;
            uint16_t lFrequency = lpToneDesc->segments[currSegment_].waveFreq[lIndex];

            while (lFrequency != 0) {
                GenerateEndWave(lFrequency, lpAudioBuf, lReqSmp);
                lFrequency = lpToneDesc->segments[currSegment_].waveFreq[++lIndex];
            }
        }
        // fade out before stopping if maximum duration reached
        AUDIO_INFO_LOG("Audicallback fade out before stopping lwave cmd moved to STOP");
        return true;
    }
    return false;
}

bool TonePlayerPrivate::CheckToneStarted(uint32_t lReqSmp, int16_t * lpAudioBuf)
{
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;
    if (mState == TONE_PLAYER_STARTING) {
            mState = TONE_PLAYER_PLAYING;
            AUDIO_INFO_LOG("Audicallback move to playing");
        
        if (lpToneDesc->segments[currSegment_].duration != 0) {
            uint16_t lIndex = 0;
            uint16_t lFrequency = lpToneDesc->segments[currSegment_].waveFreq[lIndex];

            while (lFrequency != 0) {
                GenerateStartWave(lFrequency, lpAudioBuf, lReqSmp);
                lFrequency = lpToneDesc->segments[currSegment_].waveFreq[++lIndex];
            }
        }
        // fade out before stopping if maximum duration reached
        AUDIO_INFO_LOG("CheckToneStarted GenerateStartWave to currseg: %{public}d ", currSegment_);
        return true;
    }
    return false;
}

bool TonePlayerPrivate::CheckToneContinuity ()
{
    bool lResult = false;
    struct timeval tv = {0};
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;
    if (lpToneDesc->segments[currSegment_].loopCnt) {
        if (loopCounter_ < lpToneDesc->segments[currSegment_].loopCnt) {
            AUDIO_INFO_LOG ("in if loop loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
                lpToneDesc->segments[currSegment_].loopCnt,
                loopCounter_,
                currSegment_);
            currSegment_ = lpToneDesc->segments[currSegment_].loopIndx;
            ++loopCounter_;
        } else {
         // completed loop. go to next segment
            loopCounter_ = 0;
            currSegment_++;
            AUDIO_INFO_LOG ("in else loop loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
                lpToneDesc->segments[currSegment_].loopCnt,
                loopCounter_,
                currSegment_);
        }
    } else {
        currSegment_++;
        AUDIO_INFO_LOG ("Goto next seg loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
            lpToneDesc->segments[currSegment_].loopCnt,
            loopCounter_,
            currSegment_);
    }

    // Handle loop if last segment reached
    if (lpToneDesc->segments[currSegment_].duration == 0) {
        AUDIO_INFO_LOG("Last Seg: %{public}d", currSegment_);

        // Pre increment loop count and restart if total count not reached. Stop sequence otherwise
        if (++currCount_ <= lpToneDesc->repeatCnt) {
            AUDIO_INFO_LOG("Repeating Count: %{public}d", currCount_);

            currSegment_ = lpToneDesc->repeatSegment;
            lResult = true;
            AUDIO_INFO_LOG("New segment %{public}d, Next Time: %lld", currSegment_,
                ((long long)(nextSegSample_)*C1000MS)/samplingRate_);
        } else {
            gettimeofday(&tv, NULL);
            AUDIO_INFO_LOG("End repeat, time: %{public}d", (unsigned int)(tv.tv_sec));
            lResult = false;
        }
    } else {
        AUDIO_INFO_LOG("New segment %{public}d, Next Time: %lld", currSegment_,
            ((long long)(nextSegSample_)*C1000MS)/samplingRate_);
        lResult = true;
    }
    return lResult;

}

bool TonePlayerPrivate::ContinueToneplay(uint32_t lReqSmp, int16_t * lpAudioBuf)
{
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;
    if (mState != TONE_PLAYER_PLAYING) {
        return false;
    }

    if (totalSample_ <= nextSegSample_) {
         AUDIO_INFO_LOG("Audicallback Continue Playing Tone");
        if (lpToneDesc->segments[currSegment_].duration != 0) {
            uint16_t lIndex = 0;
            uint16_t lFrequency = lpToneDesc->segments[currSegment_].waveFreq[lIndex];

            while (lFrequency != 0) {
                GenerateWave(lFrequency, lpAudioBuf, lReqSmp);
                lFrequency = lpToneDesc->segments[currSegment_].waveFreq[++lIndex];
            }
        }
    } else {
        AUDIO_INFO_LOG("Fade out the tone");
        if (lpToneDesc->segments[currSegment_].duration != 0) {
            uint16_t lIndex = 0;
            uint16_t lFrequency = lpToneDesc->segments[currSegment_].waveFreq[lIndex];

            while (lFrequency != 0) {
                GenerateEndWave(lFrequency, lpAudioBuf, lReqSmp);
                lFrequency = lpToneDesc->segments[currSegment_].waveFreq[++lIndex];
            }
        }
        if (CheckToneContinuity())
        {
            if (lpToneDesc->segments[currSegment_].duration != 0) {
                uint16_t lIndex = 0;
                uint16_t lFrequency = lpToneDesc->segments[currSegment_].waveFreq[lIndex];

                while (lFrequency != 0) {
                    GenerateStartWave(lFrequency, lpAudioBuf, lReqSmp);
                    lFrequency = lpToneDesc->segments[currSegment_].waveFreq[++lIndex];
                }
            }
        }
        nextSegSample_
            += (lpToneDesc->segments[currSegment_].duration * samplingRate_) / C1000MS;

    }
    return true;
}

void TonePlayerPrivate::AudioToneRendererCallback()
{

    BufferDesc bufDesc_ = {};
    pAudioRenderer_->GetBufferDesc(bufDesc_);
    int16_t *lpAudioBuf = (int16_t *)bufDesc_.buffer;
    size_t lbufLength = 0;
    pAudioRenderer_->GetBufferSize(lbufLength);
    uint32_t totalBufAvailable = lbufLength /sizeof(int16_t);
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;

    AUDIO_INFO_LOG("Audio Rendere Callback, buflen: %{public}zu  totalBufAvailable %{public}d  lbufLength %{public}zu",
        bufDesc_.bufLength, totalBufAvailable, lbufLength);
    AUDIO_INFO_LOG("Audio Callback, mstate : %{public}d", mState);
    if (lbufLength  == 0) return;
    // Clear output buffer: WaveGenerator accumulates into lpAudioBuf buffer
    memset_s(lpAudioBuf, bufDesc_.bufLength, 0, bufDesc_.bufLength);

    while (totalBufAvailable) {
        uint32_t lSamples = totalBufAvailable < processSize_* CDOUBLE ? totalBufAvailable : processSize_;
        bool lSignal = false;
        AUDIO_INFO_LOG("Audio Callback, lReqSmp : %{public}d mState : %{public}d", lSamples, mState);
        mutexLock_.lock();
        // Update pcm frame count and end time (current time at the end of this process)
        totalSample_ += lSamples;
        AUDIO_INFO_LOG("Audio Callback, totalSample : %{public}d", totalSample_);
        AUDIO_INFO_LOG("Audio Callback, maxSample : %{public}d", maxSample_);
        if (CheckToneStopped(lSamples, lpAudioBuf)) {
            if(mState == TONE_PLAYER_STOPPING) {
                AUDIO_INFO_LOG("Audicallback move stopping to STOPPED");
                mState = TONE_PLAYER_STOPPED;
                nextSegSample_ = TONEINFO_INF;
                bufDesc_.dataLength += lSamples*sizeof(int16_t);
                mutexLock_.unlock();
                pAudioRenderer_->Enqueue(bufDesc_);
            } else if (mState == TONE_PLAYER_STOPPED) {
                mState = TONE_PLAYER_INIT;
                totalBufAvailable = 0;
                bufDesc_.dataLength = 0;
                AUDIO_INFO_LOG("Notifing all the STOP");
                mutexLock_.unlock();
                waitAudioCbkCond_.notify_all();
                AUDIO_INFO_LOG("Cbk TONE_STOPPED");
            }
            return;
        } else if(CheckToneStarted(lSamples, lpAudioBuf)) {
            AUDIO_INFO_LOG("CheckToneStarted ToneStart");
            bufDesc_.dataLength += lSamples*sizeof(int16_t);
            lSignal = true;
        } else {
            if(ContinueToneplay(lSamples, lpAudioBuf)) {
                AUDIO_INFO_LOG("ContinueToneplay tone playing");
                bufDesc_.dataLength += lSamples*sizeof(int16_t);
            }

        }
        totalBufAvailable -= lSamples;
        lpAudioBuf += lSamples;

        AUDIO_INFO_LOG("mutexLock_.unlock");
        mutexLock_.unlock();
        if (lSignal) {
            AUDIO_INFO_LOG("Notifing all the");
            waitAudioCbkCond_.notify_all();
        }

    }
    AUDIO_INFO_LOG("Exiting the AudioToneRendererCallback %{public}zu, mState %{public}d ", bufDesc_.dataLength, mState);
    if (bufDesc_.dataLength) {
        pAudioRenderer_->Enqueue(bufDesc_);
    }
}

bool TonePlayerPrivate::InitToneWaveInfo()
{
    unsigned int lSegmentIdx = 0;
    AUDIO_INFO_LOG("InitToneWaveInfo, Entered");
    if (pNewToneInfo_ == NULL) {
        return false;
    }

    // Remove existing wave Informations if any
    clearWaveGenInfos();

    pToneInfo_ = pNewToneInfo_;
    maxSample_ = TONEINFO_INF;
    while (pToneInfo_->segments[lSegmentIdx].duration) {
        uint16_t lFreqIdx = 0;
        // Get total number of sine waves: needed to adapt sine wave gain.
        uint16_t numOfwaves = NumOfWaves(lSegmentIdx);
        uint16_t lFrequency = pToneInfo_->segments[lSegmentIdx].waveFreq[lFreqIdx];
        AUDIO_INFO_LOG("InitToneWaveInfo, lNumWaves %{public}d", numOfwaves);
        while (lFrequency && numOfwaves != 0) {
            AUDIO_INFO_LOG("frequency, Frequency %{public}d", lFrequency);
            GenerateWaveInfos(lFrequency,numOfwaves);
            lFrequency = pNewToneInfo_->segments[lSegmentIdx].waveFreq[++lFreqIdx];
        }
        lSegmentIdx++;
    }

    // Initialize tone sequencer
    totalSample_ = 0;
    currSegment_ = 0;
    currCount_ = 0;
    loopCounter_ = 0;
    if (pToneInfo_->segments[0].duration == TONEINFO_INF) {
        nextSegSample_ = TONEINFO_INF;
    } else {
        nextSegSample_ = (pToneInfo_->segments[0].duration * samplingRate_) / C1000MS;
    }
    AUDIO_INFO_LOG("Prepare wave, nextSegSample_ %{public}d", nextSegSample_);
    return true;
}


uint16_t TonePlayerPrivate::NumOfWaves(uint16_t segmentIdx)
{
    uint16_t lCount = 0;

    if (pToneInfo_->segments[segmentIdx].duration) {
        while (pToneInfo_->segments[segmentIdx].waveFreq[lCount]) {
            lCount++;
        }
        lCount++;
    }

    return lCount;
}

void TonePlayerPrivate::clearWaveGenInfos()
{
    AUDIO_INFO_LOG("clearWaveGenInfos:");
    std::map <uint16_t, WaveInfo *>::iterator lItr;
    for (lItr = waveGenInfos_.begin(); lItr !=  waveGenInfos_.end(); lItr++) {
        delete lItr->second;
    }
    waveGenInfos_.clear();
}


bool TonePlayerPrivate::GenerateWaveInfos(uint16_t frequency, uint16_t lNumOfWaves) {
    double d0;
    double F_div_Fs;  // frequency / samplingRate
    float volume = TONEINFO_GAIN / lNumOfWaves;
    TonePlayerPrivate::WaveInfo *lpWaveInfo = NULL;
 // Instantiate a wave generator if  not already done for this frequency
    if (waveGenInfos_.find(frequency) == waveGenInfos_.end()) {
        lpWaveInfo = new TonePlayerPrivate::WaveInfo();
        waveGenInfos_[frequency] = lpWaveInfo;
    } else {
        return true;
    }
    if (waveGenInfos_[frequency] == NULL) {
        return false;
    }
    F_div_Fs = frequency / (double)samplingRate_;
    d0 = - (float)GEN_AMP * sin(CDOUBLE * M_PI * F_div_Fs);
    lpWaveInfo->mS2_0 = (int16_t)d0;
    lpWaveInfo->mS1 = 0;
    lpWaveInfo->mS2 = lpWaveInfo->mS2_0;

    lpWaveInfo->mAmplitude_Q15 = (int16_t)(CINTMAX * CINTMAX * volume / GEN_AMP);
    // take some margin for amplitude fluctuation
    if (lpWaveInfo->mAmplitude_Q15 > C32500)
        lpWaveInfo->mAmplitude_Q15 = C32500;

    d0 = (CINTMAX+1) * cos(CDOUBLE * M_PI * F_div_Fs); // Q14*2*cos()
    if (d0 > CINTMAX)
        d0 = CINTMAX;
    lpWaveInfo->mA1_Q14 = (int16_t) d0;
    return true;
}

bool TonePlayerPrivate::GenerateStartWave(int16_t frequency, int16_t *outBuffer, uint16_t sampleCnt)
{
    long lS1, lS2;
    long lA1, lAmplitude;
    long lCurWave;  // current sample
    TonePlayerPrivate::WaveInfo *lpWaveInfo = NULL;
    if (waveGenInfos_.find(frequency) == waveGenInfos_.end()) {
        return false;
    }
    lpWaveInfo = waveGenInfos_[frequency];
    lS1 = (long)0;
    lS2 = (long)lpWaveInfo->mS2_0;
    lA1 = (long)lpWaveInfo->mA1_Q14;
    lAmplitude = (long)lpWaveInfo->mAmplitude_Q15;
    while (sampleCnt) {
        sampleCnt--;
        lCurWave = ((lA1 * lS1) >> SHIFT_Q14) - lS2;
        // shift delay
        lS2 = lS1;
        lS1 = lCurWave;
        lCurWave = (lAmplitude * lCurWave) >> SHIFT_Q15;
        *(outBuffer++) += (int16_t)lCurWave;  // put result in buffer
    }
    // save status
    lpWaveInfo->mS1 = lS1;
    lpWaveInfo->mS2 = lS2;
    return true;
}

bool TonePlayerPrivate::GenerateWave(int16_t frequency, int16_t *outBuffer, uint16_t sampleCnt)
{
    long lS1, lS2;
    long lA1, lAmplitude;
    long lCurWave;  // current sample
    TonePlayerPrivate::WaveInfo *lpWaveInfo = NULL;
    if (waveGenInfos_.find(frequency) == waveGenInfos_.end()) {
        return false;
    }
    lpWaveInfo = waveGenInfos_[frequency];
    lS1 = (long)lpWaveInfo->mS1;
    lS2 = (long)lpWaveInfo->mS2;
    lA1 = (long)lpWaveInfo->mA1_Q14;
    lAmplitude = (long)lpWaveInfo->mAmplitude_Q15;
    while (sampleCnt) {
        sampleCnt--;
        lCurWave = ((lA1 * lS1) >> SHIFT_Q14) - lS2;
        // shift delay
        lS2 = lS1;
        lS1 = lCurWave;
        lCurWave = (lAmplitude * lCurWave) >> SHIFT_Q15;
        *(outBuffer++) += (int16_t)lCurWave;  // put result in buffer
    }
    // save status
    lpWaveInfo->mS1 = lS1;
    lpWaveInfo->mS2 = lS2;
    return true;
}

bool TonePlayerPrivate::GenerateEndWave(int16_t frequency, int16_t *outBuffer, uint16_t sampleCnt)
{
    long lS1, lS2;
    long lA1, lAmplitude;
    long lCurWave;  // current sample
    TonePlayerPrivate::WaveInfo *lpWaveInfo;
    if (waveGenInfos_.find(frequency) == waveGenInfos_.end()) {
        return false;
    }
    lpWaveInfo = waveGenInfos_[frequency];
    lS1 = (long)lpWaveInfo->mS1;
    lS2 = (long)lpWaveInfo->mS2;
    lA1 = (long)lpWaveInfo->mA1_Q14;
    lAmplitude = (long)lpWaveInfo->mAmplitude_Q15;
    lAmplitude <<= C16BIT;
    if (sampleCnt == 0) {
            return false;
    }
    long dec = lAmplitude/sampleCnt;
    while (sampleCnt) {
        sampleCnt--;
        lCurWave = ((lA1 * lS1) >> SHIFT_Q14) - lS2;
        // shift delay
        lS2 = lS1;
        lS1 = lCurWave;
        lCurWave = ((lAmplitude>>C16BIT) * lCurWave) >> SHIFT_Q15;
        *(outBuffer++) += (int16_t)lCurWave;  // put result in buffer
        lAmplitude -= dec;
    }
    // save status
    lpWaveInfo->mS1 = lS1;
    lpWaveInfo->mS2 = lS2;
    return true;
}

#if 0
TonePlayerPrivate::WaveGenerator::WaveGenerator(uint32_t samplingRate,
    uint16_t frequency, float volume)
{
#ifndef PROPRIETARY_WAVEGEN
    double d0;
    double F_div_Fs;  // frequency / samplingRate

    F_div_Fs = frequency / (double)samplingRate;
    d0 = - (float)GEN_AMP * sin(CDOUBLE * M_PI * F_div_Fs);
    mS2_0 = (int16_t)d0;
    mS1 = 0;
    mS2 = mS2_0;

    mAmplitude_Q15 = (int16_t)(CINTMAX * CINTMAX * volume / GEN_AMP);
    // take some margin for amplitude fluctuation
    if (mAmplitude_Q15 > C32500)
        mAmplitude_Q15 = C32500;

    d0 = (CINTMAX+1) * cos(CDOUBLE * M_PI * F_div_Fs); // Q14*2*cos()
    if (d0 > CINTMAX)
        d0 = CINTMAX;
    mA1_Q14 = (int16_t) d0;

    AUDIO_INFO_LOG("WaveGenerator init, mA1_Q14: %{public}d, mS2_0: %{public}d, mAmplitude_Q15: %{public}d",
        mA1_Q14, mS2_0, mAmplitude_Q15);
#else
    double F_div_Fs = frequency / (double)samplingRate; // frequency / samplingRate
    mAmplitude_Q15  = C32500;
    mS1 = CDOUBLE * M_PI * F_div_Fs;

    AUDIO_INFO_LOG("WaveGenerator init, F_div_Fs: %{public}f, mS1 : %{public}f mAmplitude_Q15: %{public}d",
        F_div_Fs, mS1, mAmplitude_Q15);
#endif
}


TonePlayerPrivate::WaveGenerator::~WaveGenerator()
{
}

void TonePlayerPrivate::WaveGenerator::getSamples(int16_t *outBuffer,
    unsigned int count, unsigned int command)
{
#ifndef PROPRIETARY_WAVEGEN
    long lS1, lS2;
    long lA1, lAmplitude;
    long lCurWave;  // current sample

    // init local
    if (command == WAVEGEN_START) {
        lS1 = (long)0;
        lS2 = (long)mS2_0;
    } else {
        lS1 = mS1;
        lS2 = mS2;
    }
    lA1 = (long)mA1_Q14;
    lAmplitude = (long)mAmplitude_Q15;

    if (command == WAVEGEN_STOP) {
        lAmplitude <<= C16BIT;
        if (count == 0) {
            return;
        }
        long dec = lAmplitude/count;
        // loop generation
        while (count) {
            count--;
            lCurWave = ((lA1 * lS1) >> S_Q14) - lS2;
            // shift delay
            lS2 = lS1;
            lS1 = lCurWave;
            lCurWave = ((lAmplitude>>C16BIT) * lCurWave) >> S_Q15;
            *(outBuffer++) += (int16_t)lCurWave;  // put result in buffer
            lAmplitude -= dec;
        }
    } else {
        // loop generation
        while (count) {
            count--;
            Sample = ((lA1 * lS1) >> S_Q14) - lS2;
            // shift delay
            lS2 = lS1;
            lS1 = lCurWave;
            lCurWave = (lAmplitude * lCurWave) >> S_Q15;
            *(outBuffer++) += (int16_t)lCurWave;  // put result in buffer
        }
    }

    // save status
    mS1 = lS1;
    mS2 = lS2;
#else
    double Sample;  // current sample
    double lAmplitude;

// init local
    if (command == WAVEGEN_START) {
        mS2_0 = (long)0;  // Time count
    }
    // comment mS1 is two times x M_PI  x F_div_Fs;
    mA1_Q14 = 0;
    mS2 = mS2_0;

    if (command == WAVEGEN_STOP) {
        lAmplitude = mAmplitude_Q15;
        if (count == 0) {
            return;
        }
        long dec = lAmplitude/count;
        // loop generation
        while (count) {
            count--;
            Sample = sin(mS1 * mS2_0);
            mS2_0 += 1;
            Sample = (lAmplitude * Sample);
            *(outBuffer++) += (int16_t)Sample;  // put result in buffer
            lAmplitude -= dec;
        }
    } else {
        // loop generation
        while (count) {
            count--;
            Sample = sin(mS1 * mS2_0);
            mS2_0 += 1;
            Sample = mAmplitude_Q15 * Sample;
            *(outBuffer++) += (int16_t)Sample;
        }
    }
#endif
}
#endif
} // end namespace AudioStandard
} // end OHOS
