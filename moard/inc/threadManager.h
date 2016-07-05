//
// Created by svalov on 7/5/16.
//

#ifndef MOARSTACK_THREADMANAGER_H
#define MOARSTACK_THREADMANAGER_H

#include <moarLayerEntryPoint.h>
#include <moarLibInterface.h>
#include <libraryLoader.h>

#define THREAD_CREATE_OK 0
#define THREAD_CREATE_FAILED 1

#define THREAD_EXIT_OK 0
#define THREAD_EXIT_FAILED 1

int createThread(MoarLibrary_T *lib, void *arg);
int exitThread(MoarLibrary_T *lib);

#endif //MOARSTACK_THREADMANAGER_H
