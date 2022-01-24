/* Engine implementation */

#include <OpenSLES.h>
#include <common.h>
#include <table_struct.h>
#include <builder.h>
#include <all_itf.h>
#include<iostream>

static SLresult CreateLEDDevice(SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateVibraDevice(SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateAudioPlayer(SLEngineItf self, SLObjectItf *pPlayer,
    SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    std::cout << "CreateAudioPlayer in" << std::endl;
    // objectid -> objectclass;使用 objectid 映射 拿到对应 类对象
    ClassTable *audioPlayerClass = ObjectIdToClass(SL_OBJECTID_AUDIOPLAYER);
    // 使用 类对象 构造 对象
    CAudioPlayer *thiz = (CAudioPlayer *) Construct(audioPlayerClass, self);
    
    //后续Ixx 的 初始化交给各Ixx 的 realize
    IObjectInit(&thiz->mObject);
    IPlayInit(&thiz->mPlay);
    IVolumeInit(&thiz->mVolume);
    IBufferQueueInit(&thiz->mBufferQueue);
    
    // 用于标识 class，和 adapter 一一对应
    thiz->mId = 0;
    // 拿到 接口
    *pPlayer = &thiz->mObject.mItf;
    // adapter侧同步创建
    //CreateAudioPlayerAdapter(a, b, c)?
    // Not implemented
    std::cout << "CreateAudioPlayer out" << std::endl;
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
 static SLresult CreateAudioRecorder(SLEngineItf self, SLObjectItf *pRecorder,
    SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
 static SLresult CreateMidiPlayer(SLEngineItf self, SLObjectItf *pPlayer,
    SLDataSource *pMIDISrc, SLDataSource *pBankSrc, SLDataSink *pAudioOutput,
    SLDataSink *pVibra, SLDataSink *pLEDArray, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateListener(SLEngineItf self, SLObjectItf *pListener,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult Create3DGroup(SLEngineItf self, SLObjectItf *pGroup, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateOutputMix(SLEngineItf self, SLObjectItf *pMix, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    std::cout << "CreateOutputMix in" << std::endl;
    // objectid -> objectclass;使用 objectid 映射 拿到对应 类对象
    ClassTable *outPutClass = ObjectIdToClass(SL_OBJECTID_OUTPUTMIX);
    // 使用 类对象 构造 对象
    COutputMix *thiz = (COutputMix *) Construct(outPutClass, self);
    IObjectInit(&thiz->mObject);
    // 拿到 接口
    *pMix = &thiz->mObject.mItf;

    std::cout << "CreateOutputMix out" << std::endl;
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateMetadataExtractor(SLEngineItf self, SLObjectItf *pMetadataExtractor,
    SLDataSource *pDataSource, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult CreateExtensionObject(SLEngineItf self, SLObjectItf *pObject,
    void *pParameters, SLuint32 objectID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult QueryNumSupportedInterfaces(SLEngineItf self,
    SLuint32 objectID, SLuint32 *pNumSupportedInterfaces)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult QuerySupportedInterfaces(SLEngineItf self,
    SLuint32 objectID, SLuint32 index, SLInterfaceID *pInterfaceId)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
};
 
 
static SLresult QueryNumSupportedExtensions(SLEngineItf self, SLuint32 *pNumExtensions)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult QuerySupportedExtension(SLEngineItf self,
    SLuint32 index, SLchar *pExtensionName, SLint16 *pNameLength)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
 
 
static SLresult IsExtensionSupported(SLEngineItf self,
    const SLchar *pExtensionName, SLboolean *pSupported)
{
    // Not implemented
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static const struct SLEngineItf_ EngineItf = {
    CreateLEDDevice,
    CreateVibraDevice,
    CreateAudioPlayer,
    CreateAudioRecorder,
    CreateMidiPlayer,
    CreateListener,
    Create3DGroup,
    CreateOutputMix,
    CreateMetadataExtractor,
    CreateExtensionObject,
    QueryNumSupportedInterfaces,
    QuerySupportedInterfaces,
    QueryNumSupportedExtensions,
    QuerySupportedExtension,
    IsExtensionSupported
};

void IEngineInit(void *self)
{
    IEngine *thiz = (IEngine *) self;
    thiz->mItf = &EngineItf;
}