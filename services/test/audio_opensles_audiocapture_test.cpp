#include <OpenSLES.h>
#include <OpenSLES_OpenHarmony.h>
#include <OpenSLES_Platform.h>
#include "audio_info.h"
#include "media_log.h"
#include <iostream>
#include "pcm2wav.h"
#include <unistd.h>

using namespace std;

static void BuqqerQueueCallback (SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size);

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile);

static void CaptureStop(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf);

static void OpenSLCaptureTest();

FILE *wavFile_ = nullptr;
SLObjectItf engineObject = nullptr;
SLRecordItf  recordItf;
SLOHBufferQueueItf bufferQueueItf;
SLObjectItf pcmCapturerObject = nullptr;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        return -1;
    }
    
    char *outputPath = argv[1];
    char path[PATH_MAX + 1] = {0x00};
    if ((strlen(outputPath) > PATH_MAX) || (realpath(outputPath, path) == nullptr)) {
        MEDIA_ERR_LOG("Invalid path.");
        return -1;
    }
    wavFile_ = fopen(path, "wb+");
    if (wavFile_ == nullptr) {
        MEDIA_ERR_LOG("AudioCaptureTest: create record file failed.");
        return -1;
    }
    
    OpenSLCaptureTest();
    fflush(wavFile_);
    CaptureStop(recordItf, bufferQueueItf);
    (*pcmCapturerObject)->Destroy(pcmCapturerObject);
    fclose(wavFile_);

}

static void OpenSLCaptureTest()
{
    MEDIA_ERR_LOG("OpenSLCaptureTest");
    engineObject = nullptr;
    SLEngineItf engineItf = nullptr;
    SLresult result;
    result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);

    //设置IO设备（麦克风）
    SLDataLocator_IODevice io_device = {
            SL_DATALOCATOR_IODEVICE,        
            SL_IODEVICE_AUDIOINPUT,         
            SL_DEFAULTDEVICEID_AUDIOINPUT,   
            NULL                             
    };
    SLDataSource audioSource = {
            &io_device,                      
            NULL                            
    };

    //设置输出buffer队列
    SLDataLocator_BufferQueue buffer_queue = {
            SL_DATALOCATOR_BUFFERQUEUE,    
            3                              
    };
    //设置输出数据的格式
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,                             
            OHOS::AudioStandard::AudioChannel::MONO,                            
            OHOS::AudioStandard::AudioSamplingRate::SAMPLE_RATE_44100,          
            OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE,               
            0,                 
            0,
            0
    };
    SLDataSink audioSink = {
            &buffer_queue,                   //SLDataFormat_PCM配置输出
            &format_pcm                      //输出数据格式
    };

    result = (*engineItf)->CreateAudioPlayer(
        engineItf, &pcmCapturerObject, &audioSource, &audioSink, 3, nullptr, nullptr);
    (*pcmCapturerObject)->Realize(pcmCapturerObject, SL_BOOLEAN_FALSE);
    
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_RECORD, &recordItf);
    (*pcmCapturerObject)->GetInterface(pcmCapturerObject, SL_IID_OH_BUFFERQUEUE, &bufferQueueItf);
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BuqqerQueueCallback, wavFile_);
    
    CaptureStart(recordItf, bufferQueueItf, wavFile_);

    return;
}

static void BuqqerQueueCallback(SLOHBufferQueueItf bufferQueueItf, void *pContext, SLuint32 size)
{
    FILE *wavFile = (FILE *)pContext;
    if (wavFile != nullptr) {
        SLuint8 *buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, size);
        fwrite(buffer, 1, pSize, wavFile);
    }
    return;
}

static void CaptureStart(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf, FILE *wavFile)
{
    MEDIA_INFO_LOG("RecordStart");
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);
    if (wavFile != nullptr) {
        SLuint8* buffer = nullptr;
        SLuint32 pSize = 0;
        (*bufferQueueItf)->GetBuffer(bufferQueueItf, &buffer, pSize);
        (*bufferQueueItf)->Enqueue(bufferQueueItf, buffer, pSize);
        fwrite(buffer, 1, pSize, wavFile);
    }
    return;
}

static void CaptureStop(SLRecordItf recordItf, SLOHBufferQueueItf bufferQueueItf)
{
    MEDIA_INFO_LOG("CaptureStop");
    (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
    return;
}