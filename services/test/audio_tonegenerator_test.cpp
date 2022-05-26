/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

#include "audio_toneplayer_private.h"

#include "audio_log.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

#define REQ_ARG 2
int main(int argc, char *argv[])
{
    AudioRendererInfo rendererInfo = {};
    rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererInfo.rendererFlags = 0;
    int32_t toneType;
    constexpr int32_t STOP_RENDER_TIME_SECONDS = 6;

    shared_ptr<TonePlayer> lToneGen = TonePlayer::Create(rendererInfo);
    if (argc < REQ_ARG) {
        for (toneType = 0 ; toneType < 25; toneType++) {
            cout << "Entered  the Tone Type as argument to play" << endl << endl;
            AUDIO_INFO_LOG("Start Tone for %{public}d ", toneType );
            cout << "Entered  the Tone Type as argument to play" << endl << endl;
            lToneGen->loadTone((TonePlayer::tone_type)toneType);
            lToneGen->startTone();
            sleep(STOP_RENDER_TIME_SECONDS);
            lToneGen->stopTone();
            AUDIO_INFO_LOG("Stop Tone for %{public}d", toneType);
            sleep(STOP_RENDER_TIME_SECONDS);
        }
        return 0;
    }
    toneType = atoi(argv[1]);
    cout << "Entered  the Tone Type as argument to play" << endl << endl;
    AUDIO_INFO_LOG("Start Tone for %{public}d ", toneType );
    cout << "Entered  the Tone Type as argument to play" << endl << endl;
    lToneGen->loadTone((TonePlayer::tone_type)toneType);
    lToneGen->startTone();
    sleep(STOP_RENDER_TIME_SECONDS);
    lToneGen->stopTone();
    AUDIO_INFO_LOG("Stop Tone for %{public}d", toneType);
    sleep(STOP_RENDER_TIME_SECONDS);
    return 0;
}

