/* Pbject implementation */

#include<OpenSLES.h>
#include<common.h>
#include<table_struct.h>
#include<iostream>

static SLresult Realize(SLObjectItf self, SLboolean async) 
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult Resume(SLObjectItf self, SLboolean async)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetState(SLObjectItf self, SLuint32 *state) {
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetInterface(SLObjectItf self, const SLInterfaceID iid, void *interface)
{
    std::cout << "GetInterface in" << std::endl;
    // 后续下面这部分交给 mapper, IIdMapperingItf(iid, void *itf);
    
    //CEngine
    if (iid == SL_IID_ENGINE) {
        CEngine *cEngine = (CEngine *)self;
        *(void **)interface = &cEngine->mEngine.mItf;
        std::cout << "SL_IID_ENGINE" << std::endl;
    } else {
        // unsupport
    }
    
    //CAudioPlayer
    if (iid == SL_IID_PLAY) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = &cAudioPlayer->mPlay.mItf;
        std::cout << "SL_IID_PLAY" << std::endl;
    } else if (iid == SL_IID_VOLUME) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = &cAudioPlayer->mVolume.mItf;
        std::cout << "SL_IID_VOLUME" << std::endl;
    } else if (iid == SL_IID_BUFFERQUEUE) {
        CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
        *(void **)interface = &cAudioPlayer->mBufferQueue.mItf;
        std::cout << "SL_IID_BUFFERQUEUE" << std::endl;
    } else {
        std::cout << "not supported" << std::endl;
        // unsupport
    }
    std::cout << "GetInterface out" << std::endl;
    return SL_RESULT_SUCCESS;
    
    //根据 iid 把 itf 强转为指定 Cxx 类
    //从Cxx 类获取对应 itf (此itf是 Ixx 中 init 初始化的)
}

static SLresult RegisterCallback(SLObjectItf self, slObjectCallback callback, void *pContext)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static void AbortAsyncOperation(SLObjectItf self)
{
    return;
}

static SLresult SetPriority(SLObjectItf self, SLint32 priority, SLboolean preemptable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult GetPriority(SLObjectItf self, SLint32 *pPriority, SLboolean *pPreemptable)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

static SLresult SetLossOfControlInterfaces(SLObjectItf self,
    SLint16 numInterfaces, SLInterfaceID *pInterfaceIDs, SLboolean enabled)
{
    return SL_RESULT_FEATURE_UNSUPPORTED;
}

void Destroy(SLObjectItf self) {
    return;
}

static const struct SLObjectItf_ ObjectItf = {
    Realize,
    Resume,
    GetState,
    GetInterface,
    RegisterCallback,
    AbortAsyncOperation,
    Destroy,
    SetPriority,
    GetPriority,
    SetLossOfControlInterfaces
};

void IObjectInit(void *self)
{
    IObject *thiz = (IObject *) self;
    thiz->mItf = &ObjectItf;
}

void IObjectPublish(IObject *thiz)
{
    //IEngine *thisEngine = &thiz->mEngine->mEngine;
}
