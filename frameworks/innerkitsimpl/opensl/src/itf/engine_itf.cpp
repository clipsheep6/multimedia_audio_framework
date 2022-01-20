/* Engine implementation */

#include <OpenSLES.h>
#include <class_struct.h>
#include <itf_struct.h>
#include <table_struct.h>
#include <builder.h>
#include <all_itf.h>

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
    // objectid -> objectclass;使用 objectid 映射 拿到对应 类对象
    ClassTable *audioPlayerClass = ObjectIdToClass(SL_OBJECTID_AUDIOPLAYER);
    // 使用 类对象 构造 对象
    CAudioPlayer *thiz = (CAudioPlayer *) Construct(audioPlayerClass, self);
    
    //后续Ixx 的 初始化交给各Ixx 的 realize
    IObjectInit(&thiz->mObject);
    IPlayInit(&thiz->mPlay);
    IVolumeInit(&thiz->mVolume);
    IBufferQueueInit(&thiz->mBufferQueue);
    
    // 拿到 接口
    *pPlayer = &thiz->mObject.mItf;
    // adapter侧同步创建
    //CreateAudioPlayerAdapter(a, b, c)?
    // Not implemented
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
    // Not implemented
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