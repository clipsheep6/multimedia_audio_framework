/**
* @tc.name  : Test SetAudioRendererDesc API via legal input
* @tc.number: Audio_Renderer_SetAudioRendererDesc_001
* @tc.desc  : Test SetAudioRendererDesc interface. Returns 0 {SUCCESS}, if the setting is successful.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetAudioRendererDesc_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererDesc rendererDesc;
    rendererDesc.contentType = ContentType.CONTENT_TYPE_SPEECH;
    rendererDesc.streamUsage = StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION; 
    int32_t ret = audioRenderer->SetAudioRendererDesc(rendererDesc);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetStreamType API via legal input
* @tc.number: Audio_Renderer_SetStreamType_001
* @tc.desc  : Test SetStreamType interface. Returns 0 {SUCCESS}, if the setting is successful.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetStreamType_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);

    AudioRendererDesc rendererDesc;
    rendererDesc.contentType = ContentType.CONTENT_TYPE_SPEECH;
    rendererDesc.streamUsage = StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION; 
    AudioStreamType audioStreamType = AudioRenderer->GetStreamType(rendererDesc.contentType, rendererDesc.streamUsage);
    int32_t ret = audioRenderer->SetStreamType(audioStreamType);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetRenderRate API via legal input
* @tc.number: Audio_Renderer_SetRenderRate_001
* @tc.desc  : Test SetRenderRate interface. Returns 0 {SUCCESS}, if the setting is successful.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetRenderRate_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = audioRenderer->SetRenderRate(1);
    EXPECT_EQ(SUCCESS, ret);
    int32_t ret_get = audioRenderer->GetRenderRate();
    EXPECT_NE(0, ret_get);
    audioRenderer->Release();
}

/**
* @tc.name  : Test SetInterruptMode API via legal input
* @tc.number: Audio_Renderer_SetInterruptMode_001
* @tc.desc  : Test SetInterruptMode interface. Returns 0 {SUCCESS}, if the setting is successful.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetInterruptMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = audioRenderer->SetInterruptMode(0);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}
void StartRenderThread(AudioRenderer *audioRenderer, uint32_t limit)
{
    int32_t ret = -1;
    FILE *wavFile = fopen(AUDIORENDER_TEST_FILE_PATH.c_str(), "rb");
    ASSERT_NE(nullptr, wavFile);

    size_t bufferLen;
    ret = audioRenderer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    int32_t streamId = -1;
    streamId = audioRenderer->GetAudioStreamId(streamId);
    EXPECT_NE(-1, streamId);
void StartCaptureThread(AudioCapturer *audioCapturer, const string filePath)
{
    int32_t ret = -1;
    bool isBlockingRead = true;
    size_t bufferLen;
    ret = audioCapturer->GetBufferSize(bufferLen);
    EXPECT_EQ(SUCCESS, ret);

    int32_t streamId = -1;
    streamId = audioCapturer->GetAudioStreamId(streamId);
    EXPECT_NE(-1, streamId);


}

/**
* @tc.name  : Test GetCaptureMode, CAPTURER_MODE_CALLBACK
* @tc.number: Audio_CAPTURER_GetCaptureMode_001
* @tc.desc  : Test GetCaptureMode interface. Returns CAPTURE_MODE_CALLBACK, if obtained successfully.
*/
HWTEST(AudioCapturerUnitTest, Audio_CAPTURER_GetCaptureMode_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;

    AudioCapturerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = audioCapturer->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);
    AudioCaptureMode capturerMode = audioCapturer->GetCaptureMode();
    EXPECT_EQ(CAPTURE_MODE_CALLBACK, capturerMode);
    audioCapturer->Release();
}

/**
* @tc.name  : Test GetBufQueueState 
* @tc.number: Audio_Renderer_GetBufQueueState_001
* @tc.desc  : Test GetBufQueueState interface. Returns BufferQueueState, if obtained successfully.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_GetBufQueueState_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;

    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    EXPECT_EQ(SUCCESS, ret);
    AudioRenderMode renderMode = audioRenderer->GetRenderMode();
    EXPECT_EQ(RENDER_MODE_CALLBACK, renderMode);

    shared_ptr<AudioRendererWriteCallback> cb = make_shared<AudioRenderModeCallbackTest>();

    ret = audioRenderer->SetRendererWriteCallback(cb);
    EXPECT_EQ(SUCCESS, ret);

    BufferQueueState bQueueSate {};
    bQueueSate.currentIndex = 1;
    bQueueSate.numBuffers = 1;

    ret = audioRenderer->GetBufQueueState(bQueueSate);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}


/**
* @tc.name  : Test SetInterruptMode API via legal input
* @tc.number: Audio_Renderer_SetInterruptMode_001
* @tc.desc  : Test SetInterruptMode interface. Returns 0 {SUCCESS}, if the setting is successful.
*/
HWTEST(AudioRendererUnitTest, Audio_Renderer_SetInterruptMode_001, TestSize.Level1)
{
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(STREAM_MUSIC);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = audioRenderer->SetInterruptMode(0);
    EXPECT_EQ(SUCCESS, ret);
    audioRenderer->Release();
}