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

#include "i_audio_capturer_source.h"

#include <cstring>
#include <string>

#include "audio_capturer_file_source.h"
#include "audio_capturer_source.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "remote_audio_capturer_source.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
IAudioCapturerSource *IAudioCapturerSource::GetInstance(const char *devceClass, const char *deviceNetworkId)
{
    AUDIO_DEBUG_LOG("%{public}s Source:GetInstance[%{public}s]", devceClass, deviceNetworkId);
    const char *deviceClassPrimary = "primary";
    const char *deviceClassA2DP = "a2dp";
    const char *deviceClassFile = "file_io";
    const char *deviceClassRemote = "remote";

    if (!strcmp(devceClass, deviceClassPrimary)) {
        return AudioCapturerSource::GetInstance();
    }
    if (!strcmp(devceClass, deviceClassA2DP)) {
        static AudioCapturerFileSource audioCapturer;
        return &audioCapturer;
    }
    if (!strcmp(devceClass, deviceClassFile)) {
        static AudioCapturerFileSource audioCapturer;
        return &audioCapturer;
    }
    if (!strcmp(devceClass, deviceClassRemote)) {
        std::string networkId = deviceNetworkId;
        RemoteAudioCapturerSource *rSource = RemoteAudioCapturerSource::GetInstance(networkId);
        return rSource;
    }
    return nullptr;
}
} // namespace AudioStandard
} // namesapce OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

int32_t FillinSourceWapper(const char *deviceClass, const char *deviceNetworkId, void **wapper)
{
    IAudioCapturerSource *iSource = IAudioCapturerSource::GetInstance(deviceClass, deviceNetworkId);

    if (iSource != nullptr) {
        *wapper = static_cast<void *>(iSource);
        return SUCCESS;
    }
    return ERROR;
}

IAudioCapturerSource *iAudioCapturerSource = nullptr;

int32_t IAudioCapturerSourceInit(void *wapper, IAudioSourceAttr *attr)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (iAudioCapturerSource->IsInited())
        return SUCCESS;

    ret = iAudioCapturerSource->Init(*attr);

    return ret;
}

void IAudioCapturerSourceDeInit(void *wapper)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_LOG(iAudioCapturerSource != nullptr, "null audioCapturerSource");
    if (iAudioCapturerSource->IsInited())
        iAudioCapturerSource->DeInit();
}

int32_t IAudioCapturerSourceStop(void *wapper)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited())
        return SUCCESS;

    ret = iAudioCapturerSource->Stop();

    return ret;
}

int32_t IAudioCapturerSourceStart(void *wapper)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = iAudioCapturerSource->Start();

    return ret;
}

int32_t IAudioCapturerSourceFrame(void *wapper, char *frame, uint64_t requestBytes, uint64_t *replyBytes)
{
    int32_t ret;
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = iAudioCapturerSource->CaptureFrame(frame, requestBytes, *replyBytes);

    return ret;
}

int32_t IAudioCapturerSourceSetVolume(void *wapper, float left, float right)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = iAudioCapturerSource->SetVolume(left, right);

    return ret;
}

int32_t IAudioCapturerSourceGetVolume(void *wapper, float *left, float *right)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }
    ret = iAudioCapturerSource->GetVolume(*left, *right);

    return ret;
}

bool IAudioCapturerSourceIsMuteRequired(void *wapper)
{
    bool muteStat = false;
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, muteStat, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return muteStat;
    }
    iAudioCapturerSource->GetMute(muteStat);
    return muteStat;
}

int32_t IAudioCapturerSourceSetMute(void *wapper, bool isMute)
{
    int32_t ret;
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited()) {
        AUDIO_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = iAudioCapturerSource->SetMute(isMute);

    return ret;
}
#ifdef __cplusplus
}
#endif
