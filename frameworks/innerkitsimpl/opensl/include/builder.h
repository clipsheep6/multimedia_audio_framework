#include<OpenSLES.h>

ClassTable *ObjectIdToClass(SLuint32 objectId);

IObject *Construct(const ClassTable *classTable, SLEngineItf itf);