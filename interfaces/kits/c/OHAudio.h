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

#include "common/native_audiostream_builder.h"

extern "C" {

/**
 * Create a OH_AudioStreamBuilder that can be used to open a Stream.
 *
 * OH_AudioStream_Destroy() must be called when you are done using the builder.
 *
 * @param builder The builder reference to the created result. 
 * @param type The stream type to be created. {@link #AUDIOSTREAM_TYPE_RERNDERER} or {@link #AUDIOSTREAM_TYPE_CAPTURER}
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStream_CreateBuilder(OH_AudioStreamBuilder** builder, OH_AudioStream_Type type);

/**
 * Destroy a OH_AudioStreamBuilder.
 *
 * This function must be called when you are done using the builder.
 *
 * @param builder Reference provided by AAudio_createStreamBuilder()
 * @return {@link #AUDIOSTREAM_SUCCESS} or an undesired error.
 */
OH_AudioStream_Result OH_AudioStreamBuilder_Destroy(OH_AudioStreamBuilder* builder);

}
