#ifndef COMMON_H
#define COMMON_H

#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<table_struct.h>

struct CEngine;

// interface

struct IObject {
    const struct SLObjectItf_ *mItf;
    CEngine *mEngine; // this 指针
    const ClassTable *mClass;
    SLuint8 mState;
};

struct IEngine {
    const struct SLEngineItf_ *mItf;
    IObject *mThis; // this 指针
};

struct IPlay {
    const struct SLPlayItf_ *mItf;
    IObject *mThis; // this 指针
    SLuint32 mState;
};

struct IBufferQueue {
    const struct SLBufferQueueItf_ *mItf;
    IObject *mThis; // this 指针
};

struct IVolume {
    const struct SLVolumeItf_ *mItf;
    IObject *mThis; // this 指针
};

// class

struct CEngine {
    IObject mObject;
    IEngine mEngine;
};

struct CAudioPlayer {
    IObject mObject;
    IPlay mPlay;
    IVolume mVolume;
    IBufferQueue mBufferQueue;
    SLuint32 mId;
};

#endif