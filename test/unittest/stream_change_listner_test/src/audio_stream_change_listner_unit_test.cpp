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

#include "audio_stream_change_listner_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_system_manager.h"
#include "audio_log.h"
#include "hilog/log.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    AudioSystemManager *g_audioManagerInstance = nullptr;
    int g_isCallbackReceived = false;
    const string AUDIORENDER_FIRST_TEST_FILE_PATH = "/data/test_44100_1.wav";
    const string AUDIORENDER_SECOND_TEST_FILE_PATH = "/data/test_44100_2.wav";
    //const int32_t VALUE_NEGATIVE = -1;
    const int32_t VALUE_ZERO = 0;
    //const int32_t VALUE_HUNDRED = 100;
    //const int32_t VALUE_THOUSAND = 1000;
    const int32_t RENDERER_FLAG = 0;
    // Writing only 500 buffers of data for test
    const int32_t WRITE_BUFFERS_COUNT = 500;
    //constexpr int32_t PAUSE_BUFFER_POSITION = 400000;
    //constexpr int32_t PAUSE_RENDER_TIME_SECONDS = 1;

    //constexpr uint64_t BUFFER_DURATION_FIVE = 5;
    //constexpr uint64_t BUFFER_DURATION_TEN = 10;
    //constexpr uint64_t BUFFER_DURATION_FIFTEEN = 15;
    //constexpr uint64_t BUFFER_DURATION_TWENTY = 20;

    //constexpr uint32_t PLAYBACK_DURATION = 4;...uncomment me

    static size_t g_reqBufLen = 0;
    std::string g_callbackName("");
    std::mutex g_mutex;
    std::condition_variable g_condVar;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfosRcvd;
} // namespace

AudioRendererStateChangeCallbackTest::AudioRendererStateChangeCallbackTest(const std::string &testCaseName) : testCaseName_(testCaseName) {}

void AudioStreamChangeListnerUnitTest::SetUpTestCase(void)
{
    g_audioManagerInstance = AudioSystemManager::GetInstance();
    if (g_audioManagerInstance == nullptr) {
        //HiLog::Error(LABEL, "AudioSystemManager instance not available");
        return;
    }
}

void AudioStreamChangeListnerUnitTest::TearDownTestCase(void) {}
void AudioStreamChangeListnerUnitTest::SetUp(void) {}
void AudioStreamChangeListnerUnitTest::TearDown(void) {}

void AudioRenderModeCallbackTest::OnWriteData(size_t length)
{
    g_reqBufLen = length;
}

int32_t AudioStreamChangeListnerUnitTest::InitializeRenderer(unique_ptr<AudioRenderer> &audioRenderer)
{
    AudioRendererParams rendererParams;
    rendererParams.sampleFormat = SAMPLE_S16LE;
    rendererParams.sampleRate = SAMPLE_RATE_44100;
    rendererParams.channelCount = STEREO;
    rendererParams.encodingType = ENCODING_PCM;

    return audioRenderer->SetParams(rendererParams);
}

void AudioStreamChangeListnerUnitTest::InitializeRendererOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RENDERER_FLAG;
    return;
}

void AudioRendererStateChangeCallbackTest::OnRendererStateChange(
        const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    for (const auto &changeInfo : audioRendererChangeInfos) {
        audioRendererChangeInfosRcvd.push_back(std::make_unique<AudioRendererChangeInfo>(*changeInfo));
    }
    //audioRendererChangeInfosRcvd = move(audioRendererChangeInfos);
    AUDIO_DEBUG_LOG("AudioRendererStateChangeCallbackTest:  OnRendererStateChange");

    g_isCallbackReceived = true;
/*
    g_streamType = streamType;
    g_volumeLevel = volumeLevel;
    g_callbackName = testCaseName_;
    g_isUpdateUi = isUpdateUi;
    g_condVar.notify_all();
*/
}

void AudioStreamChangeListnerUnitTest::WaitForCallback()
{
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerUnitTest:  WaitForCallback enter");
    std::unique_lock<std::mutex> lock(g_mutex);
    g_condVar.wait_until(lock, std::chrono::system_clock::now() + std::chrono::minutes(1),
        []() { return g_isCallbackReceived == true; });
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerUnitTest:  WaitForCallback exit");
}

