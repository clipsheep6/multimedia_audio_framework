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
#ifndef AUDIO_CONVERTER_3DA
#define AUDIO_CONVERTER_3DA

#include <cstdint>
#include <string>
#include <cstring>
#include "audio_info.h"
#include "audio_effect.h"
#include "audio_log.h"
#include "unistd.h"
#include "dlfcn.h"

namespace OHOS {
namespace AudioStandard {
    
class LibLoader {
public:
    LibLoader() = default;
    bool Init() ;
    void SetIOBufferConfig(bool isInput, uint32_t samplingRate, uint32_t channels);
    int32_t ApplyAlgo(AudioBuffer &inputBuffer, AudioBuffer &outputBuffer);

private:
    bool AddAlgoHandle();
    std::unique_ptr<AudioEffectLibEntry> libEntry_;
    AudioEffectHandle handle_;
    AudioEffectConfig ioBufferConfig;
    std::map<AudioChannelLayout, AudioChannelLayout> inChannelLayout2out;
};

class AudioFormatConverter3DA {
public:
    AudioFormatConverter3DA() = default;
    int32_t Init(const AudioStreamParams info);
    bool AllocateMem();
    bool IsReady();
    void SetNotReady();
    int32_t Process(const BufferDesc pcmBuf, const BufferDesc metadataBuf);
    int32_t GetInputBuffers(BufferDesc &pcmBuf, BufferDesc &metadataBuf);
    void ConvertChannels(uint8_t &channel, uint64_t &channelLayout);
    void GetOutputBufferStream(uint8_t *&buffer, uint32_t &bufferLen) ;

private:
    std::unique_ptr<uint8_t[]> inPcmBuf_;
    std::unique_ptr<uint8_t[]> metadataBuf_;
    std::unique_ptr<uint8_t[]> outPcmBuf_;

    int32_t inChannel_;
    int32_t outChannel_;

    bool isReady_;
    bool loadSuccess_;

    LibLoader externalLoader_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_converter_3DA
