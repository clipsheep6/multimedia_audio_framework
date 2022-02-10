/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>
#include <vector>

#include "audio_renderer.h"
#include "media_log.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

class AudioRendererTest : public AudioRendererWriteCallback, public enable_shared_from_this<AudioRendererTest> {
public:
    void OnWriteData(size_t length) override
    {
        MEDIA_INFO_LOG("AudioRendererTest: OnWriteData is called");
        // MEDIA_INFO_LOG("AudioRendererTest: isDataReady: %{public}d", isDataReady);
        reqBufLen_ = length;
        isEnqueue_ = true;
        /* if (isDataReady) {
            bufferQueue_->Enqueue(bufDesc_);
            isDataReady = false;
        } */
    }

    bool InitRender(const AudioRendererParams &rendererParams)
    {
        audioRenderer_->SetParams(rendererParams);

        audioRenderer_->SetRenderMode(RENDER_MODE_CALLBACK);

        audioRenderer_->SetRendererWriteCallback(shared_from_this());
        audioRenderer_->GetBufferSize(reqBufLen_);
        audioRenderer_->Start();
        return true;
    }

    int32_t TestPlayback(int argc, char *argv[])
    {
        MEDIA_INFO_LOG("AudioRendererTest: TestPlayback start");
        wav_hdr wavHeader;
        size_t headerSize = sizeof(wav_hdr);
        char *inputPath = argv[1];
        char path[PATH_MAX + 1] = {0x00};
        if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
            MEDIA_ERR_LOG("Invalid path");
            return -1;
        }

        wavFile_ = fopen(path, "rb");
        if (wavFile_ == nullptr) {
            MEDIA_INFO_LOG("AudioRendererTest: Unable to open wave file");
            return -1;
        }
        fread(&wavHeader, 1, headerSize, wavFile_);

        AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
        audioRenderer_ = AudioRenderer::Create(streamType);

        AudioRendererParams rendererParams;
        rendererParams.sampleFormat = static_cast<AudioSampleFormat>(wavHeader.bitsPerSample);
        rendererParams.sampleRate = static_cast<AudioSamplingRate>(wavHeader.SamplesPerSec);
        rendererParams.channelCount = static_cast<AudioChannel>(wavHeader.NumOfChan);
        rendererParams.encodingType = static_cast<AudioEncodingType>(ENCODING_PCM);

        InitRender(rendererParams);
        enqueueThread_ = make_unique<thread>(&AudioRendererTest::EnqueueBuffer, this);
        enqueueThread_ ->join();

        audioRenderer_->Clear();
        audioRenderer_->Stop();
        audioRenderer_->Release();
        fclose(wavFile_);
        MEDIA_INFO_LOG("AudioRendererTest: TestPlayback end");

        return 0;
    }

private:
    void EnqueueBuffer()
    {
        MEDIA_INFO_LOG("AudioRendererTest: EnqueueBuffer thread");
        while (!feof(wavFile_)) {
            // if (!isDataReady) {
            if (isEnqueue_) {
                retry_ = true;
                size_t bufLen = reqBufLen_;
                bufDesc_.buffer = nullptr;
                while (retry_) {
                    audioRenderer_->GetBufferDesc(bufDesc_);
                    if (bufDesc_.buffer != nullptr) {
                        fread(bufDesc_.buffer, 1, bufLen, wavFile_);
                        audioRenderer_->Enqueue(bufDesc_);
                        retry_ = false;
                    } else {
                        MEDIA_ERR_LOG("AudioRendererTest: bufDesc_.buffer == nullptr retry");
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
                isEnqueue_ = false;
                // isDataReady = true;
            }
        }
    }

    std::unique_ptr<AudioRenderer> audioRenderer_ = nullptr;
    FILE *wavFile_ = nullptr;
    unique_ptr<thread> enqueueThread_ = nullptr;
    // bool isDataReady = false;
    bool isEnqueue_ = true;
    BufferDesc bufDesc_ {};
    size_t reqBufLen_;
    bool retry_ = true;
};

int main(int argc, char *argv[])
{
    auto testObj = std::make_shared<AudioRendererTest>();

    return testObj->TestPlayback(argc, argv);
}
