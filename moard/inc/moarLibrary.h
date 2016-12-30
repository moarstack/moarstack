//
// Created by svalov on 7/5/16.
//

#ifndef MOARSTACK_MOARLIBRARY_H
#define MOARSTACK_MOARLIBRARY_H

#include <moarLayerEntryPoint.h>
#include <moarLibInterface.h>
#include <pthread.h>

#pragma pack(push, 1)

typedef struct{
    char* Filename;
    MoarLibInfo_T Info;
    moarLibraryInfo_F LibraryInfoFunction;
    moarLayerEntryPoint_F LayerEntryPointFunction;
    void* Handle;
    pthread_t Thread;
} MoarLibrary_T;

#pragma pack(pop)

#endif //MOARSTACK_MOARLIBRARY_H
