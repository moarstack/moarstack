//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_LIBRARYLOADER_H
#define MOARSTACK_LIBRARYLOADER_H

#include <moarLibrary.h>
#include <funcResults.h>

#define FUNC_RESULT_FAILED_NONMOAR 2
#define FUNC_RESULT_FAILED_NONLAYER 3
#define FUNC_RESULT_FAILED_NOTCOMPATIBLE 4

int loadLibrary(char* name, MoarLibrary_T* library);
int closeLibrary(MoarLibrary_T* library);

#endif //MOARSTACK_LIBRARYLOADER_H
