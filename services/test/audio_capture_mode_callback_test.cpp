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

#include <chrono>
#include <thread>
#include <vector>

#include "audio_capturer.h"
#include "media_log.h"
#include "pcm2wav.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

class AudioCaptureModeCallbackTest : public AudioCapturerReadCallback,
    public enable_shared_from_this<AudioCaptureModeCallbackTest> {
public:
    void OnReadData() override
    {
        MEDIA_INFO_LOG("CapturerCallbackTest: OnReadData is called");
        isRecordData_ = true;
    }

    bool InitCapture(string& filePath)
    {
        pcmFile_ = fopen(filePath.c_str(), "wb");
        if (pcmFile_ == nullptr) {
            MEDIA_INFO_LOG("CapturerCallbackTest: Unable to open file");
            return false;
        }

        AudioCapturerOptions capturerOptions;
        capturerOptions.streamInfo.samplingRate = SAMPLE_RATE_44100;
        capturerOptions.streamInfo.encoding = ENCODING_PCM;
        capturerOptions.streamInfo.format = SAMPLE_S16LE;
        capturerOptions.streamInfo.channels = STEREO;
        capturerOptions.capturerInfo.sourceType = SOURCE_TYPE_MIC;
        capturerOptions.capturerInfo.capturerFlags = 0;

        audioCapturer_ = AudioCapturer::Create(capturerOptions);
        if (audioCapturer_== nullptr) {
            MEDIA_ERR_LOG("CapturerCallbackTest: Capturer create failed");
            return false;
        }

        MEDIA_INFO_LOG("CapturerCallbackTest: Recording capturer created");
        if (audioCapturer_->SetCaptureMode(CAPTURE_MODE_CALLBACK)) {
            MEDIA_ERR_LOG("CapturerCallbackTest: SetCapturerMode failed");
            return false;
        }

        if (audioCapturer_->SetCapturerReadCallback(shared_from_this())) {
            MEDIA_ERR_LOG("CapturerCallbackTest: SetCapturerReadCallback failed");
            return false;
        }

        return true;
    }

    int32_t TestRecording(string& filePath)
    {
        MEDIA_INFO_LOG("CapturerCallbackTest: TestRecording start");
        if (!InitCapture(filePath)) {
            return -1;
        }

        if (!audioCapturer_->Start()) {
            MEDIA_ERR_LOG("CapturerCallbackTest: Start failed");
            audioCapturer_->Release();
            return -1;
        }

        recordThread_ = make_unique<thread>(&AudioCaptureModeCallbackTest::RecordData, this);
        recordThread_->join();

        audioCapturer_->Clear();
        audioCapturer_->Stop();
        audioCapturer_->Release();
        MEDIA_INFO_LOG("CapturerCallbackTest: TestPlayback end");

        return 0;
    }

    ~AudioCaptureModeCallbackTest()
    {
        MEDIA_INFO_LOG("CapturerCallbackTest: Inside ~AudioCaptureModeCallbackTest");
        if (fclose(pcmFile_)) {
            MEDIA_INFO_LOG("CapturerCallbackTest: pcmFile_ failed");
        } else {
            MEDIA_INFO_LOG("CapturerCallbackTest: fclose(pcmFile_) success");
        }
        pcmFile_ = nullptr;

        if (recordThread_ && recordThread_->joinable()) {
            recordThread_->join();
            recordThread_ = nullptr;
        }
    }

    FILE *pcmFile_ = nullptr;
private:
    void RecordData()
    {
        MEDIA_INFO_LOG("CapturerCallbackTest: RecordData thread");
        size_t numBuffersToCapture = 512;
        while (numBuffersToCapture) {
            if (isRecordData_) {
                bufDesc_.buffer = nullptr;
                audioCapturer_->GetBufferDesc(bufDesc_);
                if (bufDesc_.buffer == nullptr) {
                    MEDIA_ERR_LOG("bufDesc_.buffer == nullptr continue");
                    continue;
                }

                if (fwrite(bufDesc_.buffer, 1, bufDesc_.dataLength, pcmFile_) != bufDesc_.dataLength) {
                    MEDIA_ERR_LOG("CapturerCallbackTest: error occured in fwrite");
                }
                audioCapturer_->Enqueue(bufDesc_);
                isRecordData_ = false;
                --numBuffersToCapture;
            }
        }
    }
 
    unique_ptr<AudioCapturer> audioCapturer_ = nullptr;
    unique_ptr<thread> recordThread_ = nullptr;
    bool isRecordData_ = false;
    BufferDesc bufDesc_ {};
};

int main(int argc, char *argv[])
{
    string filePath = argv[1];
    MEDIA_INFO_LOG("CapturerCallbackTest: output file path = %{public}s", filePath.c_str());

    auto testObj = std::make_shared<AudioCaptureModeCallbackTest>();

    return testObj->TestRecording(filePath);
}
