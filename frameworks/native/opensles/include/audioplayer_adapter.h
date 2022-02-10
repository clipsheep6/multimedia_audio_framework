#ifndef AUDIOPLAYER_ADAPTER_H
#define AUDIOPLAYER_ADAPTER_H

#include <OpenSLES.h>
#include <OpenSLES_Platform.h>
#include <iostream>
#include <map>
#include <audio_renderer.h>

OHOS::AudioStandard::AudioRenderer *GetAudioRenderById(SLuint32 id);

void EraseAudioRenderById(SLuint32 id);

SLresult CreateAudioPlayerAdapter(SLuint32 id, SLDataSource *dataSource, SLDataSink *dataSink, OHOS::AudioStandard::AudioStreamType streamType);

SLresult SetPlayStateAdapter(SLuint32 id, SLuint32 state);

SLresult GetPlayStateAdapter(SLuint32 id, SLuint32 *state);

SLresult SetVolumeLevelAdapter(SLuint32 id, SLmillibel level);

SLresult GetVolumeLevelAdapter(SLuint32 id, SLmillibel *level);

SLresult EnqueueAdapter(SLuint32 id, const void *pBuffer, SLuint32 size);

SLresult ClearAdapter(SLuint32 id);

SLresult GetStateAdapter(SLuint32 id, SLOHBufferQueueState *pState);

SLresult GetBufferAdapter(SLuint32 id, SLuint8 **pBuffer, SLuint32 &pSize);

SLresult RegisterCallbackAdapter(SLOHBufferQueueItf itf, slOHBufferQueueCallback callback, void *pContext);
#endif