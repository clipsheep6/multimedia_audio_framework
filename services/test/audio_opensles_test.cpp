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

#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>
#include "media_log.h"
#include <iostream>
#include "pcm2wav.h"
#include <thread>
#include <unistd.h>

using namespace std;

static void BuqqerQueueCallback (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void BuqqerQueueCallback1 (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void BuqqerQueueCallback2 (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void PlayerStart (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf);

static void PlayerStart1 (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf);

static void PlayerStart2 (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf);

static void PlayerStop (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf);

static void OpenSlTest();

static void OpenSlTestAll();

FILE *wavFile_ = nullptr;
FILE *wavFile1_ = nullptr;
FILE *wavFile2_ = nullptr;
wav_hdr wavHeader_;
wav_hdr wavHeader1_;
wav_hdr wavHeader2_;
unique_ptr<thread> enqueueThread_ = nullptr;
SLObjectItf engineObject = nullptr;
SLObjectItf outputMixObject = nullptr;
SLPlayItf playItf;
SLPlayItf playItf1;
SLPlayItf playItf2;
SLOHBufferQueueItf bufferQueueItf;
SLOHBufferQueueItf bufferQueueItf1;
SLOHBufferQueueItf bufferQueueItf2;
SLObjectItf pcmPlayerObject = nullptr;
SLObjectItf pcmPlayerObject1 = nullptr;
SLObjectItf pcmPlayerObject2 = nullptr;

int main(int argc, char *argv[])
{
    MEDIA_INFO_LOG("main in");
    
    if (argc == 4) {
        // 1
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            MEDIA_ERR_LOG("Invalid path");
            return -1;
        }

        wavFile1_ = fopen(path, "rb");
        if (wavFile1_ == nullptr) {
            MEDIA_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader1_, 1, headerSize, wavFile1_);

        // 2
        headerSize = sizeof(wav_hdr);
        inputPath = argv[2];
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            MEDIA_ERR_LOG("Invalid path");
            return -1;
        }

        wavFile2_ = fopen(path, "rb");
        if (wavFile2_ == nullptr) {
            MEDIA_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader2_, 1, headerSize, wavFile2_);

        OpenSlTestAll();

        while (!feof(wavFile1_) || !feof(wavFile2_)) {
            sleep(1);
        }
        PlayerStop(playItf1, bufferQueueItf1);
        PlayerStop(playItf2, bufferQueueItf2);
        (*pcmPlayerObject1)->Destroy(pcmPlayerObject1);
        (*pcmPlayerObject2)->Destroy(pcmPlayerObject2);
        (*engineObject)->Destroy(engineObject);
        (*outputMixObject)->Destroy(outputMixObject);
        return 0;
    }

    if (argc < 2) {
        return -1;
    }
    // 1
    size_t headerSize = sizeof(wav_hdr);
    char *inputPath = argv[1];
    char path[PATH_MAX + 1] = {0x00};
    if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
        MEDIA_ERR_LOG("Invalid path");
        return -1;
    }

    wavFile_ = fopen(path, "rb");
    if (wavFile_ == nullptr) {
        MEDIA_INFO_LOG("AudioRendererTest: Unable to open wave file");
        return -1;
    }
    fread(&wavHeader_, 1, headerSize, wavFile_);

    OpenSlTest();

    while (!feof(wavFile_)) {
        sleep(1);
    }
    PlayerStop(playItf, bufferQueueItf);
    (*pcmPlayerObject)->Destroy(pcmPlayerObject);

    if (argc < 3) {
        return -1;
    }
    // 2
    char *inputPath2 = argv[2];
    char path2[PATH_MAX + 1] = {0x00};
    if ((strlen(inputPath2) > PATH_MAX) || (realpath(inputPath2, path2) == nullptr)) {
        MEDIA_ERR_LOG("Invalid path");
        return -1;
    }

    wavFile_ = fopen(path2, "rb");
    if (wavFile_ == nullptr) {
        MEDIA_INFO_LOG("AudioRendererTest: Unable to open wave file");
        return -1;
    }
    fread(&wavHeader_, 1, headerSize, wavFile_);

    OpenSlTest();

    while (!feof(wavFile_)) {
        sleep(1);
    }
    PlayerStop(playItf, bufferQueueItf);
    (*pcmPlayerObject)->Destroy(pcmPlayerObject);
    return 0;
}

static void OpenSlTest()
{   
    engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;
    SLresult result;

    MEDIA_INFO_LOG("slCreateEngine");
    //create engine
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    
    MEDIA_INFO_LOG("get engine itf");
    //get engine itf
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    
    MEDIA_INFO_LOG("create outputmix");
    
    if (*engineEngine == nullptr) {
        MEDIA_INFO_LOG("engineEngine is null");
    }
    //create outputmix
    outputMixObject = nullptr;
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    MEDIA_INFO_LOG("config audio sink");
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
        wavHeader_.NumOfChan, // numChannels *
        wavHeader_.SamplesPerSec * 1000, // samplesPerSec *
        wavHeader_.bitsPerSample, // bitsPerSample *
        0, // containerSize
        0, // channelMask
        0 // endianness
    };

    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    
    MEDIA_INFO_LOG("create audioplayer");
    //create audioplayer
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slSource, &slSink, 3, nullptr, nullptr);
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    MEDIA_INFO_LOG("get play itf");
    //get play itf
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &playItf);

    MEDIA_INFO_LOG("get volume itf");
    //get volume itf
    SLVolumeItf volumeItf;
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &volumeItf);

    SLmillibel pLevel = 0;
    (*volumeItf)->GetVolumeLevel(volumeItf, &pLevel);

    MEDIA_INFO_LOG("get bufferqueue itf");
    //get bufferqueue itf
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf);

    MEDIA_INFO_LOG("register callback");
    //register callback
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BuqqerQueueCallback, nullptr);

    PlayerStart(playItf, bufferQueueItf);
}

