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

#ifndef AUDIO_TONEPLAYER_PRIVATE_H_
#define AUDIO_TONEPLAYER_PRIVATE_H_

// #define PROPRIETARY_WAVEGEN 1 // For OHOS Wave Generation
#include <map>
#include <thread>
#include "toneplayer.h"
#include "audio_renderer.h"


namespace OHOS {
namespace AudioStandard {
class TonePlayerPrivate : public AudioRendererWriteCallback, public AudioRendererCallback, public TonePlayer,
    public std::enable_shared_from_this<TonePlayerPrivate> {
public:
    TonePlayerPrivate(const std::string cachePath, const AudioRendererInfo &rendererInfo, float volume);
    ~TonePlayerPrivate();
    bool LoadTone(ToneType toneType) override;
    bool StartTone() override;
    bool StopTone() override;
    bool Release() override;
    void OnInterrupt(const InterruptEvent &interruptEvent) override;
    void OnStateChange(const RendererState state) override;
    void OnWriteData(size_t length) override;
private:
    enum tone_player_state {
        TONE_PLAYER_IDLE,  // TonePlayer is being initialized or initialization failed
        TONE_PLAYER_INIT,  // TonePlayer has been successfully initialized and is not playing
        TONE_PLAYER_STARTING,  // TonePlayer is starting playing
        TONE_PLAYER_PLAYING,  // TonePlayer is playing
        TONE_PLAYER_STOPPING,  // TonePlayer is stoping
        TONE_PLAYER_STOPPED,  // TonePlayer is stopped: the AudioTrack will be stopped
        TONE_PLAYER_RESTARTING  // A start request was received in active state (playing or stopping)
    };
    typedef std::cv_status status_t;
    AudioRendererOptions mRendererOptions = {};
    static const constexpr float TONEINFO_GAIN = 0.9;  // Default gain passed to  WaveGenerator().

    // ToneDescriptor class contains all parameters needed to generate a tone:
    //    - The array waveFreq[]:
    //         1 for static tone descriptors: contains the frequencies of all individual waves making the multi-tone.
    //         2 for active tone descritors: contains the indexes of the WaveGenerator objects in mWaveGens
    //        The number of sine waves varies from 1 to TONEGEN_MAX_WAVES.
    //        The first null value indicates that no more waves are needed.
    //    - The array segments[] is used to generate the tone pulses. A segment is a period of time
    //        during which the tone is ON or OFF.    Segments with even index (starting from 0)
    //        correspond to tone ON state and segments with odd index to OFF state.
    //        The data stored in segments[] is the duration of the corresponding period in ms.
    //        The first segment encountered with a 0 duration    indicates that no more segment follows.
    //    - loopCnt - Number of times to repeat a sequence of seqments after playing this
    //    - loopIndx - The segment index to go back and play is loopcnt > 0
    //    - repeatCnt indicates the number of times the sequence described by segments[] array must be repeated.
    //        When the tone generator encounters the first 0 duration segment, it will compare repeatCnt to currCount.
    //        If currCount > repeatCnt, the tone is stopped automatically. Otherwise, tone sequence will be
    //        restarted from segment repeatSegment.
    //    - repeatSegment number of the first repeated segment when repeatCnt is not null

    uint32_t totalSample;  // Total number of audio samples played (gives current time)
    uint32_t nextSegSample;  // Position of next segment transition expressed in samples
    // NOTE: because totalSample, nextSegSample are stored on 32 bit, current design will operate properly
    // only if tone duration is less than about 27 Hours(@44100Hz sampling rate). If this time is exceeded,
    // no crash will occur but tone sequence will show a glitch.
    uint32_t maxSample;  // Maximum number of audio samples played (maximun tone duration)
    uint32_t durationMs_;  // Maximum tone duration in ms

    uint16_t currSegment;  // Current segment index in ToneDescriptor segments[]
    uint16_t currCount;  // Current sequence repeat count
    volatile uint16_t mState;  // TonePlayer state (tone_state)
    std::shared_ptr<ToneInfo> pToneInfo_;  // pointer to active tone Info
    std::shared_ptr<ToneInfo> pNewToneInfo_;  // pointer to next active tone Info
    std::vector<int32_t> supportedTones_;
    std::string cachePath_; // NAPI interface to create AudioRenderer
    uint16_t loopCounter_; // Current tone loopback count

    uint32_t samplingRate_;  // AudioFlinger Sampling rate
    std::unique_ptr<AudioRenderer> pAudioRenderer_;  // Pointer to AudioRenderer used for playback
    std::mutex mutexLock_; // Mutex to control concurent access to TonePlayer object
    // from audio callback and application API
    std::mutex cbkCondLock_; // Mutex associated to mWaitCbkCond
    std::condition_variable waitAudioCbkCond_; // condition enabling interface
    // to wait for audio callback completion after a change is requested
    float volume_;  // Volume applied to audio Renderer
    uint32_t processSize_;  // Size of audio blocks generated at a time by audioCallback() (in PCM frames).
    struct timespec startTime_; // tone start time: needed to guaranty actual tone duration

    bool initAudioRenderer();
    void audioRendererCallback();
    bool prepareWave();
    unsigned int numWaves(unsigned int segmentIdx);
    void clearWaveGens();

    // WaveGenerator generates a single sine wave
    class WaveGenerator {
    public:
        enum gen_command {
            WAVEGEN_START,  // Start/restart wave from phase 0
            WAVEGEN_CONT,  // Continue wave from current phase
            WAVEGEN_STOP  // Stop wave on zero crossing
        };

        WaveGenerator(uint32_t samplingRate, uint16_t frequency,
                float volume);
        ~WaveGenerator();

        void getSamples(int16_t *outBuffer, unsigned int count,
                unsigned int command);

    private:
        static const int16_t GEN_AMP = 32000;  // amplitude of generator
        static const int16_t S_Q14 = 14;  // shift for Q14
        static const int16_t S_Q15 = 15;  // shift for Q15

        int16_t mA1_Q14;  // Q14 coefficient
        // delay line of full amplitude generator
#ifndef PROPRIETARY_WAVEGEN
        long mS1, mS2;  // delay line S2 oldest
#else
        double mS1, mS2;  // delay line S2 oldest
#endif
        int16_t mS2_0;  // saved value for reinitialisation
        int16_t mAmplitude_Q15;  // Q15 amplitude
    };

    // KeyedVector<uint16_t, WaveGenerator *> mWaveGens;  // list of active wave generators.
    std::map <uint16_t, WaveGenerator *> mWaveGens;

    std::string mOpPackageName;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_TONEPLAYER_H_ */