void StartRenderThread(AudioRenderer *audioRenderer, uint32_t limit, const std::string TEST_FILE_PATH)
{
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  thread in %{public}s",TEST_FILE_PATH.c_str());
    int32_t ret = -1;
    FILE *wavFile = fopen(TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    auto buffer = std::make_unique<uint8_t[]>(bufferLen);
    ASSERT_NE(nullptr, buffer);

    size_t bytesToWrite = 0;
    int32_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t numBuffersToRender = WRITE_BUFFERS_COUNT;
    auto start = chrono::system_clock::now();

    while (numBuffersToRender) {
        bytesToWrite = fread(buffer.get(), 1, bufferLen, wavFile);
        bytesWritten = 0;
        while ((static_cast<size_t>(bytesWritten) < bytesToWrite) &&
            ((static_cast<size_t>(bytesToWrite) - bytesWritten) > minBytes)) {
            bytesWritten += audioRenderer->Write(buffer.get() + static_cast<size_t>(bytesWritten),
                                                 bytesToWrite - static_cast<size_t>(bytesWritten));
            EXPECT_GE(bytesWritten, VALUE_ZERO);
            if (bytesWritten < 0) {
                break;
            }
        }
        numBuffersToRender--;

        if ((limit > 0) && (duration_cast<seconds>(system_clock::now() - start).count() > limit)) {
            break;
        }
    }

    audioRenderer->Drain();

    fclose(wavFile);
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  thread out %{public}s",TEST_FILE_PATH.c_str());
}

HWTEST(AudioStreamChangeListnerUnitTest, RendererStateChangeCallbackTest_001, TestSize.Level1)
{
    int32_t ret = -1;
    int callBackSetResult;
    std::string testCaseName("RendererStateChangeCallbackTest_001");
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    g_callbackName = testCaseName;
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  RendererStateChangeCallbackTest_001");

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(), audioRendererStateChangeCallbackTest);

    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  RendererStateChangeCallbackTest_001");
    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    if (isStarted == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamChangeListnerUnitTest::WaitForCallback();
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, audioRendererChangeInfosRcvd.size());
        EXPECT_EQ(2, audioRendererChangeInfosRcvd[0]->rendererState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  RendererStateChangeCallbackTest_001");
    
    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest:  RendererStateChangeCallbackTest_001 exit");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    //g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());

}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: GetConnectedDevicesList_001
* @tc.desc  : Test GetCurrentRendererChangeInfos interface. Returns current single active renderer stream information
*/
/*
HWTEST(AudioStreamChangeListnerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_001, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    ret = AudioSystemManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, audioRendererChangeInfos.size());
    EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}
*/
/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: GetConnectedDevicesList_002
* @tc.desc  : Test GetCurrentRendererChangeInfos interface. Returns current single active renderer stream information
*/
/*
HWTEST(AudioStreamChangeListnerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_002, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(StartRenderThread, audioRenderer.get(), PLAYBACK_DURATION, AUDIORENDER_FIRST_TEST_FILE_PATH);

    ret = AudioSystemManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, audioRendererChangeInfos.size());
    EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);

    renderThread.join();

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);


    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}
*/

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: GetCurrentRendererChangeInfos_003
* @tc.desc  : Test GetCurrentRendererChangeInfos interface. Returns current single active renderer stream information
*/
/*
HWTEST(AudioStreamChangeListnerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_003, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    ret = AudioSystemManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(1, audioRendererChangeInfos.size());
    EXPECT_EQ(3, audioRendererChangeInfos[0]->rendererState);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
}
*/
/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: GetCurrentRendererChangeInfos_003
* @tc.desc  : Test GetCurrentRendererChangeInfos interface. Returns current single active renderer stream information
*/
/*
HWTEST(AudioStreamChangeListnerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_004, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    ret = AudioSystemManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, audioRendererChangeInfos.size());
}
*/

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: GetConnectedDevicesList_002
* @tc.desc  : Test GetCurrentRendererChangeInfos interface. Returns current two active renderer stream information
*/
/*
HWTEST(AudioStreamChangeListnerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_002, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamChangeListnerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioFirstRenderer = AudioRenderer::Create(rendererOptions);
    unique_ptr<AudioRenderer> audioSecondRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioFirstRenderer);
    ASSERT_NE(nullptr, audioSecondRenderer);

    bool isStarted = audioFirstRenderer->Start();
    EXPECT_EQ(true, isStarted);
    isStarted = audioSecondRenderer->Start();
    EXPECT_EQ(true, isStarted);

    thread renderThread(StartRenderThread, audioFirstRenderer.get(), PLAYBACK_DURATION, AUDIORENDER_FIRST_TEST_FILE_PATH);
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest: first thread started");

    thread renderSecondThread(StartRenderThread, audioSecondRenderer.get(), PLAYBACK_DURATION, AUDIORENDER_SECOND_TEST_FILE_PATH);
    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest: second thread started");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ret = AudioSystemManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);;
    EXPECT_EQ(SUCCESS, ret);

    renderThread.join();
    renderSecondThread.join();

    AUDIO_DEBUG_LOG("AudioStreamChangeListnerTest: audioRendererChangeInfos Number of entries %{public}u",
        static_cast<uint32_t>(audioRendererChangeInfos.size()));
    int index = 0;
    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfo = **it;
        AUDIO_DEBUG_LOG("audioRendererChangeInfos[%{public}d]", index++);
        AUDIO_DEBUG_LOG("clientUID = %{public}d", audioRendererChangeInfo.clientUID);
        AUDIO_DEBUG_LOG("sessionId = %{public}d", audioRendererChangeInfo.sessionId);
        AUDIO_DEBUG_LOG("rendererState = %{public}d", audioRendererChangeInfo.rendererState);
    }

    bool isStopped = audioFirstRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    isStopped = audioSecondRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioFirstRenderer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioSecondRenderer->Release();
    EXPECT_EQ(true, isReleased);
}
*/
} // namespace AudioStandard
} // namespace OHOS