static void OpenSlTestAll()
{   
    engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;
    SLresult result;

    MEDIA_INFO_LOG("slCreateEngine");
    //create engine
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    
    MEDIA_INFO_LOG("get engine itf");
    //get engine itf
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    
    MEDIA_INFO_LOG("create outputmix");
    
    if (*engineEngine == nullptr) {
        MEDIA_INFO_LOG("engineEngine is null");
    }
    //create outputmix
    SLObjectItf outputMixObject = nullptr;
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, nullptr, nullptr);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    MEDIA_INFO_LOG("config audio sink");
    //config audio sink
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink slSink = {&slOutputMix, nullptr};
    
    //config audio source
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };

    SLDataFormat_PCM pcmFormat1 = {
        SL_DATAFORMAT_PCM, // formatType
        wavHeader1_.NumOfChan, // numChannels
        wavHeader1_.SamplesPerSec * 1000, // samplesPerSec, milliHertz = 1000 * Hertz
        wavHeader1_.bitsPerSample, // bitsPerSample
        0, // containerSize
        0, // channelMask
        0 // endianness
    };

    SLDataSource slSource1 = {&slBufferQueue, &pcmFormat1};

    SLDataFormat_PCM pcmFormat2 = {
        SL_DATAFORMAT_PCM, // formatType
        wavHeader2_.NumOfChan, // numChannels *
        wavHeader2_.SamplesPerSec * 1000, // samplesPerSec *
        wavHeader2_.bitsPerSample, // bitsPerSample *
        0, // containerSize
        0, // channelMask
        0 // endianness
    };

    SLDataSource slSource2 = {&slBufferQueue, &pcmFormat2};
    
    MEDIA_INFO_LOG("create audioplayer1");
    //create audioplayer
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject1, &slSource1, &slSink, 3, nullptr, nullptr);
    (*pcmPlayerObject1)->Realize(pcmPlayerObject1, SL_BOOLEAN_FALSE);

    MEDIA_INFO_LOG("create audioplayer2");
    //create audioplayer
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject2, &slSource2, &slSink, 3, nullptr, nullptr);
    (*pcmPlayerObject2)->Realize(pcmPlayerObject2, SL_BOOLEAN_FALSE);

    MEDIA_INFO_LOG("get play itf1");
    //get play itf
    (*pcmPlayerObject1)->GetInterface(pcmPlayerObject1, SL_IID_PLAY, &playItf1);

    MEDIA_INFO_LOG("get play itf2");
    //get play itf
    (*pcmPlayerObject2)->GetInterface(pcmPlayerObject2, SL_IID_PLAY, &playItf2);

    MEDIA_INFO_LOG("get bufferqueue itf1");
    //get bufferqueue itf
    (*pcmPlayerObject1)->GetInterface(pcmPlayerObject1, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf1);

    MEDIA_INFO_LOG("get bufferqueue itf2");
    //get bufferqueue itf
    (*pcmPlayerObject2)->GetInterface(pcmPlayerObject2, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf2);

    MEDIA_INFO_LOG("register callback");
    //register callback
    (*bufferQueueItf1)->RegisterCallback(bufferQueueItf1, BuqqerQueueCallback1, nullptr);
    (*bufferQueueItf2)->RegisterCallback(bufferQueueItf2, BuqqerQueueCallback2, nullptr);

    PlayerStart1(playItf1, bufferQueueItf1);
    PlayerStart2(playItf2, bufferQueueItf2);
}

static void BuqqerQueueCallback (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    if (!feof(wavFile_)) {
        SLuint8 *buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, size, wavFile_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
    }
    return;
}

static void BuqqerQueueCallback1 (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    if (!feof(wavFile1_)) {
        SLuint8 *buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, size, wavFile1_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
    }
    return;
}

static void BuqqerQueueCallback2 (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    if (!feof(wavFile2_)) {
        SLuint8 *buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, size, wavFile2_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
    }
    return;
}

static void PlayerStart (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    MEDIA_INFO_LOG("PlayerStart");
    if (!feof(wavFile_)) {
        SLuint8* buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, pSize, wavFile_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, pSize);
    }
    return;
}

static void PlayerStart1 (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    MEDIA_INFO_LOG("PlayerStart1");
    if (!feof(wavFile1_)) {
        SLuint8* buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, pSize, wavFile1_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, pSize);
    }
    return;
}

static void PlayerStart2 (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    MEDIA_INFO_LOG("PlayerStart2");
    if (!feof(wavFile2_)) {
        SLuint8* buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        fread(buffer, 1, pSize, wavFile2_);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, pSize);
    }
    return;
}

static void PlayerStop (SLPlayItf playItf, SLOHBufferQueueItf bufferQueueItf)
{
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
    MEDIA_INFO_LOG("PlayerStop");
    return;
}

