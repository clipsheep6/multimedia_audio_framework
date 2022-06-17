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
    constexpr float TRACK_VOLUME = 0.5f;
    return std::make_shared <TonePlayerPrivate>("", rendererInfo, TRACK_VOLUME);
}

std::shared_ptr<TonePlayer> TonePlayer::Create(const std::string cachePath, const AudioRendererInfo &rendererInfo)
{
    constexpr float TRACK_VOLUME = 0.5f;
    return std::make_shared <TonePlayerPrivate>(cachePath, rendererInfo, TRACK_VOLUME);
}

TonePlayerPrivate::TonePlayerPrivate(const std::string cachePath, const AudioRendererInfo &rendereInfo, float volume)
{
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  volume=%{public}f",  volume);

    mState = TONE_PLAYER_IDLE;
    mRendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    mRendererOptions.streamInfo.samplingRate = SAMPLE_RATE_8000;
    mRendererOptions.streamInfo.format = SAMPLE_S16LE;
    mRendererOptions.streamInfo.channels = MONO;
    mRendererOptions.rendererInfo.contentType = rendereInfo.contentType; // contentType::CONTENT_TYPE_MUSIC;
    mRendererOptions.rendererInfo.streamUsage = rendereInfo.streamUsage; // streamUsage::STREAM_USAGE_MEDIA;
    mRendererOptions.rendererInfo.rendererFlags = rendereInfo.rendererFlags;
    supportedTones_ = AudioPolicyManager::GetInstance().GetSupportedTones();
    volume_ = volume;
    pToneInfo_ = NULL;
    pNewToneInfo_ = NULL;
    samplingRate_ = mRendererOptions.streamInfo.samplingRate;
    if (!cachePath.empty()) {
        AUDIO_INFO_LOG("copy application cache path");
        cachePath_.assign(cachePath);
    }
    // Generate tone by chunks of 20 ms to keep cadencing precision
    AUDIO_INFO_LOG("TonePlayerPrivate constructor:  size=%{public}d",  supportedTones_.size());

    processSize_ = (samplingRate_ * C20MS) / C1000MS;
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
    clearWaveGens();
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
        if (!initAudioRenderer()) {
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
    bool lResult = false;
    struct timeval tv = {0};
    status_t lStatus;
    mutexLock_.lock();
    if (mState != TONE_PLAYER_IDLE && mState != TONE_PLAYER_INIT) {
        return lResult;
    }
    if (mState == TONE_PLAYER_IDLE) {
        AUDIO_DEBUG_LOG("startTone: try to re-init AudioTrack");
        if (!initAudioRenderer()) {
            mutexLock_.unlock();
            return lResult;
        }
    }
    AUDIO_INFO_LOG("STARTTONE mstate %{public}d", mState);
    durationMs_ = -1;
    if (prepareWave()) {
        lResult = true;
        mState = TONE_PLAYER_STARTING;
        if (clock_gettime(CLOCK_MONOTONIC, &startTime_) != 0) {
            startTime_.tv_sec = 0;
        }
        AUDIO_INFO_LOG("DEBUG Immediate start, time %{public}d", (unsigned int)tv.tv_sec);
        mutexLock_.unlock(); // Newly added
        pAudioRenderer_->Start();
        mutexLock_.lock(); // Newly added
        if (mState == TONE_PLAYER_STARTING) {
            AUDIO_INFO_LOG("Wait for START CALLBACK");
            mutexLock_.unlock(); // Newly added
            std::unique_lock<std::mutex> lock(cbkCondLock_);
            lStatus = waitAudioCbkCond_.wait_for(lock, std::chrono::seconds(CMAXWAIT));
            AUDIO_INFO_LOG("Immediate start got notified");
            mutexLock_.lock(); // Newly added
            if (lStatus == std::cv_status::timeout) {
                AUDIO_ERR_LOG("--- Immediate start timed out, status %{public}d", lStatus);
                mState = TONE_PLAYER_IDLE;
                lResult = false;
            }
        }
        AUDIO_INFO_LOG("mstate is not TONE_STARTING");
    } else {
        AUDIO_INFO_LOG("Prepare Wave Return ERR");
        mState = TONE_PLAYER_IDLE;
    }
    AUDIO_INFO_LOG("mutexLock_.unlock()");
    mutexLock_.unlock();
    gettimeofday(&tv, NULL);

    CHECK_AND_RETURN_RET_LOG(lResult == false, lResult, "Tone started, time %{public}d", (unsigned int)(tv.tv_sec));
    CHECK_AND_RETURN_RET_LOG(!lResult == true, lResult, "Tone start failed!!!, time %{public}d",
        (unsigned int)(tv.tv_sec));

    return lResult;
}

bool TonePlayerPrivate::Release()
{
    if (pAudioRenderer_ != nullptr) {
        StopTone();
        AUDIO_DEBUG_LOG("Delete Track: %p", pAudioRenderer_.get());
        pAudioRenderer_->Clear();
    }
    clearWaveGens();
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
                maxSample = sec * samplingRate_;
            } else {
                // samplingRate_ is always > 1000
                sec = sec * C1000MS + nsec / CMICROSEC; // duration in milliseconds
                maxSample = (unsigned int)(((int64_t)sec * samplingRate_) / C1000MS);
            }
            AUDIO_INFO_LOG("stopTone() forcing maxSample to %{public}d, total for far %{public}d",
                maxSample,  totalSample);
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
    clearWaveGens();
    mutexLock_.unlock();
    pAudioRenderer_->Stop(); // Newly added
    return lResult;
}


