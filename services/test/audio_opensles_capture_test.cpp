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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <securec.h>
#include <iostream>
#include <unistd.h>
#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>
#include "audio_info.h"
#include "audio_log.h"

using namespace std;

static void CaptureOption(char option);

static void BuqqerQueueCallback (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile);

static void CaptureStop(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf);

static void OpenSLCaptureTest();

const int PARAMETERS = 4;
FILE *wavFile_ = nullptr;
SLObjectItf engineObject = nullptr;
SLRecordItf  recordItf;
SLOHBufferQueueItf bufferQueueItf;
SLObjectItf pcmCapturerObject = nullptr;
struct timespec tv1 = {0};
struct timespec tv2 = {0};

int main(int argc, char *argv[])
{
    AUDIO_INFO_LOG("opensl es capture test in");
    if (argc > PARAMETERS) {
        AUDIO_ERR_LOG("Incorrect number of parameters.");
        return -1;
    }
    
    int opt = 0;
    string filePath = "/data/test.pcm";
    wavFile_ = fopen(filePath.c_str(), "wb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("OpenSLES record: Unable to open file");
        return -1;
    }

    OpenSLCaptureTest();
    while ((opt = getopt(argc, argv, "a:b")) != -1) {
        switch (opt) {
            case 'a':
                CaptureOption(opt);
                break;
            case 'b':
            default:
                fflush(wavFile_);
                CaptureStop(recordItf, bufferQueueItf);
                (*pcmCapturerObject)->Destroy(pcmCapturerObject);
                fclose(wavFile_);
                wavFile_ = nullptr;
                break;
        }
    }
}

static void CaptureOption(char option)
{
    AUDIO_INFO_LOG("CaptureOption func");
    int64_t usecTimes = 1000000000;
    unsigned long long totalTime = strtoull(optarg, nullptr, 10);
    AUDIO_INFO_LOG("CaptureOption total time: %{public}llu", totalTime);
    totalTime *= usecTimes;
    clock_gettime(CLOCK_REALTIME, &tv1);

    CaptureStart(recordItf, bufferQueueItf, wavFile_);
    clock_gettime(CLOCK_REALTIME, &tv2);
    while (((tv2.tv_sec * usecTimes + tv2.tv_nsec) - (tv1.tv_sec * usecTimes + tv1.tv_nsec)) < totalTime) {
        sleep(1);
        clock_gettime(CLOCK_REALTIME, &tv2);
    }
}

static void OpenSLCaptureTest()
{
    AUDIO_INFO_LOG("OpenSLCaptureTest");
    engineObject = nullptr;
    SLEngineItf engineItf = nullptr;

    SLresult result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    SLDataLocator_IODevice io_device = {
        SL_DATALOCATOR_IODEVICE,
        SL_IODEVICE_AUDIOINPUT,
        SL_DEFAULTDEVICEID_AUDIOINPUT,
        NULL
    };

    SLDataSource audioSource = {
        &io_device,
        NULL
    };

    SLDataLocator_BufferQueue buffer_queue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        3
    };

    SLDataFormat_PCM format_pcm = {
        SL_DATAFORMAT_PCM,
        OHOS::AudioStandard::AudioChannel::MONO,
        OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_44100,
        OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE,
        0,
        0,
        0
    };
    SLDataSink audioSink = {
        &buffer_queue,
        &format_pcm
    };

    result = (*engineItf)->CreateAudioRecorder(engineItf, &pcmCapturerObject,
        &audioSource, &audioSink, 0, nullptr, nullptr);
    (*pcmCapturerObject)->Realize(pcmCapturerObject, SL_BOOLEAN_FALSE);
    
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_RECORD, &recordItf);
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf);
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BuqqerQueueCallback, wavFile_);

    return;
}

static void BuqqerQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    AUDIO_INFO_LOG("BuqqerQueueCallback");
    FILE *wavFile = (FILE *)pContext;
    if (wavFile != nullptr) {
        SLuint8 *buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        if (buffer != nullptr) {
            AUDIO_INFO_LOG("BuqqerQueueCallback, length, pSize:%{public}lu, size: %{public}lu.",
                           pSize, size);
            fwrite(buffer, 1, pSize, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        } else {
            AUDIO_INFO_LOG("BuqqerQueueCallback, buffer is null or pSize: %{public}lu, size: %{public}lu.",
                           pSize, size);
        }
    }

    return;
}

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile)
{
    AUDIO_INFO_LOG("CaptureStart");
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);
    if (wavFile != nullptr) {
        SLuint8* buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        if (buffer != nullptr) {
            AUDIO_INFO_LOG("CaptureStart, enqueue buffer length: %{public}lu.", pSize);
            fwrite(buffer, 1, pSize, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, pSize);
        } else {
            AUDIO_INFO_LOG("BuqqerQueueCallback, buffer is null or pSize: %{public}lu.", pSize);
        }
    }

    return;
}

static void CaptureStop(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf)
{
    AUDIO_INFO_LOG("CaptureStop");
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
    return;
}
