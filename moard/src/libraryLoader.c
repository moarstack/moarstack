//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <dlfcn.h>

int loadLibrary(char* name, MoarLibrary_T* library){
    library->Filename = name;
    //load library
    //search library info function
    //search entry point function
    return LIBRARY_LOAD_FAILED;
}

int closeLibrary(MoarLibrary_T* library){
    //close handle
    //fill pointers with null
    return 0;
}