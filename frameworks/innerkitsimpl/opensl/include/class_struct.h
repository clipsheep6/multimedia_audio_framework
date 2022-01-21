/*
#ifndef CLASS_STRUCT_H
#define CLASS_STRUCT_H

#include<OpenSLES.h>
#include<OpenSLES_Platform.h>
#include<stddef.h>

struct CEngine {
    IObject mObject;
    IEngine mEngine;
};

struct CAudioPlayer {
    IObject mObject;
    IPlay mPlay;
    IVolume mVolume;
    IBufferQueue mBufferQueue;
};

//table -> class -> itf

//table 对于 类对象，用于construct()构造class

//class 被construct()构造，持有ObjectItf

// xx->mObject->mItf

//itf 通过ObjectItf的getInterface方法获取，对外提供函数调用，对内调用adapter

#endif
*/