//
// Created by svalov on 7/5/16.
//

#ifndef MOARSTACK_THREADMANAGER_H
#define MOARSTACK_THREADMANAGER_H

#include <moarLayerEntryPoint.h>
#include <moarLibInterface.h>
#include <libraryLoader.h>

int createThread(MoarLibrary_T *lib, void *arg);
int exitThread(MoarLibrary_T *lib);

#endif //MOARSTACK_THREADMANAGER_H
