#ifndef TABLE_STRUCT_H
#define TABLE_STRUCT_H

#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<stddef.h>

struct ClassTable {
    SLuint32 mInterfaceCount;
    char * mName;
    SLuint32 mObjectId;
    size_t mSize;
};

extern ClassTable EngineTab;

extern ClassTable AudioPlayerTab;

#endif