/* Pbject implementation */

#include<OpenSLES.h>
#include<itf_struct.h>

static SLresult Realize(SLObjectItf self, SLboolean async) {
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
    IObject *thiz = (IObject *) self;
    const ClassTable *clazz = thiz->mClass;
    // *interface = iip -> xxx;
    return SL_RESULT_FEATURE_UNSUPPORTED;
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