bool TonePlayerPrivate::initAudioRenderer()
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
    AUDIO_INFO_LOG("OnWriteData Cbk : %{public}d", length);
    audioRendererCallback();
}

void TonePlayerPrivate::OnStateChange(const RendererState state)
{
    AUDIO_INFO_LOG("OnStateChange Cbk : %{public}d not calling", state);
    if (state == RENDERER_RUNNING || state == RENDERER_STOPPED) {
    }
}

void TonePlayerPrivate::audioRendererCallback()
{
    struct timeval tv = {0};
    BufferDesc bufDesc_ = {};
    pAudioRenderer_->GetBufferDesc(bufDesc_);
    int16_t *lpOut = (int16_t *)bufDesc_.buffer;
    size_t lbufLength = 0;
    pAudioRenderer_->GetBufferSize(lbufLength);
    unsigned int totalBufAvailable = lbufLength /sizeof(int16_t);
    std::shared_ptr<ToneInfo> lpToneDesc = pToneInfo_;

    AUDIO_INFO_LOG("Audio Rendere Callback, buflen: %{public}d  totalBufAvailable %{public}d  lbufLength %{public}d",
        bufDesc_.bufLength, totalBufAvailable, lbufLength);
    AUDIO_INFO_LOG("Audio Callback, mstate : %{public}d", mState);
    if (lbufLength  == 0) return;
    // Clear output buffer: WaveGenerator accumulates into lpOut buffer
    memset_s(lpOut, bufDesc_.bufLength, 0, bufDesc_.bufLength);

    while (totalBufAvailable) {
        uint32_t lReqSmp = totalBufAvailable < processSize_* CDOUBLE ? totalBufAvailable : processSize_;
        uint32_t lGenSmp;
        uint32_t lWaveCmd = WaveGenerator::WAVEGEN_CONT;
        bool lSignal = false;
        AUDIO_INFO_LOG("Audio Callback, lReqSmp : %{public}d", lReqSmp);
        mutexLock_.lock();
        // Update pcm frame count and end time (current time at the end of this process)
        totalSample += lReqSmp;
        AUDIO_INFO_LOG("Audio Callback, totalSample : %{public}d", totalSample);
        // Update tone gen state machine and select wave gen command
        switch (mState) {
            case TONE_PLAYER_PLAYING:
                lWaveCmd = WaveGenerator::WAVEGEN_CONT;
                break;
            case TONE_PLAYER_STARTING:
                AUDIO_INFO_LOG("Starting Cbk");

                lWaveCmd = WaveGenerator::WAVEGEN_START;
                break;
            case TONE_PLAYER_STOPPING:
            case TONE_PLAYER_RESTARTING:
                AUDIO_INFO_LOG("Stop/restart Cbk");

                lWaveCmd = WaveGenerator::WAVEGEN_STOP;
                nextSegSample = TONEINFO_INF; // forced to skip state machine management below
                break;
            case TONE_PLAYER_STOPPED:
                AUDIO_INFO_LOG("Stopped Cbk");
                goto audioCallback_EndLoop;
            default:
                AUDIO_INFO_LOG("Extra Cbk");
                goto audioCallback_EndLoop;
            }

        // Exit if tone sequence is over
        AUDIO_INFO_LOG("Audicallback entry currSegment %{public}d totalSample %{public}d maxSample %{public}d",
            currSegment, totalSample, maxSample);
        if (lpToneDesc->segments[currSegment].duration == 0 ||
            totalSample > maxSample) {
            if (mState == TONE_PLAYER_PLAYING) {
                mState = TONE_PLAYER_STOPPING;
                AUDIO_INFO_LOG("Audicallback move playing to stoping");
            }
            if (lpToneDesc->segments[currSegment].duration == 0) {
                AUDIO_INFO_LOG("Audicallback go to endloop as duration is 0 , in playing state");
                goto audioCallback_EndLoop;
            }
            // fade out before stopping if maximum duration reached
            lWaveCmd = WaveGenerator::WAVEGEN_STOP;
            nextSegSample = TONEINFO_INF; // forced to skip state machine management below
            AUDIO_INFO_LOG("Audicallback fade out before stopping lwave cmd moved to STOP");
        }

        if (totalSample > nextSegSample) {
            // Time to go to next sequence segment

            gettimeofday(&tv, NULL);
            AUDIO_INFO_LOG("End Segment, time: %{public}d", (unsigned int)tv.tv_sec);

            lGenSmp = lReqSmp;

            // If segment,  ON -> OFF transition : ramp volume down
            if (lpToneDesc->segments[currSegment].waveFreq[0] != 0) {
                lWaveCmd = WaveGenerator::WAVEGEN_STOP;
                unsigned int lFreqIdx = 0;
                uint16_t lFrequency = lpToneDesc->segments[currSegment].waveFreq[lFreqIdx];

                while (lFrequency != 0) {
                    // Kathir , need to check whethr content is present or not
                    WaveGenerator *lpWaveGen = mWaveGens[lFrequency];
                    lpWaveGen->getSamples(lpOut, lGenSmp, lWaveCmd);
                    lFrequency = lpToneDesc->segments[currSegment].waveFreq[++lFreqIdx];
                }
                bufDesc_.dataLength += lGenSmp*sizeof(int16_t);
                AUDIO_INFO_LOG("ON->OFF, lGenSmp: %{public}d, lReqSmp: %{public}d", lGenSmp, lReqSmp);
            }

            // check if we need to loop and loop for the reqd times
            if (lpToneDesc->segments[currSegment].loopCnt) {
                if (loopCounter_ < lpToneDesc->segments[currSegment].loopCnt) {
                    AUDIO_INFO_LOG ("in if loop loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
                        lpToneDesc->segments[currSegment].loopCnt,
                        loopCounter_,
                        currSegment);
                    currSegment = lpToneDesc->segments[currSegment].loopIndx;
                    ++loopCounter_;
                } else {
                    // completed loop. go to next segment
                    loopCounter_ = 0;
                    currSegment++;
                    AUDIO_INFO_LOG ("in else loop loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
                        lpToneDesc->segments[currSegment].loopCnt,
                        loopCounter_,
                        currSegment);
                }
            } else {
                currSegment++;
                AUDIO_INFO_LOG ("Goto next seg loopCnt(%{public}d) loopctr(%{public}d), CurSeg(%{public}d)",
                    lpToneDesc->segments[currSegment].loopCnt,
                    loopCounter_,
                    currSegment);
            }

            // Handle loop if last segment reached
            if (lpToneDesc->segments[currSegment].duration == 0) {
                AUDIO_INFO_LOG("Last Seg: %{public}d", currSegment);

                // Pre increment loop count and restart if total count not reached. Stop sequence otherwise
                if (++currCount <= lpToneDesc->repeatCnt) {
                    AUDIO_INFO_LOG("Repeating Count: %{public}d", currCount);

                    currSegment = lpToneDesc->repeatSegment;
                    if (lpToneDesc->segments[lpToneDesc->repeatSegment].waveFreq[0] != 0) {
                        lWaveCmd = WaveGenerator::WAVEGEN_START;
                    }

                    AUDIO_INFO_LOG("New segment %{public}d, Next Time: %lld", currSegment,
                        ((long long)(nextSegSample)*C1000MS)/samplingRate_);
                } else {
                    lGenSmp = 0;
                    gettimeofday(&tv, NULL);
                    AUDIO_INFO_LOG("End repeat, time: %{public}d", (unsigned int)(tv.tv_sec));
                }
            } else {
                AUDIO_INFO_LOG("New segment %{public}d, Next Time: %lld", currSegment,
                    ((long long)(nextSegSample)*C1000MS)/samplingRate_);

                if (lpToneDesc->segments[currSegment].waveFreq[0] != 0) {
                    // If next segment is not silent,  OFF -> ON transition : reset wave generator
                    lWaveCmd = WaveGenerator::WAVEGEN_START;
                    AUDIO_INFO_LOG("OFF->ON, lGenSmp: %{public}d, lReqSmp: %{public}d", lGenSmp, lReqSmp);
                } else {
                    lGenSmp = 0;
                }
            }

            // Update next segment transition position. No harm to do it also for last segment
            // as nextSegSample won't be used any more
            nextSegSample
                    += (lpToneDesc->segments[currSegment].duration * samplingRate_) / C1000MS;

            AUDIO_INFO_LOG("audioRendererCallback, nextSegSample %{public}d", nextSegSample);
        } else {
            // Inside a segment keep tone ON or OFF
            if (lpToneDesc->segments[currSegment].waveFreq[0] == 0) {
                lGenSmp = 0;  // If odd segment, tone is currently OFF
                AUDIO_INFO_LOG("OIf odd segment, tone is currently OFF");
            } else {
                lGenSmp = lReqSmp;  // If event segment, tone is currently ON
            }
        }
        AUDIO_INFO_LOG("audioRendererCallback, lGenSmp %{public}d", lGenSmp);
        if (lGenSmp) {
            // If samples must be generated, call all active wave generators and acumulate
            // waves in lpOut
            unsigned int lFreqIdx = 0;
            uint16_t lFrequency = lpToneDesc->segments[currSegment].waveFreq[lFreqIdx];

            while (lFrequency != 0) {
                WaveGenerator *lpWaveGen = mWaveGens[lFrequency];
                lpWaveGen->getSamples(lpOut, lGenSmp, lWaveCmd);
                lFrequency = lpToneDesc->segments[currSegment].waveFreq[++lFreqIdx];
            }
            bufDesc_.dataLength += lGenSmp*sizeof(int16_t);
        }

        totalBufAvailable -= lReqSmp;
        lpOut += lReqSmp;

audioCallback_EndLoop:

        switch (mState) {
            case TONE_PLAYER_STOPPING:
                AUDIO_INFO_LOG("Cbk Stopping");
                mState = TONE_PLAYER_STOPPED;
                // Force loop exit
                totalBufAvailable = 0;
                break;
            case TONE_PLAYER_STOPPED:
                mState = TONE_PLAYER_INIT;
                AUDIO_INFO_LOG("Cbk Stopped track");
                // stoped pAudioRenderer Stop;
                // Force loop exit
                totalBufAvailable = 0;
                bufDesc_.dataLength = 0;
                lSignal = true;
                AUDIO_INFO_LOG("Cbk TONE_STOPPED");
                break;
            case TONE_PLAYER_STARTING:
                AUDIO_INFO_LOG("Cbk starting track");
                mState = TONE_PLAYER_PLAYING;
                lSignal = true;
                break;
            case TONE_PLAYER_PLAYING:
                break;
            default:
                // Force loop exit
                totalBufAvailable = 0;
                bufDesc_.dataLength = 0;
                break;
        }
        AUDIO_INFO_LOG("mutexLock_.unlock");
        mutexLock_.unlock();
        if (lSignal) {
            AUDIO_INFO_LOG("Notifing all the");
            waitAudioCbkCond_.notify_all();
        }
    }
    AUDIO_INFO_LOG("Exiting the audioRendererCallback %{public}d, mState %{public}d ", bufDesc_.dataLength, mState);
    if (bufDesc_.dataLength) {
        pAudioRenderer_->Enqueue(bufDesc_);
    }
}

