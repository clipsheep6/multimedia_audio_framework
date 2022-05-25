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

#include "audio_capturer_file_source.h"

#include <cstring>
#include <dlfcn.h>
#include <string>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioCapturerFileSource::AudioCapturerFileSource()
{
}

AudioCapturerFileSource::~AudioCapturerFileSource()
{
    DeInit();
}

AudioCapturerFileSource *AudioCapturerFileSource::GetInstance()
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s", __func__);
    static AudioCapturerFileSource audioCapturer;

    return &audioCapturer;
}

void AudioCapturerFileSource::DeInit()
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s", __func__);
    if (filePtr != nullptr) {
        fclose(filePtr);
        filePtr = nullptr;
    }
}

int32_t AudioCapturerFileSource::Init(const char *filePath)
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s filePath is %{public}s", __func__, filePath);
    filePtr = fopen(filePath, "rb");
    if (filePtr == nullptr) {
        AUDIO_ERR_LOG("Error opening pcm test file!");
        return ERROR;
    }

    return SUCCESS;
}

int32_t AudioCapturerFileSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s", __func__);
    if (filePtr == nullptr) {
        AUDIO_ERR_LOG("Invalid filePtr!");
        return ERROR;
    }

    if (feof(filePtr)) {
        AUDIO_ERR_LOG("End of the file reached, start reading from beginning");
        rewind(filePtr);
    }

    replyBytes = fread(frame, 1, requestBytes, filePtr);

    return SUCCESS;
}

int32_t AudioCapturerFileSource::Start(void)
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s", __func__);
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Stop(void)
{
    AUDIO_ERR_LOG("AudioCapturerFileSource %{public}s", __func__);
    if (filePtr != nullptr) {
        fclose(filePtr);
        filePtr = nullptr;
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

AudioCapturerFileSource *g_fileSourceInstance = AudioCapturerFileSource::GetInstance();

int32_t AudioCapturerFileSourceInit(const char *filePath)
{
    return g_fileSourceInstance->Init(filePath);
}

void AudioCapturerFileSourceDeInit()
{
    g_fileSourceInstance->DeInit();
}

int32_t AudioCapturerFileSourceStop()
{
    return SUCCESS;
}

int32_t AudioCapturerFileSourceStart()
{
    return SUCCESS;
}

int32_t AudioCapturerFileSourceSetMute()
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSourceIsMuteRequired()
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSourceFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    return g_fileSourceInstance->CaptureFrame(frame, requestBytes, replyBytes);
}

int32_t AudioCapturerFileSourceSetVolume(float left, float right)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSourceGetVolume(float *left, float *right)
{
    return ERR_NOT_SUPPORTED;
}
#ifdef __cplusplus
}
#endif
