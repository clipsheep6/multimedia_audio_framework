#include<OpenSLES.h>

typedef struct EngineClass {
    IObject mObject;
} CEngine;

typedef struct AudioPlayerClass {
    IObject mObject;
} CAudioPlayer;

ClassTable *ObjectIdToClass(SLuint32 objectId);

IObject *Construct(ClassTable classTable, SLEngineItf itf);