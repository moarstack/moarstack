//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_LIBRARYLOADER_H
#define MOARSTACK_LIBRARYLOADER_H

#include <moarLibInterface.h>
#include <moarLayerEntryPoint.h>

#define LIBRARY_LOAD_OK 0
#define LIBRARY_LOAD_FAILED 1
#define LIBRARY_LOAD_NONMOAR 2
#define LIBRARY_LOAD_NONLAYER 3

typedef struct{
    char* Filename;
    MoarLibInfo_T Info;
    moarLibraryInfo_F LibraryInfoFunction;
    moarLayerEntryPoint_F LayerEntryPointFunction;
    void* Handle;
} MoarLibrary_T;

int loadLibrary(char* name, MoarLibrary_T* library);
int closeLibrary(MoarLibrary_T* library);

#endif //MOARSTACK_LIBRARYLOADER_H
