#include<OpenSLES.h>

typedef struct {
    
} AudioPlayer;


SLresult AudioPlayerRealize(void *self, SLboolean async)
{
    // do Realize
    return 0;
}

SLresult AudioPlayerDestroy(void *self)
{
    // do Destroy
    return 0;
}

SLresult AudioPlayerGetOutputMix(AudioPlayer *audioPlayer)
{
    // create OutputMix
    return 0;
}