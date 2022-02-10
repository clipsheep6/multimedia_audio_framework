#include<common.h>

SLresult AudioPlayerDestroy(void *self)
{
    CAudioPlayer *cAudioPlayer = (CAudioPlayer *)self;
    EraseAudioRenderById(cAudioPlayer->mId);
    return SL_RESULT_SUCCESS;
}