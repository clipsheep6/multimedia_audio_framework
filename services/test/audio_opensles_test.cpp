/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include<OpenSLES.h>
#include<OpenSLES_Platform.h>

static void BuqqerQueueCallback (SLBufferQueueItf bufferQueueitf, void *pContext);
static void PlayerStart (SLPlayItf playItf, SLBufferQueueItf bufferQueueItf);
static void PlayerStop (SLPlayItf playItf);

int main(int argc, char *argv[])
{
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;
    SLObjectItf pcmPlayerObject = nullptr;
    SLresult result;

    //create engine
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    
    //get engine itf
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    
    //create outputmix
    SLObjectItf outputMixObject = nullptr;
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    //config audio sink
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slSink = {&slOutputMix, nullptr};

    //config audio source
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };

    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM, // formatType
        0, // numChannels
        0, // samplesPerSec
        0, // bitsPerSample
        0, // containerSize
        0, // channelMask
        0 // endianness
    };

    SLDataSource slSource = {&slBufferQueue, &pcmFormat};

    //create audioplayer
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slSource, &slSink, 3, nullptr, nullptr);
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //get play itf
    SLPlayItf playItf;
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &playItf);

    //get volume itf
    SLVolumeItf volumeItf;
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &volumeItf);

    //get play itf
    SLBufferQueueItf bufferQueueItf;
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &bufferQueueItf);

    //register callback
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BuqqerQueueCallback, nullptr);

    PlayerStart(playItf, bufferQueueItf);

    PlayerStop(playItf);

    return 0;
}

static void BuqqerQueueCallback (SLBufferQueueItf bufferQueueitf, void *pContext)
{
    //write pcm
    (*bufferQueueitf)->Enqueue(bufferQueueitf, nullptr, 0);
    return;
}

static void PlayerStart (SLPlayItf playItf, SLBufferQueueItf bufferQueueItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    (*bufferQueueItf)->Enqueue(bufferQueueItf, nullptr, 0);
    return;
}

static void PlayerStop (SLPlayItf playItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    return;
}
