#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<stddef.h>;

typedef struct {
    SLuint32 mInterfaceCount;
    const char * const mName;
    SLuint32 mObjectId;
    size_t mSize;
} ClassTable;

static const ClassTable EngineTab;

static const ClassTable AudioPlayerTab;