//
// Created by svalov on 7/2/16.
//
#define _GNU_SOURCE
#include <libraryLoader.h>
#include <dlfcn.h>
#include <stddef.h>

//#define LM_ID LM_ID_NEWLM //use in release
#define LM_ID LM_ID_BASE //use in debug, not support multiple loads of single library, soft/hard links does`t work

int loadLibrary(char* name, MoarLibrary_T* library){
    if(NULL == name || NULL == library)
        return LIBRARY_LOAD_FAILED;
    //fill in
    library->Filename = name;
    //use dlmopen instead dlopen to load libraries to different namespaces
    //that allow to load multiple independent copies of single library
    library->Handle = dlmopen(LM_ID, name, RTLD_LAZY | RTLD_LOCAL);
    if(NULL == library->Handle)
        return LIBRARY_LOAD_FAILED;

    //search library info function
    library->LibraryInfoFunction = (moarLibraryInfo_F)dlsym(library->Handle, MOAR_LIBRARY_INFO_NAME);
    if(NULL == library->LibraryInfoFunction)
        return LIBRARY_LOAD_NONMOAR;

    //get library info
    int res = library->LibraryInfoFunction(&(library->Info));
    if(MOAR_LIBRARY_INFO_OK != res)
        return LIBRARY_LOAD_NONMOAR;

    //search entry point function
    library->LayerEntryPointFunction = (moarLayerEntryPoint_F)dlsym(library->Handle, MOAR_LAYER_ENTRY_POINT_NAME);
    if(NULL == library->LayerEntryPointFunction)
        return LIBRARY_LOAD_NONLAYER;
    return LIBRARY_LOAD_OK;
}

int closeLibrary(MoarLibrary_T* library){
    //check for correct input values
    if(NULL == library)
        return LIBRARY_CLOSE_FAILED;
    if(NULL == library->Handle)
        return LIBRARY_CLOSE_FAILED;
    //close handle
    int res = dlclose(library->Handle);
    if(res)
        return LIBRARY_CLOSE_FAILED;
    //fill pointers with null
    library->Handle = NULL;
    library->LayerEntryPointFunction = NULL;
    library->LibraryInfoFunction = NULL;
    return LIBRARY_CLOSE_OK;
}