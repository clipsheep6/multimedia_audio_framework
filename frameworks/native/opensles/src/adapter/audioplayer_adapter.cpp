#include <common.h>

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

void ConvertPcmFormat(SLDataFormat_PCM *slFormat, AudioRendererParams *rendererParams);

static AudioSampleFormat SlToOhosSampelFormat(SLDataFormat_PCM *pcmFormat);

static AudioSamplingRate SlToOhosSamplingRate(SLDataFormat_PCM *pcmFormat);

static AudioChannel SlToOhosChannel(SLDataFormat_PCM *pcmFormat);

static map<SLuint32, AudioRenderer*> renderMap;

class MyWriteCallback : public AudioRendererWriteCallback {
public:
    MyWriteCallback (slOHBufferQueueCallback callback, SLOHBufferQueueItf itf, void *pContext) {
        this->callback = callback;
        this->itf = itf;
        this->context = pContext;
    }

    slOHBufferQueueCallback callback;
    SLOHBufferQueueItf itf;
    void *context;
    
    void OnWriteData(size_t length) override {
        callback(itf, context, length);
    };
};

static shared_ptr<MyWriteCallback> callbackPtr;

static map<SLuint32, shared_ptr<MyWriteCallback>> callbackMap;

AudioRenderer *GetAudioRenderById(SLuint32 id)
{
    MEDIA_INFO_LOG("GetAudioRenderById: %{public}lu", id);
    return renderMap.find(id)->second;
}

void EraseAudioRenderById(SLuint32 id)
{
    MEDIA_INFO_LOG("EraseAudioRenderById: %{public}lu", id);
    renderMap.erase(id);
    callbackMap.erase(id);
    return;
}

SLresult CreateAudioPlayerAdapter(SLuint32 id, SLDataSource *dataSource, SLDataSink *dataSink, AudioStreamType streamType)
{    
    SLDataFormat_PCM *pcmFormat = (SLDataFormat_PCM *)dataSource->pFormat;
    AudioRendererParams rendererParams;
    ConvertPcmFormat(pcmFormat, &rendererParams);
    streamType = AudioStreamType::STREAM_MUSIC;
    unique_ptr<AudioRenderer> rendererHolder = AudioRenderer::Create(streamType);
    rendererHolder->SetParams(rendererParams);
    AudioRenderer *renderer = rendererHolder.release();
    MEDIA_INFO_LOG("CreateAudioPlayer ID: %{public}lu", id);
    renderer->SetRenderMode(RENDER_MODE_CALLBACK);
    renderMap.insert(make_pair(id, renderer));
    return SL_RESULT_SUCCESS;
}

//play itf

SLresult SetPlayStateAdapter(SLuint32 id, SLuint32 state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    SLresult slResult = SL_RESULT_SUCCESS;
    bool reseult = false;
    switch (state) {
        case SL_PLAYSTATE_PLAYING:
            reseult = pRender->Start();
            break;
        case SL_PLAYSTATE_PAUSED:
            reseult = pRender->Pause();
            break;
        case SL_PLAYSTATE_STOPPED: {
            reseult = pRender->Clear();
            reseult = reseult & pRender->Stop();
            reseult = reseult & pRender->Release();
            break;
        }
        default:
            MEDIA_ERR_LOG("play state not supported ");
            break;
    }
    return slResult;
}

SLresult GetPlayStateAdapter(SLuint32 id, SLuint32 *state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    RendererState rendererState = pRender->GetStatus();
    switch (rendererState) {
        case RENDERER_RUNNING:
            *state = SL_PLAYSTATE_PLAYING;
            break;    
        case RENDERER_PAUSED:
            *state = SL_PLAYSTATE_PAUSED;
            break; 
        case RENDERER_STOPPED:
            *state = SL_PLAYSTATE_STOPPED;
            break;    
        default:
            *state = -1;
            break;
    }
    return SL_RESULT_SUCCESS;
}

//volume itf

SLresult SetVolumeLevelAdapter(SLuint32 id, SLmillibel level)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    float volume = pow(level, 10) / 2000.0;
    audioRenderer->SetVolume(volume);
    return SL_RESULT_SUCCESS;
}

SLresult GetVolumeLevelAdapter(SLuint32 id, SLmillibel *level)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    float volume = audioRenderer->GetVolume();
    *level = 2000 * log10(volume);
    return SL_RESULT_SUCCESS;
}

//buffer_queue

SLresult EnqueueAdapter(SLuint32 id, const void *pBuffer, SLuint32 size)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    BufferDesc bufDesc = {};
    bufDesc.buffer = (uint8_t*) pBuffer;
    bufDesc.length = size;
    audioRenderer->Enqueue(bufDesc);
    return SL_RESULT_SUCCESS;
}

SLresult ClearAdapter(SLuint32 id)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    audioRenderer->Clear();
    return SL_RESULT_SUCCESS;
}

