/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#undef LOG_TAG
#define LOG_TAG "AudioCapturerSource"

#include "audio_capturer_source.h"

#include "audio_capturer_source_normal.h"
#include "audio_capturer_source_wakeup.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioCapturerSource *AudioCapturerSource::GetInstance(const std::string &halName,
    const SourceType sourceType, const char *sourceName)
{
    Trace trace("AudioCapturerSourceInner:GetInstance");
    if (halName == "usb") {
        static AudioCapturerSourceInner audioCapturerUsb(halName);
        return &audioCapturerUsb;
    }

    switch (sourceType) {
        case SourceType::SOURCE_TYPE_MIC:
        case SourceType::SOURCE_TYPE_VOICE_CALL:
            return GetMicInstance();
        case SourceType::SOURCE_TYPE_WAKEUP:
            if (!strcmp(sourceName, "Built_in_wakeup_mirror")) {
                return GetWakeupInstance(true);
            } else {
                return GetWakeupInstance(false);
            }
        default:
            AUDIO_ERR_LOG("sourceType error %{public}d", sourceType);
            return GetMicInstance();
    }
}

AudioCapturerSource *AudioCapturerSource::GetMicInstance()
{
    static AudioCapturerSourceInner audioCapturer;
    return &audioCapturer;
}

AudioCapturerSource *AudioCapturerSource::GetWakeupInstance(bool isMirror)
{
    if (isMirror) {
        static AudioCapturerSourceWakeup audioCapturerMirror;
        return &audioCapturerMirror;
    }
    static AudioCapturerSourceWakeup audioCapturer;
    return &audioCapturer;
}
} // namespace AudioStandard
} // namesapce OHOS
