#include <audioplayer_adapter.h>

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

static map<int, AudioRenderer*> renderMap;

AudioRenderer* GetAudioRenderById(int id)
{
    return renderMap.find(id)->second;
}

SLresult setPlayStateAdapter(int id, SLuint32 state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    switch (state)
        {
        case SL_PLAYSTATE_PLAYING:
            pRender->Start();
            return SL_RESULT_SUCCESS;
        case SL_PLAYSTATE_PAUSED:
            pRender->Pause();
            return SL_RESULT_SUCCESS;
            break; 
        case SL_PLAYSTATE_STOPPED:
            pRender->Stop();
            return SL_RESULT_SUCCESS;
            break;     
        default:
            return SL_RESULT_FEATURE_UNSUPPORTED;
            break;

    }
}

SLresult EnqueueAdapter(int id, SLuint32 state)
{
    AudioRenderer* pRender = GetAudioRenderById(id);
    //pRender->Write();
}

SLresult CreateAudioPlayerAdapter(int id)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    unique_ptr<AudioRenderer> ptr = AudioRenderer::Create(streamType);
    renderMap.insert(make_pair(id, ptr.get()));
}

//static audioplayerMap

//unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(streamType);