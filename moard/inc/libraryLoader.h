//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_LIBRARYLOADER_H
#define MOARSTACK_LIBRARYLOADER_H

#include <moarLibrary.h>

#define LIBRARY_LOAD_OK 0
#define LIBRARY_LOAD_FAILED 1
#define LIBRARY_LOAD_NONMOAR 2
#define LIBRARY_LOAD_NONLAYER 3
#define LIBRARY_CLOSE_OK 0
#define LIBRARY_CLOSE_FAILED 1

int loadLibrary(char* name, MoarLibrary_T* library);
int closeLibrary(MoarLibrary_T* library);

#endif //MOARSTACK_LIBRARYLOADER_H