bool TonePlayerPrivate::prepareWave()
{
    unsigned int segmentIdx = 0;
    AUDIO_INFO_LOG("prepareWave, Entered");
    if (pNewToneInfo_ == NULL) {
        return false;
    }

    // Remove existing wave generators if any
    clearWaveGens();

    pToneInfo_ = pNewToneInfo_;
    AUDIO_INFO_LOG("prepareWave, duration %{public}d", durationMs_);
    if (durationMs_ == -1) {
        AUDIO_INFO_LOG("prepareWave, -1 duration");
        maxSample = TONEINFO_INF;
    } else {
        if (durationMs_ > (int)(TONEINFO_INF / samplingRate_)) {
            maxSample = (durationMs_ / C1000MS) * samplingRate_;
        } else {
            maxSample = (durationMs_ * samplingRate_) / C1000MS;
        }
        AUDIO_INFO_LOG("prepareWave, duration limited to %{public}d ms  maxSample %{public}d", durationMs_, maxSample);
    }

    while (pToneInfo_->segments[segmentIdx].duration) {
        // Get total number of sine waves: needed to adapt sine wave gain.
        unsigned int lNumWaves = numWaves(segmentIdx);
        unsigned int freqIdx = 0;
        unsigned int frequency = pToneInfo_->segments[segmentIdx].waveFreq[freqIdx];
        AUDIO_INFO_LOG("prepareWave, lNumWaves %{public}d", lNumWaves);
        while (frequency && lNumWaves != 0) {
            AUDIO_INFO_LOG("prepareWave, Frequency %{public}d", frequency);
            // Instantiate a wave generator if  not already done for this frequency
            if (mWaveGens.find(frequency) == mWaveGens.end()) {
                TonePlayerPrivate::WaveGenerator *lpWaveGen =
                    new TonePlayerPrivate::WaveGenerator(samplingRate_,
                        frequency,
                        TONEINFO_GAIN/lNumWaves);
                mWaveGens[frequency] = lpWaveGen;
            }
            frequency = pNewToneInfo_->segments[segmentIdx].waveFreq[++freqIdx];
        }
        segmentIdx++;
    }

    // Initialize tone sequencer
    totalSample = 0;
    currSegment = 0;
    currCount = 0;
    loopCounter_ = 0;
    if (pToneInfo_->segments[0].duration == TONEINFO_INF) {
        nextSegSample = TONEINFO_INF;
    } else {
        nextSegSample = (pToneInfo_->segments[0].duration * samplingRate_) / C1000MS;
    }
    AUDIO_INFO_LOG("Prepare wave, nextSegSample %{public}d", nextSegSample);
    return true;
}


