#include <OpenSLES.h>
#include <class_struct.h>
#include <itf_struct.h>
#include <table_struct.h>

static const ClassTable EngineTab = {
    0,
    nullptr,
    SL_OBJECTID_ENGINE,
    sizeof(CEngine),
};

static const ClassTable AudioPlayerTab = {
    0,
    nullptr,
    SL_OBJECTID_AUDIOPLAYER,
    sizeof(CAudioPlayer),
};