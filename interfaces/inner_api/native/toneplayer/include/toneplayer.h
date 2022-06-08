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
class TonePlayer {
public:
/**
 * @brief Defines information about ToneType Enum
 */
     // List of all available tones
    // This enum must be kept consistant with constants in TonePlayer
    enum tone_type {
        // DTMF tones  ITU-T Recommendation Q.23
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
        // Call supervisory tones:  3GPP TS 22.001 (CEPT)
        TONE_TYPE_CALL_SUP_DIAL,  // Dial tone: CEPT: 425Hz, continuous
        FIRST_SUP_TONE =   TONE_TYPE_CALL_SUP_DIAL,
        TONE_SUP_BUSY,  // Busy tone, CEPT: 425Hz, 500ms ON, 500ms OFF...
        TONE_SUP_CONGESTION,  // Congestion tone CEPT, JAPAN: 425Hz, 200ms ON, 200ms OFF...
        TONE_SUP_RADIO_ACK,  // Radio path acknowlegment, CEPT, ANSI: 425Hz, 200ms ON
        TONE_SUP_RADIO_NOTAVAIL,  // Radio path not available: 425Hz, 200ms ON, 200 OFF 3 bursts
        TONE_SUP_ERROR,  // Error/Special info:  950Hz+1400Hz+1800Hz, 330ms ON, 1s OFF...
        TONE_SUP_CALL_WAITING,  // Call Waiting CEPT,JAPAN:  425Hz, 200ms ON, 600ms OFF, 200ms ON, 3s OFF...
        TONE_SUP_RINGTONE,  // Ring Tone CEPT, JAPAN:  425Hz, 1s ON, 4s OFF...
        LAST_SUP_TONE = TONE_SUP_RINGTONE,
        // Proprietary tones:  3GPP TS 31.111
        TONE_TYPE_CALL_PROP_BEEP,  // General beep: 400Hz+1200Hz, 35ms ON
        TONE_PROP_ACK,  // Positive Acknowlgement: 1200Hz, 100ms ON, 100ms OFF 2 bursts
        TONE_PROP_NACK,  // Negative Acknowlgement: 300Hz+400Hz+500Hz, 400ms ON
        TONE_PROP_PROMPT,  // Prompt tone: 400Hz+1200Hz, 200ms ON
        TONE_PROP_BEEP2,  // General double beep: 400Hz+1200Hz, 35ms ON, 200ms OFF, 35ms on
        // Additional call supervisory tones: specified by IS-95 only
        TONE_SUP_INTERCEPT, // Intercept tone: alternating 440 Hz and 620 Hz tones, each on for 250 ms.
        TONE_SUP_INTERCEPT_ABBREV, // Abbreviated intercept: intercept tone limited to 4 seconds
        TONE_SUP_CONGESTION_ABBREV, // Abbreviated congestion: congestion tone limited to 4 seconds
        TONE_SUP_CONFIRM, // Confirm tone: a 350 Hz tone added to a 440 Hz tone repeated 
        // 3 times in a 100 ms on, 100 ms off cycle.
        TONE_SUP_PIP, // Pip tone: four bursts of 480 Hz tone (0.1 s on, 0.1 s off).

        // CDMA Tones
        TONE_CDMA_DIAL_TONE_LITE,
        TONE_CDMA_NETWORK_USA_RINGBACK,
        TONE_CDMA_INTERCEPT,
        TONE_CDMA_ABBR_INTERCEPT,
        TONE_CDMA_REORDER,
        TONE_CDMA_ABBR_REORDER,
        TONE_CDMA_NETWORK_BUSY,
        TONE_CDMA_CONFIRM,
        TONE_CDMA_ANSWER,
        TONE_CDMA_NETWORK_CALLWAITING,
        TONE_CDMA_PIP,

        // ISDN
        TONE_CDMA_CALL_SIGNAL_ISDN_NORMAL,  // ISDN Alert Normal
        TONE_CDMA_CALL_SIGNAL_ISDN_INTERGROUP, // ISDN Intergroup
        TONE_CDMA_CALL_SIGNAL_ISDN_SP_PRI, // ISDN SP PRI
        TONE_CDMA_CALL_SIGNAL_ISDN_PAT3,  // ISDN Alert PAT3
        TONE_CDMA_CALL_SIGNAL_ISDN_PING_RING, // ISDN Alert PING RING
        TONE_CDMA_CALL_SIGNAL_ISDN_PAT5,  // ISDN Alert PAT5
        TONE_CDMA_CALL_SIGNAL_ISDN_PAT6,  // ISDN Alert PAT6
        TONE_CDMA_CALL_SIGNAL_ISDN_PAT7,  // ISDN Alert PAT7
        // ISDN end

