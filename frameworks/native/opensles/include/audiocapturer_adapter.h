#ifndef AUDIOCAPTURER_ADAPTER_H
#define AUDIOCAPTURER_ADAPTER_H

#include <OpenSLES.h>
#include <OpenSLES_Platform.h>
#include <iostream>
#include <map>
#include <audio_capturer.h>
#include <audio_system_manager.h>
#include <writecallback_adapter.h>

namespace OHOS {
namespace AudioStandard {
class AudioCapturerAdapter {
public:
    static AudioCapturerAdapter* GetInstance();
    AudioCapturer *GetAudioCapturerById(SLuint32 id);
    void EraseAudioCapturerById(SLuint32 id);
    SLresult CreateAudioCapturerAdapter
        (SLuint32 id, SLDataSource *dataSource, SLDataSink *dataSink, AudioStreamType streamType);
    SLresult SetCaptureStateAdapter(SLuint32 id, SLuint32 state);
    SLresult GetCaptureStateAdapter(SLuint32 id, SLuint32 *state);
   // SLresult SetVolumeLevelAdapter(SLuint32 id, SLmillibel level);
   // SLresult GetVolumeLevelAdapter(SLuint32 id, SLmillibel *level);
   // SLresult GetMaxVolumeLevelAdapter(SLuint32 id, SLmillibel *level);
    SLresult EnqueueAdapter(SLuint32 id, const void *buffer, SLuint32 size);
    SLresult ClearAdapter(SLuint32 id);
    SLresult GetStateAdapter(SLuint32 id, SLOHBufferQueueState *state);
    SLresult GetBufferAdapter(SLuint32 id, SLuint8 **buffer, SLuint32 &size);
    SLresult RegisterCallbackAdapter(SLOHBufferQueueItf itf, SlOHBufferQueueCallback callback, void *pContext);
    
private:
    AudioCapturerAdapter();
    ~AudioCapturerAdapter();
    const float MAGNIFICATION = 2000;
    std::map<SLuint32, AudioCapturer*> captureMap_;
    std::shared_ptr<ReadOrWriteCallbackAdapter> callbackPtr_;
    std::map<SLuint32, std::shared_ptr<ReadOrWriteCallbackAdapter>> callbackMap_;

    void ConvertPcmFormat(SLDataFormat_PCM *slFormat, AudioCapturerParams *capturerParams);
    AudioSampleFormat SlToOhosSampelFormat(SLDataFormat_PCM *pcmFormat);
    AudioSamplingRate SlToOhosSamplingRate(SLDataFormat_PCM *pcmFormat);
    AudioChannel SlToOhosChannel(SLDataFormat_PCM *pcmFormat);
};
}
}
#endif