unsigned int TonePlayerPrivate::numWaves(unsigned int segmentIdx)
{
    unsigned int lCnt = 0;

    if (pToneInfo_->segments[segmentIdx].duration) {
        while (pToneInfo_->segments[segmentIdx].waveFreq[lCnt]) {
            lCnt++;
        }
        lCnt++;
    }

    return lCnt;
}

void TonePlayerPrivate::clearWaveGens()
{
    AUDIO_INFO_LOG("Clearing mWaveGens:");
    std::map <uint16_t, WaveGenerator *>::iterator lItr;
    for (lItr = mWaveGens.begin(); lItr !=  mWaveGens.end(); lItr++) {
        delete lItr->second;
    }
    mWaveGens.clear();
}


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
    long Sample;  // current sample

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
            Sample = ((lA1 * lS1) >> S_Q14) - lS2;
            // shift delay
            lS2 = lS1;
            lS1 = Sample;
            Sample = ((lAmplitude>>C16BIT) * Sample) >> S_Q15;
            *(outBuffer++) += (int16_t)Sample;  // put result in buffer
            lAmplitude -= dec;
        }
    } else {
        // loop generation
        while (count) {
            count--;
            Sample = ((lA1 * lS1) >> S_Q14) - lS2;
            // shift delay
            lS2 = lS1;
            lS1 = Sample;
            Sample = (lAmplitude * Sample) >> S_Q15;
            *(outBuffer++) += (int16_t)Sample;  // put result in buffer
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
} // end namespace AudioStandard
} // end OHOS
