#include <common.h>

ClassTable EngineTab = {
    0,
    nullptr,
    SL_OBJECTID_ENGINE,
    sizeof(CEngine),
};

ClassTable AudioPlayerTab = {
    0,
    nullptr,
    SL_OBJECTID_AUDIOPLAYER,
    sizeof(CAudioPlayer),
};

ClassTable OutputMixTab = {
    0,
    nullptr,
    SL_OBJECTID_OUTPUTMIX,
    sizeof(CAudioPlayer),
};