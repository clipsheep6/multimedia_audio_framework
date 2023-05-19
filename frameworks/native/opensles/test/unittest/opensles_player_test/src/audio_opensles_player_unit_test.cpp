/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "audio_opensles_player_unit_test.h"

#include "common.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "pcm2wav.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    const char *g_testFilePath = "/data/test_44100_2.wav";
    FILE *wavFile_;
    wav_hdr wavHeader_;
    SLObjectItf engineObject_;
    SLObjectItf outputMixObject_;
    SLPlayItf playItf_;
    SLVolumeItf volumeItf_;
    SLOHBufferQueueItf bufferQueueItf_;
    SLObjectItf pcmPlayerObject_;
    SLEngineItf engineEngine_;
} // namespace

static void BufferQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    FILE *wavFile = (FILE *)pContext;
    if (!feof(wavFile)) {
        SLuint8 *buffer = nullptr;
        SLuint32 bufferSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, &bufferSize);
        if (buffer != nullptr) {
            fread(buffer, 1, size, wavFile);
            (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        }
    }
    return;
}

void AudioOpenslesPlayerUnitTest::SetUpTestCase(void) { }

void AudioOpenslesPlayerUnitTest::TearDownTestCase(void) { }

void AudioOpenslesPlayerUnitTest::SetUp(void) { }