        // IS54
        TONE_CDMA_HIGH_L,  // IS54 High Pitch Long
        TONE_CDMA_MED_L, // IS54 Med Pitch Long
        TONE_CDMA_LOW_L, // IS54 Low Pitch Long
        TONE_CDMA_HIGH_SS, // IS54 High Pitch Short Short
        TONE_CDMA_MED_SS, // IS54 Medium Pitch Short Short
        TONE_CDMA_LOW_SS, // IS54 Low Pitch Short Short
        TONE_CDMA_HIGH_SSL, // IS54 High Pitch Short Short Long
        TONE_CDMA_MED_SSL, // IS54 Medium  Pitch Short Short Long
        TONE_CDMA_LOW_SSL, // IS54 Low  Pitch Short Short Long
        TONE_CDMA_HIGH_SS_2, // IS54 High Pitch Short Short 2
        TONE_CDMA_MED_SS_2, // IS54 Med Pitch Short Short 2
        TONE_CDMA_LOW_SS_2, // IS54 Low  Pitch Short Short 2
        TONE_CDMA_HIGH_SLS, // IS54 High Pitch Short Long Short
        TONE_CDMA_MED_SLS, // IS54 Med Pitch Short Long Short
        TONE_CDMA_LOW_SLS, // IS54 Low Pitch Short Long Short
        TONE_CDMA_HIGH_S_X4, // IS54 High Pitch Short Short Short Short
        TONE_CDMA_MED_S_X4, // IS54 Med Pitch Short Short Short Short
        TONE_CDMA_LOW_S_X4, // IS54 Low Pitch Short Short Short Short
        TONE_CDMA_HIGH_PBX_L, // PBX High Pitch Long
        TONE_CDMA_MED_PBX_L, // PBX Med Pitch Long
        TONE_CDMA_LOW_PBX_L, // PBX Low  Pitch Long
        TONE_CDMA_HIGH_PBX_SS, // PBX High Short Short
        TONE_CDMA_MED_PBX_SS, // PBX Med Short Short
        TONE_CDMA_LOW_PBX_SS, // PBX Low  Short Short
        TONE_CDMA_HIGH_PBX_SSL, // PBX High Short Short Long
        TONE_CDMA_MED_PBX_SSL, // PBX Med Short Short Long
        TONE_CDMA_LOW_PBX_SSL,  // PBX Low Short Short Long
        TONE_CDMA_HIGH_PBX_SLS, // PBX High  SLS
        TONE_CDMA_MED_PBX_SLS,  // PBX Med SLS
        TONE_CDMA_LOW_PBX_SLS, // PBX Low SLS
        TONE_CDMA_HIGH_PBX_S_X4, // PBX High SSSS
        TONE_CDMA_MED_PBX_S_X4, // PBX Med SSSS
        TONE_CDMA_LOW_PBX_S_X4, // PBX LOW SSSS
        // IS54 end
        // proprietary
        TONE_CDMA_ALERT_NETWORK_LITE,
        TONE_CDMA_ALERT_AUTOREDIAL_LITE,
        TONE_CDMA_ONE_MIN_BEEP,
        TONE_CDMA_KEYPAD_VOLUME_KEY_LITE,
        TONE_CDMA_PRESSHOLDKEY_LITE,
        TONE_CDMA_ALERT_INCALL_LITE,
        TONE_CDMA_EMERGENCY_RINGBACK,
        TONE_CDMA_ALERT_CALL_GUARD,
        TONE_CDMA_SOFT_ERROR_LITE,
        TONE_CDMA_CALLDROP_LITE,
        // proprietary end
        TONE_CDMA_NETWORK_BUSY_ONE_SHOT,
        TONE_CDMA_ABBR_ALERT,
        TONE_CDMA_SIGNAL_OFF,
        // CDMA end
        NUM_TONES,
        NUM_SUP_TONES = LAST_SUP_TONE-FIRST_SUP_TONE+1
    };
    static std::shared_ptr<TonePlayer> Create(const AudioRendererInfo &rendererInfo);
    virtual bool loadTone(tone_type toneType)=0;
    virtual bool startTone()=0;
    virtual void stopTone()=0;

    virtual ~TonePlayer();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_TONEPLAYER_H
