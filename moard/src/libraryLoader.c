//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <dlfcn.h>
#include <stddef.h>

int loadLibrary(char* name, MoarLibrary_T* library){
    if(NULL == name || NULL == library)
        return LIBRARY_LOAD_FAILED;
    //fill in
    library->Filename = name;
    library->Handle = dlopen(name, RTLD_LAZY);
    if(NULL == library->Handle)
        return LIBRARY_LOAD_FAILED;

    //search library info function
    library->LibraryInfoFunction = (moarLibraryInfo_F)dlsym(library->Handle, MOAR_LIBRARY_INFO_NAME);
    if(NULL == library->LibraryInfoFunction)
        return LIBRARY_LOAD_NONMOAR;

    //get library info
    if(library->LibraryInfoFunction(&(library->Info)))
        return LIBRARY_LOAD_NONMOAR;

    //search entry point function
    library->LayerEntryPointFunction = (moarLayerEntryPoint_F)dlsym(library->Handle, MOAR_LAYER_ENTRY_POINT_NAME);
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