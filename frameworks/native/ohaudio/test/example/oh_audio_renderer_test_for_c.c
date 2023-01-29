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
#include <stdio.h>

#include <OHAudio.h>
#include <native_audiorenderer.h>
#include <native_audiocapturer.h>

int main(int argc, char *argv[])
{

    enum  OH_AudioStream_Result ret;
    OH_AudioStreamBuilder* builder;
//    enum OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RERNDERER;
 //   ret = OH_AudioStream_CreateBuilder(&builder, type);

    enum OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    ret = OH_AudioStream_CreateBuilder(&builder, type);
    printf("ret: %d", ret);

    // OH_AudioRenderer* audioRenderer;
    OH_AudioCapturer* audioCapturer;
    ret = OH_AudioCapturerBuilder_Generate(builder, &audioCapturer);
    printf("ret: %d", ret);

    
    //ret = OH_AudioRenderer_Start(audioCapturer);
    ret = OH_AudioCapturer_Start(audioCapturer);
    printf("ret: %d", ret);

    // ret = OH_AudioRenderer_Release(audioCapturer);
     ret = OH_AudioCapturer_Release(audioCapturer);
    printf("ret: %d", ret);

//  enum  OH_AudioStream_Result ret;
//  OH_AudioStreamBuilder* builder;
//  enum OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RERNDERER;
//  ret = OH_AudioStream_CreateBuilder(&builder, type);
//
//  printf("ret: %d", ret);
//
//   OH_AudioRenderer* audioRenderer;
//  ret = OH_AudioRendererBuilder_Generate(builder, &audioRenderer);
//  printf("ret: %d", ret);
//
//  
//  ret = OH_AudioRenderer_Start(audioRenderer);
//  printf("ret: %d", ret);
//
//   ret = OH_AudioRenderer_Release(audioRenderer);
//  printf("ret: %d", ret);
    return 0;
}
