#include<common.h>

SLresult EngineDestory(void* self)
{
    free(self);
    return SL_RESULT_SUCCESS;
}