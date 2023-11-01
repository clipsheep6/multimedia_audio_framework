/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <thread>
#include <chrono>
#include <ctime>
#include "common/native_audiostreambuilder.h"
#include "native_audiorenderer.h"

#ifdef __cplusplus
extern "C" {
#endif
constexpr int32_t WAIT_INTERVAL = 1000;
constexpr int32_t FILE_PATH_INDEX = 1;
constexpr int32_t SAMPLE_RATE_INDEX = 2;
constexpr int32_t CHANNEL_COUNT_INDEX = 3;
constexpr int32_t LATENCY_MODE_INDEX = 4;

std::string g_pcmPath = "/data/data/oh_test_audio.pcm";
std::string g_metaPath = "/data/data/oh_test_audio_meta.dat";

FILE *g_pcm_file = nullptr;
FILE *g_meta_file = nullptr;
bool g_readEnd = false;
int32_t g_samplingRate = 48000;
int32_t g_channelCount = 2;
int32_t g_latencyMode = 0;

static int32_t AudioRendererOnWriteDataWithMeta(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen,
    void* metaBuffer,
    int32_t metaLen)
{
    size_t readCount = fread(buffer, bufferLen, 1, g_pcm_file);
    // readCount += fread(metaBuffer, metaLen, 1, g_meta_file);
    if (!readCount)
    {
        if (ferror(g_pcm_file) || ferror(g_meta_file)) {
            printf("Error reading myfile");
        } else if (feof(g_pcm_file) || feof(g_meta_file)) {
            printf("EOF found");
            g_readEnd = true;
        }
    }
    return 0;
}

void PlayerTest() {
    OH_AudioStream_Result ret;

    // 1. create builder
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    ret = OH_AudioStreamBuilder_Create(&builder, type);
    printf("createcallback ret: %d \n", ret);

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, g_samplingRate);
    OH_AudioStreamBuilder_SetChannelCount(builder, g_channelCount);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)g_latencyMode);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteDataWithMeta = AudioRendererOnWriteDataWithMeta;
    ret = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    printf("setcallback ret: %d \n", ret);
    
    // 3. create OH_AudioRenderer
    OH_AudioRenderer* audioRenderer;
    ret = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    printf("create renderer client, ret: %d \n", ret);

    // 4. start
    ret = OH_AudioRenderer_Start(audioRenderer);
    printf("start ret: %d \n", ret);
    int32_t frameSize;
    OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, &frameSize);
    printf("framesize: %d \n", frameSize);

    int timer = 0;
    while (!g_readEnd) {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_INTERVAL));
        int64_t frames;
        OH_AudioRenderer_GetFramesWritten(audioRenderer, &frames);
        printf("Wait for the audio to finish playing.(..%d s) frames:%ld\n", ++timer, frames);
        int64_t framePosition;
        int64_t timestamp;
        OH_AudioRenderer_GetTimestamp(audioRenderer, CLOCK_MONOTONIC, &framePosition, &timestamp);
        printf("framePosition %ld timestamp:%ld\n", framePosition, timestamp);
    }
    // 5. stop and release client
    ret = OH_AudioRenderer_Stop(audioRenderer);
    printf("stop ret: %d \n", ret);
    ret = OH_AudioRenderer_Release(audioRenderer);
    printf("release ret: %d \n", ret);

    // 6. destroy the builder
    ret = OH_AudioStreamBuilder_Destroy(builder);
    printf("destroy builder ret: %d \n", ret);
}

int main(int argc, char *argv[])
{
    printf("start \n");
    if ((argv == nullptr) || (argc < 3)) {
        printf("input parms wrong. input format: filePath samplingRate channelCount latencyMode\n");
        printf("input demo: ./oh_audio_3d_renderer_test ./oh_test_audio.pcm 48000 2 1 \n");
        return 0;
    }
    printf("argc=%d ", argc);
    printf("file path =%s ", argv[FILE_PATH_INDEX]);
    printf("sample rate =%s ", argv[SAMPLE_RATE_INDEX]);
    printf("channel count =%s \n", argv[CHANNEL_COUNT_INDEX]);
    printf("latency mode =%s \n", argv[LATENCY_MODE_INDEX]);

    g_pcmPath = argv[FILE_PATH_INDEX];
    g_samplingRate = atoi(argv[SAMPLE_RATE_INDEX]);
    g_channelCount = atoi(argv[CHANNEL_COUNT_INDEX]);
    g_latencyMode = atoi(argv[LATENCY_MODE_INDEX]);

    printf("filePATH: %s \n", g_pcmPath.c_str());

    g_pcm_file = fopen(g_pcmPath.c_str(), "rb");
    if (g_pcm_file == nullptr) {
        printf("OHAudioRendererTest: Unable to open file \n");
        return 0;
    }

    PlayerTest();

    fclose(g_pcm_file);
    g_pcm_file = nullptr;
    return 0;
}

#ifdef __cplusplus
}
#endif