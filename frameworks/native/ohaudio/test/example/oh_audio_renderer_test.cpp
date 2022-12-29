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
#include <OHAudio.h>
#include <native_audiorenderer.h>

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char *argv[])
{
    OH_AudioStream_Result ret;
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RERNDERER;
    ret = OH_AudioStream_CreateBuilder(&builder, type);

    std::cout << ret <<std::endl;
    OH_AudioRenderer* audioRenderer;
    ret = OH_AudioRendererBuilder_Generate(builder, &audioRenderer);
    std::cout << ret <<std::endl;

    
    ret = OH_AudioRenderer_Start(audioRenderer);
    std::cout << ret <<std::endl;

    ret = OH_AudioRenderer_Release(audioRenderer);
    std::cout << ret <<std::endl;
    return 0;
}

#ifdef __cplusplus
}
#endif
