/* interface struct*/

#include<OpenSLES.h>
#include<class_struct.h>

typedef struct {
    
} ClassTable;

typedef struct ObjectInterface {
    const struct SLObjectItf_ *mItf;
    CEngine *mEngine;
    const ClassTable *mClass;
    SLuint8 mState;
} IObject;

typedef struct EngineInterface {
    const struct SLEngineItf_ *mItf;
    IObject *mThis;
} IEngine;

typedef struct PlayInterface {
    const struct SLPlayItf_ *mItf;
    IObject *mThis;
    SLuint32 mState;
} IPlay;

typedef struct BufferQueueInterface {
    const struct SLBufferQueueItf_ *mItf;
    IObject *mThis;
} IBufferQueue;

typedef struct VolumeInterface {
    const struct SLVolumeItf_ *mItf;
    IObject *mThis;
} IVolume;
