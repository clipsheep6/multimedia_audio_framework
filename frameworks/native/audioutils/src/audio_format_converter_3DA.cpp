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
#include <cstdint>
#include <string>
#include "audio_format_converter_3DA.h"

namespace OHOS {
namespace AudioStandard {
// constexpr int MAX_CHANNEL_COUNT = 10;
static constexpr int32_t AUDIO_VIVID_SAMPLES = 1024;

#ifdef __aarch64__
    constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib64/"};
#else
    constexpr const char *LD_EFFECT_LIBRARY_PATH[] = {"/system/lib/"};
#endif

enum {
    XML_PARSE_RECOVER   = 1<<0,    /* recover on errors */
    XML_PARSE_NOERROR   = 1<<5,    /* suppress error reports */
    XML_PARSE_NOWARNING = 1<<6,    /* suppress warning reports */
    XML_PARSE_PEDANTIC  = 1<<7,
};
static constexpr int32_t MAX_META_LENGTH = 1;
// static constexpr char AUDIO_ALGO_CONFIG_FILE[] = "system/etc/audio/audio_algorithm_config.xml";


// TODO: metadataLength
static int GetMetaLength() {
    return MAX_META_LENGTH;
}
static int GetPcmLength(int32_t channels) {
    return channels * AUDIO_VIVID_SAMPLES;
}
// static int GetMaxPcmLength() {
//     return MAX_CHANNEL_COUNT * AUDIO_VIVID_SAMPLES;
// }

// TODO: implement it
int32_t AudioFormatConverter3DA::Init(const AudioStreamParams info) 
{
    inChannel_ = info.channels;
    outChannel_ = info.channels;

    // TODO: load inChannelLayout2out from xml to map
    // LoadAlgoConfigFromXML();
    if (externalLoader_.Init()) {
        externalLoader_.SetIOBufferConfig(true, info.samplingRate, inChannel_);
        externalLoader_.SetIOBufferConfig(false, info.samplingRate, outChannel_);
        loadSuccess_ = true;
    } else 
        loadSuccess_ = false;

    return 0;
}

// TODO: implement it when map is finish
void AudioFormatConverter3DA::ConvertChannels(uint8_t &channels, uint64_t &channelLayout) 
{
    
}

bool AudioFormatConverter3DA::IsReady() 
{
    return isReady_;
}

bool AudioFormatConverter3DA::AllocateMem() 
{
    inPcmBuf_ = std::make_unique<uint8_t[]>(GetPcmLength(inChannel_));
    metadataBuf_ = std::make_unique<uint8_t[]>(GetMetaLength());
    outPcmBuf_ = std::make_unique<uint8_t[]>(GetPcmLength(outChannel_));
    return inPcmBuf_ != nullptr && metadataBuf_ != nullptr && outPcmBuf_ != nullptr;
}

int32_t AudioFormatConverter3DA::GetInputBuffers(BufferDesc &inputBufDesc, BufferDesc &metaBufDesc) 
{
    inputBufDesc = {inPcmBuf_.get(), GetPcmLength(inChannel_), 0};
    metaBufDesc = {metadataBuf_.get(), GetMetaLength(), 0};
    return 0;
}

void AudioFormatConverter3DA::GetOutputBufferStream(uint8_t *&buffer, uint32_t &bufferLen) 
{
    buffer = outPcmBuf_.get();
    bufferLen = GetPcmLength(outChannel_);
}

void AudioFormatConverter3DA::SetNotReady()
{
    isReady_ = false;
    std::fill_n(inPcmBuf_.get(), GetPcmLength(inChannel_), 0);
    std::fill_n(metadataBuf_.get(), GetMetaLength(), 0);
    std::fill_n(outPcmBuf_.get(), GetPcmLength(outChannel_), 0);
}
// TODO: process
int32_t AudioFormatConverter3DA::Process(const BufferDesc pcmBuffer, const BufferDesc metaBuffer) 
{
    if (!loadSuccess_) {
        size_t n = GetPcmLength(outChannel_);
        for (size_t i = 0; i < pcmBuffer.bufLength && i < n; i++)
            outPcmBuf_[i] = pcmBuffer.buffer[i];
    } else {
        AudioBuffer inBuffer = { 
            .frameLength = pcmBuffer.bufLength, 
            .raw = pcmBuffer.buffer, 
            .metaDataLength = metaBuffer.bufLength, 
            .metaData = metaBuffer.buffer };
        AudioBuffer outBuffer = { 
            .frameLength = GetPcmLength(outChannel_), 
            .raw = outPcmBuf_.get(), 
            .metaDataLength = metaBuffer.bufLength, 
            .metaData = metaBuffer.buffer };
        externalLoader_.ApplyAlgo(inBuffer, outBuffer);
    }
    return 0;
}


static bool ClientResolveLibrary(const std::string &path, std::string &resovledPath)
{
    for (auto *libDir: LD_EFFECT_LIBRARY_PATH) {
        std::string candidatePath = std::string(libDir) + "/" + path;
        if (access(candidatePath.c_str(), R_OK) == 0) {
            resovledPath = std::move(candidatePath);
            return true;
        }
    }

    return false;
}
static bool ClientLoadLibrary(const std::string &relativePath, std::unique_ptr<AudioEffectLibEntry>& libEntry) noexcept {
    std::string absolutePath;
    // find library in adsolutePath
    if (!ClientResolveLibrary(relativePath, absolutePath)) {
        AUDIO_ERR_LOG("<log error> find library falied in effect directories: %{public}s",
            relativePath.c_str());
        return false;
    }

    void* handle = dlopen(absolutePath.c_str(), 1);
    if (!handle) {
        AUDIO_ERR_LOG("<log error> dlopen lib %{public}s Fail", relativePath.c_str());
        return false;
    } else {
        AUDIO_INFO_LOG("<log info> dlopen lib %{public}s successful", relativePath.c_str());
    }

    AudioEffectLibrary *audioEffectLibHandle = static_cast<AudioEffectLibrary *>(dlsym(handle,
        AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    const char* error = dlerror();
    if (error) {
        AUDIO_ERR_LOG("<log error> dlsym failed: error: %{public}s, %{public}p", error, audioEffectLibHandle);
        dlclose(handle);
        return false;
    } else {
        AUDIO_INFO_LOG("<log info> dlsym lib %{public}s successful, error: %{public}s", relativePath.c_str(), error);
    }
    libEntry->audioEffectLibHandle = audioEffectLibHandle;
    return true;
}

void LibLoader::SetIOBufferConfig(bool isInput, uint32_t samplingRate, uint32_t channels) 
{    
    if (isInput) {
        ioBufferConfig.inputCfg.samplingRate = samplingRate;
        ioBufferConfig.inputCfg.channels = channels;
    } else {
        ioBufferConfig.outputCfg.samplingRate = samplingRate;
        ioBufferConfig.outputCfg.channels = channels;
    }
}
// TODO: add load from xml
bool LibLoader::AddAlgoHandle() 
{
    Library library = { .name = "", .path = "" };
    AudioEffectDescriptor descriptor = { .libraryName = "", .effectName = "" };
    libEntry_ = std::make_unique<AudioEffectLibEntry>();
    libEntry_->libraryName = library.name;
    bool loadLibrarySuccess = ClientLoadLibrary(library.path, libEntry_);
    if (!loadLibrarySuccess) {
        AUDIO_ERR_LOG("<log error> loadLibrary fail, please check logs!");
        return false;
    }
    int32_t ret = libEntry_->audioEffectLibHandle->createEffect(descriptor, &handle_);
    if (ret != 0) {
        AUDIO_ERR_LOG("MCR createEffect fail");
        return false;
    }
    return ret;
}

bool LibLoader::Init() 
{
    int32_t ret = AddAlgoHandle();
    if (ret != 0) return false;
    int32_t replyData = 0;
    AudioEffectTransInfo cmdInfo = {sizeof(AudioEffectConfig), &ioBufferConfig};
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    ret = (*handle_)->command(handle_, EFFECT_CMD_INIT, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("MCR lib EFFECT_CMD_INIT fail");
        return false;
    }
    ret = (*handle_)->command(handle_, EFFECT_CMD_ENABLE, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("MCR lib EFFECT_CMD_ENABLE fail");
        return false;
    }
    ret = (*handle_)->command(handle_, EFFECT_CMD_SET_CONFIG, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("MCR lib EFFECT_CMD_SET_CONFIG fail");
        return false;
    }
    ret = (*handle_)->command(handle_, EFFECT_CMD_SET_PARAM, &cmdInfo, &replyInfo);
    if (ret != 0) {
        AUDIO_ERR_LOG("MCR lib EFFECT_CMD_SET_PARAM fail");
        return false;
    }
    return ret;
}

int32_t LibLoader::ApplyAlgo(AudioBuffer &inputBuffer, AudioBuffer &outputBuffer)
{
    int32_t ret = (*handle_)->process(handle_, &inputBuffer, &outputBuffer);
    if (ret != 0) {
        AUDIO_ERR_LOG("converter algo lib process fail");
        return ret;
    }
    return ret;
}

} // namespace AudioStandard
} // namespace OHOS