SLresult GetStateAdapter(SLuint32 id, SLOHBufferQueueState *pState)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    BufferQueueState state = {0, 0};
    audioRenderer->GetBufQueueState(state);
    pState->count = state.numBuffers;
    pState->playIndex = state.currentIndex;
    return SL_RESULT_SUCCESS;
}

SLresult GetBufferAdapter(SLuint32 id, SLuint8 **pBuffer, SLuint32 &pSize)
{
    AudioRenderer *audioRenderer = GetAudioRenderById(id);
    BufferDesc bufferDesc = {};
    audioRenderer->GetBufferDesc(bufferDesc);
    *pBuffer = bufferDesc.buffer;
    pSize = bufferDesc.length;
    return SL_RESULT_SUCCESS;
}

SLresult RegisterCallbackAdapter(SLOHBufferQueueItf itf, slOHBufferQueueCallback callback, void *pContext)
{
    IOHBufferQueue *thiz = (IOHBufferQueue *)itf;
    AudioRenderer *audioRenderer = GetAudioRenderById(thiz->mId);
    callbackPtr = make_shared<MyWriteCallback>(callback, itf, pContext);
    audioRenderer->SetRendererWriteCallback(static_pointer_cast<AudioRendererWriteCallback>(callbackPtr));
    callbackMap.insert(make_pair(thiz->mId, callbackPtr));
    return SL_RESULT_SUCCESS;
}

//convert

void ConvertPcmFormat(SLDataFormat_PCM *slFormat, AudioRendererParams *rendererParams)
{
    AudioSampleFormat sampleFormat = SlToOhosSampelFormat(slFormat);
    AudioSamplingRate sampleRate = SlToOhosSamplingRate(slFormat);
    AudioChannel channelCount = SlToOhosChannel(slFormat);
    rendererParams->sampleFormat = sampleFormat;
    rendererParams->sampleRate = sampleRate;
    rendererParams->channelCount = channelCount;
    rendererParams->encodingType = ENCODING_PCM;
}

static AudioSampleFormat SlToOhosSampelFormat(SLDataFormat_PCM *pcmFormat)
{
    AudioSampleFormat sampleFormat;
    switch (pcmFormat->bitsPerSample) {
        case SL_PCMSAMPLEFORMAT_FIXED_8:
            sampleFormat = SAMPLE_U8;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_16:
            sampleFormat = SAMPLE_S16LE;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_20:
            sampleFormat = INVALID_WIDTH;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_24:
            sampleFormat = SAMPLE_S24LE;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_28:
            sampleFormat = INVALID_WIDTH;
            break;
        case SL_PCMSAMPLEFORMAT_FIXED_32:
            sampleFormat = SAMPLE_S32LE;
            break;
        default:
            sampleFormat = INVALID_WIDTH;  
    }
    return sampleFormat;;
}

static AudioSamplingRate SlToOhosSamplingRate(SLDataFormat_PCM *pcmFormat)
{
    AudioSamplingRate sampleRate;
    switch (pcmFormat->samplesPerSec)
    {
        case SL_SAMPLINGRATE_8:
            sampleRate = SAMPLE_RATE_8000;
            break;
        case SL_SAMPLINGRATE_11_025:
            sampleRate = SAMPLE_RATE_11025;
            break;
        case SL_SAMPLINGRATE_12:
            sampleRate = SAMPLE_RATE_12000;
            break;
        case SL_SAMPLINGRATE_16:
            sampleRate = SAMPLE_RATE_16000;
            break;
        case SL_SAMPLINGRATE_22_05:
            sampleRate = SAMPLE_RATE_22050;
            break;
        case SL_SAMPLINGRATE_24:
            sampleRate = SAMPLE_RATE_24000;
            break;
        case SL_SAMPLINGRATE_32:
            sampleRate = SAMPLE_RATE_32000;
            break;
        case SL_SAMPLINGRATE_44_1:
            sampleRate = SAMPLE_RATE_44100;
            break;
        case SL_SAMPLINGRATE_48:
            sampleRate = SAMPLE_RATE_48000;
            break;
        case SL_SAMPLINGRATE_64:
            sampleRate = SAMPLE_RATE_64000;
            break;
        case SL_SAMPLINGRATE_88_2:
            sampleRate = SAMPLE_RATE_44100;
            break;
        case SL_SAMPLINGRATE_96:
            sampleRate = SAMPLE_RATE_96000;
            break;
        case SL_SAMPLINGRATE_192:
            sampleRate = SAMPLE_RATE_44100;
            break;
        default:
            sampleRate = SAMPLE_RATE_44100;
    }
    return sampleRate;
}

static AudioChannel SlToOhosChannel(SLDataFormat_PCM *pcmFormat)
{
    AudioChannel channelCount;
    std::cout << "numChannels: " << pcmFormat->numChannels << std::endl;
    switch (pcmFormat->numChannels)
    {
        case 1:
            channelCount = MONO;
            break;
        case 2:
            channelCount = STEREO;
            break;
        default:
            channelCount = MONO;
            break;
    }
    return channelCount;
}