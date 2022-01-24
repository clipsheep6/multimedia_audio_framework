#ifndef AUDIOPLAYER_ADAPTER_H
#define AUDIOPLAYER_ADAPTER_H

#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<iostream>
#include "audio_renderer.h"

AudioRenderer *GetAudioRenderById(int id);

#endif