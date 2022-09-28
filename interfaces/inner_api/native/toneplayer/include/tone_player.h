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

#ifndef AUDIO_TONEPLAYER_H
#define AUDIO_TONEPLAYER_H

#include <memory>

#include "audio_info.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines information about ToneType Enum
 */
    // List of all available tones
    // This enum must be kept consistant with constants in TonePlayer
enum ToneType {
    // DTMF tones ITU-T Recommendation Q.23
    TONE_TYPE_DIAL_0 = 0,  // 0 key: 1336Hz, 941Hz
    TONE_TYPE_DIAL_1,  // 1 key: 1209Hz, 697Hz
    TONE_TYPE_DIAL_2,  // 2 key: 1336Hz, 697Hz
    TONE_TYPE_DIAL_3,  // 3 key: 1477Hz, 697Hz
    TONE_TYPE_DIAL_4,  // 4 key: 1209Hz, 770Hz
    TONE_TYPE_DIAL_5,  // 5 key: 1336Hz, 770Hz
    TONE_TYPE_DIAL_6,  // 6 key: 1477Hz, 770Hz
    TONE_TYPE_DIAL_7,  // 7 key: 1209Hz, 852Hz
    TONE_TYPE_DIAL_8,  // 8 key: 1336Hz, 852Hz
    TONE_TYPE_DIAL_9,  // 9 key: 1477Hz, 852Hz
    TONE_TYPE_DIAL_S,  // * key: 1209Hz, 941Hz
    TONE_TYPE_DIAL_P,  // # key: 1477Hz, 941Hz
    TONE_TYPE_DIAL_A,  // A key: 1633Hz, 697Hz
    TONE_TYPE_DIAL_B,  // B key: 1633Hz, 770Hz
    TONE_TYPE_DIAL_C,  // C key: 1633Hz, 852Hz
    TONE_TYPE_DIAL_D,  // D key: 1633Hz, 941Hz

    // Call supervisory tones: 3GPP TS 22.001 (CEPT)
    TONE_TYPE_COMMON_SUPERVISORY_DIAL = 100, // 16 Dial tone: CEPT: 425Hz, continuous
    FIRST_SUPERVISORY_TONE = TONE_TYPE_COMMON_SUPERVISORY_DIAL,
    TONE_TYPE_COMMON_SUPERVISORY_BUSY,  // Busy tone, CEPT: 425Hz, 500ms ON, 500ms OFF...
    TONE_TYPE_COMMON_SUPERVISORY_CONGESTION,  // Congestion tone CEPT, JAPAN: 425Hz, 200ms ON, 200ms OFF...
    TONE_TYPE_COMMON_SUPERVISORY_RADIO_ACK,  // Radio path acknowlegment, CEPT, ANSI: 425Hz, 200ms ON
    TONE_TYPE_COMMON_SUPERVISORY_RADIO_NOTAVAIL,  // Radio path not available: 425Hz, 200ms ON, 200 OFF 3 bursts
    TONE_TYPE_COMMON_SUPERVISORY_ERROR,  // Error/Special info: 950Hz+1400Hz+1800Hz, 330ms ON, 1s OFF...
    TONE_TYPE_COMMON_SUPERVISORY_CALL_WAITING,  // JAPAN: 425Hz, 200ms ON, 600ms OFF, 200ms ON, 3s OFF...
    TONE_TYPE_COMMON_SUPERVISORY_RINGTONE,  // Ring Tone CEPT, JAPAN: 425Hz, 1s ON, 4s OFF...
    LAST_SUPERVISORY_TONE = TONE_TYPE_COMMON_SUPERVISORY_RINGTONE,

    // Proprietary tones:  3GPP TS 31.111
    TONE_TYPE_COMMON_PROPRIETARY_BEEP = 200,  // General beep: 400Hz+1200Hz, 35ms ON
    TONE_TYPE_COMMON_PROPRIETARY_ACK,  // Positive Acknowlgement: 1200Hz, 100ms ON, 100ms OFF 2 bursts
    TONE_TYPE_COMMON_PROPRIETARY_NACK,  // Negative Acknowlgement: 300Hz+400Hz+500Hz, 400ms ON
    TONE_TYPE_COMMON_PROPRIETARY_PROMPT,  // Prompt tone: 400Hz+1200Hz, 200ms ON
    TONE_TYPE_COMMON_PROPRIETARY_BEEP2,  // General double beep: 400Hz+1200Hz, 35ms ON, 200ms OFF, 35ms on

    NUM_TONES,
    NUM_SUP_TONES = LAST_SUPERVISORY_TONE-FIRST_SUPERVISORY_TONE + 1
    };
class TonePlayer {
public:
    static std::shared_ptr<TonePlayer> Create(const std::string cachePath, const AudioRendererInfo &rendererInfo);
    static std::shared_ptr<TonePlayer> Create(const AudioRendererInfo &rendererInfo);
    virtual bool LoadTone(ToneType toneType) = 0;
    virtual bool StartTone() = 0;
    virtual bool StopTone() = 0;
    virtual bool Release() = 0;
    virtual ~TonePlayer();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_TONEPLAYER_H
