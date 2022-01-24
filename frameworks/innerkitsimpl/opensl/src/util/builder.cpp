/* Builder implementation */

#include <OpenSLES.h>
#include<common.h>
#include <stdlib.h>
#include <table_struct.h>

ClassTable *ObjectIdToClass(SLuint32 objectId) {
    ClassTable *classTable = nullptr;
    if (objectId == SL_OBJECTID_ENGINE) {
        classTable = (ClassTable *) &EngineTab;
    } else if (objectId == SL_OBJECTID_AUDIOPLAYER) {
        classTable = (ClassTable *) &AudioPlayerTab;
    } else if (objectId == SL_OBJECTID_OUTPUTMIX) {
        classTable = (ClassTable *) &OutputMixTab;
    }
    return classTable;
}

IObject *Construct(const ClassTable *classTable, SLEngineItf engine) {
    IObject *thiz = (IObject *) calloc(1, classTable->mSize);                                                                      
    if (thiz != nullptr) {
        IEngine *thisEngine = (IEngine *) engine;
        if (thisEngine != nullptr) {
            thiz->mEngine = (CEngine *) thisEngine->mThis;
        } else {
            thiz->mEngine = (CEngine *) thiz;
        }
        thiz->mClass = classTable;
    }
    return thiz;
}


