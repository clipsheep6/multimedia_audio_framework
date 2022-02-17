#include<common.h>

SLresult OutputMixDestroy(void *self)
{
    free(self);
    return SL_RESULT_SUCCESS;
}