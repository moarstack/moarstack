//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <stdio.h>
#include <moarLibInterface.h>

int main(int argc, char** argv){
    MoarLibrary_T lib;
    int res = loadLibrary("../layers/interface/libmoar_interface.so",&lib);
    if(!res)
        printf("%s loaded\n",lib.Info.LibraryName);
    else
        printf("load failed\n");
    res = closeLibrary(&lib);
    if(!res)
        printf("library closed\n");
    else
        printf("close failed\n");
    return 0;
}