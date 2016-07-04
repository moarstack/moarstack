//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <stdio.h>
#include <configFiles.h>
#include <moarLibInterface.h>

MoarLibrary_T libraries[MoarLayer_LayersCount];
char *fileNames[MoarLayer_LayersCount];

int main(int argc, char** argv)
{
    fileNames[MoarLayer_Interface] = LIBRARY_PATH_INTERACE;
    fileNames[MoarLayer_Channel] = LIBRARY_PATH_CHANNEL;
    fileNames[MoarLayer_Routing] = LIBRARY_PATH_ROUTING;
    fileNames[MoarLayer_Presentation] = LIBRARY_PATH_PRESENTATION;
    fileNames[MoarLayer_Service] = LIBRARY_PATH_SERVICE;

    for(int i=0; i<MoarLayer_LayersCount;i++) {
        int res = loadLibrary(fileNames[i], &(libraries[i]));
        if (!res)
            printf("%s from %s loaded\n", libraries[i].Info.LibraryName, libraries[i].Info.Author);
        else
            printf("%s load failed\n",fileNames[i]);
    }
    //start layers here

    //
    for(int i=0; i<MoarLayer_LayersCount;i++) {
        int res = closeLibrary(&(libraries[i]));
        //check for empty lib closing
        if (!res)
            printf("library %s closed\n",libraries[i].Filename);
        else
            printf("close %s failed\n",libraries[i].Filename);
    }
    return 0;
}