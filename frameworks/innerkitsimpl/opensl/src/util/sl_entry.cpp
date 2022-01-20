#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<class_struct.h>
#include<itf_struct.h>
#include<table_struct.h>
#include<builder.h>
#include<all_itf.h>

SLresult SLAPIENTRY slCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
    const SLEngineOption *pEngineOptions, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    ClassTable *engineClass = ObjectIdToClass(SL_OBJECTID_ENGINE);
    CEngine *thiz = (CEngine *) Construct(engineClass, nullptr);
    
    //后续Ixx 的 初始化交给各Ixx 的 realize
    IObjectInit(&thiz->mObject);
    IEngineInit(&thiz->mEngine);

    //获得itf指针
    *pEngine = &thiz->mObject.mItf;
    return SL_RESULT_SUCCESS;
} 
  
SLresult SLAPIENTRY slQueryNumSupportedEngineInterfaces(SLuint32 *pNumSupportedInterfaces)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
  
SLresult SLAPIENTRY slQuerySupportedEngineInterfaces(SLuint32 index, SLInterfaceID *pInterfaceId)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}
  