void AudioOpenslesPlayerUnitTest::TearDown(void) { }

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_001, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_002, TestSize.Level0)
{
    SLresult result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_003, TestSize.Level0)
{
    SLresult result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_001, TestSize.Level0)
{
    SLresult result = (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_002, TestSize.Level0)
{
    SLresult result = (*outputMixObject_)->Realize(outputMixObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_003, TestSize.Level0)
{
    
    SLresult result = (*engineEngine_)->CreateOutputMix(engineEngine_, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateOutputMix_004, TestSize.Level0)
{
    
    SLresult result = (*engineEngine_)->CreateOutputMix(nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Realize_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->Realize(nullptr, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Resume_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->Resume(outputMixObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetState_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->GetState(nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetState_002, TestSize.Level1)
{
    SLuint32 state;
    SLresult result = (*outputMixObject_)->GetState(outputMixObject_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_001, TestSize.Level1)
{
    SLresult result = (*outputMixObject_)->RegisterCallback(outputMixObject_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_ReadOrWriteCallbackAdapter_001, TestSize.Level1)
{
    ReadOrWriteCallbackAdapter readOrWriteCallbackAdapter = new ReadOrWriteCallbackAdapter();
    EXPECT_TRUE(nullptr != readOrWriteCallbackAdapter);
    readOrWriteCallbackAdapter.OnReadData(1);
    readOrWriteCallbackAdapter.OnWriteData(1);
    delete readOrWriteCallbackAdapter;
    readOrWriteCallbackAdapter = nullptr;
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_AbortAsyncOperation_001, TestSize.Level1)
{
    (*engineObject_)->AbortAsyncOperation(outputMixObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_001, TestSize.Level0)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_001: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_001, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_001: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.NumOfChan = 1;
    wavHeader_.SamplesPerSec = 8000;
    wavHeader_.bitsPerSample = 8;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_002, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_002: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.NumOfChan = 2;
    wavHeader_.SamplesPerSec = 11025;
    wavHeader_.bitsPerSample = 16;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_003, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_003: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.NumOfChan = 3;
    wavHeader_.SamplesPerSec = 12000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_004, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_004: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.NumOfChan = 4;
    wavHeader_.SamplesPerSec = 16000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_005, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_005: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 22050;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_006, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_006: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 24000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_007, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_007: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 32000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_008, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_008: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 44100;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_009, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_009: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 48000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_010, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_010: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 64000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_011, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_011: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 96000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_014, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetAudioPlayerAdapter_001, TestSize.Level1)
{
    AudioPlayerAdapter* audioPlayerAdapter = new AudioPlayerAdapter();
    EXPECT_TRUE(nullptr != audioPlayerAdapter);
    audioPlayerAdapter = AudioPlayerAdapter::GetInstance();
    EXPECT_TRUE(nullptr != audioPlayerAdapter);
    delete audioPlayerAdapter;
    audioPlayerAdapter = nullptr;
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetAudioRenderById_001, TestSize.Level1)
{
    AudioRenderer* audioRender = AudioPlayerAdapter::GetInstance()->GetAudioRenderById(1);
    EXPECT_TRUE(nullptr != audioRender);
    AudioPlayerAdapter::GetInstance()->EraseAudioRenderById(1);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayerAdapter_001, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_009: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 48000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = AudioPlayerAdapter::GetInstance()->CreateAudioPlayerAdapter(0,
        &slSource, &slSink, OHOS::AudioStandard::STREAM_MUSIC);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_002, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(0, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_003, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(0, SL_PLAYSTATE_PAUSED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_004, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(0, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayStateAdapter_004, TestSize.Level1)
{
    SLresult result = AudioPlayerAdapter::GetInstance()->SetPlayStateAdapter(0, -1);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayStateAdapter_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = AudioPlayerAdapter::GetInstance()->GetPlayStateAdapter(-1, &state);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetCaptureStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->SetCaptureStateAdapter(-1, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetCaptureStateAdapter_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(-1, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetCaptureStateAdapter_002, TestSize.Level1)
{
    SLuint32 state = SL_RECORDSTATE_RECORDING;
    SLresult result = AudioCapturerAdapter::GetInstance()->GetCaptureStateAdapter(0, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_EnqueueAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->EnqueueAdapter(-1, nullptr, 0);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_ClearAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->ClearAdapter(-1);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetStateAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->GetStateAdapter(-1, nullptr);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetBufferAdapter_001, TestSize.Level1)
{
    SLresult result = AudioCapturerAdapter::GetInstance()->GetBufferAdapter(-1, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_RESOURCE_ERROR);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_002, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->Realize(pcmPlayerObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_003, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_PLAY, &playItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateAudioPlayer_004, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_004: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, nullptr,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetInterface_003, TestSize.Level1)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(nullptr, SL_IID_PLAY, &playItf_);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateEngine_004, TestSize.Level0)
{
    SLresult result = (*engineObject_)->GetInterface(engineObject_, SL_IID_OUTPUTMIX, &engineEngine_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateLEDDevice_001, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateLEDDevice_001: CreateLEDDevice Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLresult result = (*engineEngine_)->CreateLEDDevice(engineEngine_, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateVibraDevice_001, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateVibraDevice_001: CreateVibraDevice Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLresult result = (*engineEngine_)->CreateVibraDevice(engineEngine_, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateVibraDevice_001, TestSize.Level1)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateVibraDevice_001: CreateVibraDevice Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);

    SLresult result = (*engineEngine_)->CreateVibraDevice(engineEngine_, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateMidiPlayer_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateMidiPlayer(engineEngine_, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateListener_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateListener(engineEngine_, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Create3DGroup_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->Create3DGroup(engineEngine_, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateMetadataExtractor_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateMetadataExtractor(engineEngine_, nullptr, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_CreateExtensionObject_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->CreateExtensionObject(engineEngine_, nullptr, nullptr, 0, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QueryNumSupportedInterfaces_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QueryNumSupportedInterfaces(engineEngine_, 0, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QueryNumSupportedExtensions_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QueryNumSupportedExtensions(engineEngine_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QuerySupportedInterfaces_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QuerySupportedInterfaces(engineEngine_, 0, 0, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_QuerySupportedExtension_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->QuerySupportedExtension(engineEngine_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_IsExtensionSupported_001, TestSize.Level1)
{
    SLresult result = (*engineEngine_)->IsExtensionSupported(engineEngine_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoumeItf_001, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoume_001, TestSize.Level0)
{
    SLmillibel level = 0;
    SLresult result = (*volumeItf_)->GetVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetVoume_002, TestSize.Level0)
{
    SLmillibel level = 0;
    SLresult result = (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetVoume_001, TestSize.Level0)
{
    SLresult result = (*volumeItf_)->SetVolumeLevel(volumeItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetVoume_002, TestSize.Level0)
{
    SLmillibel level = 0;
    (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    SLresult result = (*volumeItf_)->SetVolumeLevel(volumeItf_, level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetBufferQueue_001, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_002, TestSize.Level0)
{
    SLresult result = (*bufferQueueItf_)->RegisterCallback(bufferQueueItf_, BufferQueueCallback, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_001, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PLAYING);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_002, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, -1);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetDuration_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetDuration(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_RegisterCallback_003, TestSize.Level1)
{
    SLresult result = (*playItf_)->RegisterCallback(playItf_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetCallbackEventsMask(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetCallbackEventsMask_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetCallbackEventsMask(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetMarkerPosition(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetMarkerPosition(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPositionUpdatePeriod(playItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPositionUpdatePeriod_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetPositionUpdatePeriod(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_ClearMarkerPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->ClearMarkerPosition(playItf_);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPosition_001, TestSize.Level1)
{
    SLresult result = (*playItf_)->GetPosition(playItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_003, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_PAUSED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_004, TestSize.Level0)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_005, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(nullptr, SL_PLAYSTATE_STOPPED);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_001, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PAUSED;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_002, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_PLAYING;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_003, TestSize.Level1)
{
    SLuint32 state = SL_PLAYSTATE_STOPPED;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_004, TestSize.Level1)
{
    SLuint32 state = -1;
    SLresult result = (*playItf_)->GetPlayState(playItf_, &state);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPlayState_005, TestSize.Level1)
{
    SLuint32 state = -1;
    SLresult result = (*playItf_)->GetPlayState(nullptr, &state);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPlayState_006, TestSize.Level1)
{
    SLresult result = (*playItf_)->SetPlayState(playItf_, -1);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateEngine_001, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateEngine_002, TestSize.Level1)
{
    SLresult result = slCreateEngine(nullptr, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_DestoryEngine_001, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_Realize_001, TestSize.Level0)
{
    SLresult result = (*engineObject_)->Realize(engineObject_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetInterface_001, TestSize.Level0)
{
    SLresult result = (*engineObject_)->GetInterface(engineObject_, SL_IID_ENGINE, &engineEngine_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateOutputMix_001, TestSize.Level0)
{
    SLresult result = (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_CreateAudioPlayer_001, TestSize.Level0)
{
    
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("CreateAudioPlayer_001: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_,
        &slSource, &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetVolumeLevel_001, TestSize.Level0)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*volumeItf_)->GetVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetVolumeLevel_002, TestSize.Level1)
{
    SLresult result = (*pcmPlayerObject_)->GetInterface(pcmPlayerObject_, SL_IID_VOLUME, &volumeItf_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    result = (*volumeItf_)->GetVolumeLevel(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetMaxVolumeLevel_001, TestSize.Level0)
{
    SLresult result = (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, &level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_GetMaxVolumeLevel_002, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->GetMaxVolumeLevel(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_SetVolumeLevel_001, TestSize.Level0)
{
    SLresult result = (*volumeItf_)->SetVolumeLevel(volumeItf_, level);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Prf_Audio_Opensles_SetVolumeLevel_002, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->SetVolumeLevel(nullptr, level);
    EXPECT_TRUE(result == SL_RESULT_PARAMETER_INVALID);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetMute_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->SetMute(volumeItf_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetMute_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->GetMute(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_EnableStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->EnableStereoPosition(volumeItf_, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_IsEnabledStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->IsEnabledStereoPosition(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->SetStereoPosition(volumeItf_, 0);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetStereoPosition_001, TestSize.Level1)
{
    SLresult result = (*volumeItf_)->GetStereoPosition(volumeItf_, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_GetPriority_001, TestSize.Level1)
{
    SLresult result = (*engineObject_)->GetPriority(engineObject_, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetPriority_001, TestSize.Level1)
{
    SLresult result = (*engineObject_)->SetPriority(engineObject_, 0, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_SetLossOfControlInterfaces_001, TestSize.Level1)
{
    SLresult result = (*engineObject_)->SetLossOfControlInterfaces(engineObject_, 0, nullptr, SL_BOOLEAN_FALSE);
    EXPECT_TRUE(result == SL_RESULT_FEATURE_UNSUPPORTED);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Enqueue_001, TestSize.Level1)
{
    SLresult result = (*bufferQueueItf_)->RegisterCallback(nullptr, BufferQueueCallback, wavFile_);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_001, TestSize.Level0)
{
    wavFile_ = fopen(g_testFilePath, "rb");
    if (wavFile_ == nullptr) {
        AUDIO_INFO_LOG("Destroy_010: Unable to open wave file");
    }
    size_t headerSize = sizeof(wav_hdr);
    fread(&wavHeader_, 1, headerSize, wavFile_);
    wavHeader_.SamplesPerSec = 64000;
    SLDataLocator_BufferQueue slBufferQueue = {
        SL_DATALOCATOR_BUFFERQUEUE,
        0
    };
    SLDataFormat_PCM pcmFormat = {
        SL_DATAFORMAT_PCM,
        wavHeader_.NumOfChan,
        wavHeader_.SamplesPerSec * 1000,
        wavHeader_.bitsPerSample,
        0,
        0,
        0
    };
    SLDataLocator_OutputMix slOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject_};
    SLDataSink slSink = {&slOutputMix, nullptr};
    SLDataSource slSource = {&slBufferQueue, &pcmFormat};
    SLresult result = (*engineEngine_)->CreateAudioPlayer(engineEngine_, &pcmPlayerObject_, &slSource,
        &slSink, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*pcmPlayerObject_)->Destroy(pcmPlayerObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_002, TestSize.Level0)
{
    SLresult result = slCreateEngine(&engineObject_, 0, nullptr, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*engineObject_)->Destroy(engineObject_);
    EXPECT_TRUE(true);
}

HWTEST(AudioOpenslesPlayerUnitTest, Audio_Opensles_Destroy_003, TestSize.Level0)
{
    SLresult result = (*engineEngine_)->CreateOutputMix(engineEngine_, &outputMixObject_, 0, nullptr, nullptr);
    EXPECT_TRUE(result == SL_RESULT_SUCCESS);
    (*outputMixObject_)->Destroy(outputMixObject_);
    EXPECT_TRUE(true);
}
} // namespace AudioStandard
} // namespace OHOS
