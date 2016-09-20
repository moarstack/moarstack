//
// Created by svalov on 7/2/16.
//
#define _GNU_SOURCE
#include <libraryLoader.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>

#define LM_ID LM_ID_BASE //use in debug, not support multiple loads of single library, soft/hard links does`t work

#ifdef USE_NEWLM
    #define LM_ID LM_ID_NEWLM //use in release
#endif


int loadLibrary(char* name, MoarLibrary_T* library){
    if(NULL == name || NULL == library)
        return FUNC_RESULT_FAILED_ARGUMENT;
    //fill in
    memset(library,0,sizeof(MoarLibrary_T));
    library->Filename = name;
    //use dlmopen instead dlopen to load libraries to different namespaces
    //that allow to load multiple independent copies of single library
    library->Handle = dlmopen(LM_ID, name, RTLD_LAZY | RTLD_LOCAL);
    if(NULL == library->Handle)
        return FUNC_RESULT_FAILED;

    //search library info function
    library->LibraryInfoFunction = (moarLibraryInfo_F)dlsym(library->Handle, MOAR_LIBRARY_INFO_NAME);
    if(NULL == library->LibraryInfoFunction)
        return FUNC_RESULT_FAILED_NONMOAR;

    //get library info
    int res = library->LibraryInfoFunction(&(library->Info));
    if(MOAR_LIBRARY_INFO_OK != res)
        return FUNC_RESULT_FAILED_NONMOAR;

    //search entry point function
    library->LayerEntryPointFunction = (moarLayerEntryPoint_F)dlsym(library->Handle, MOAR_LAYER_ENTRY_POINT_NAME);
    if(NULL == library->LayerEntryPointFunction)
        return FUNC_RESULT_FAILED_NONLAYER;
    return FUNC_RESULT_SUCCESS;
}

int closeLibrary(MoarLibrary_T* library){
    //check for correct input values
    if(NULL == library)
        return FUNC_RESULT_FAILED_ARGUMENT;
    if(NULL == library->Handle)
        return FUNC_RESULT_FAILED_ARGUMENT;
    //close handle
    int res = dlclose(library->Handle);
    if(res)
        return FUNC_RESULT_FAILED;
    //fill pointers with null
    library->Handle = NULL;
    library->LayerEntryPointFunction = NULL;
    library->LibraryInfoFunction = NULL;
    return FUNC_RESULT_SUCCESS;
}