//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <stdio.h>
#include <configFiles.h>
#include <moarLibInterface.h>
#include <unistd.h>
#include <threadManager.h>
#include <moarLibrary.h>

#define LAYERS_COUNT (MoarLayer_LayersCount+1)

int main(int argc, char** argv)
{
    MoarLibrary_T libraries[LAYERS_COUNT];

    char *fileNames[MoarLayer_LayersCount+1];
    fileNames[MoarLayer_Interface] = LIBRARY_PATH_INTERFACE;
    fileNames[MoarLayer_Channel] = LIBRARY_PATH_CHANNEL;
    fileNames[MoarLayer_Routing] = LIBRARY_PATH_ROUTING;
    fileNames[MoarLayer_Presentation] = LIBRARY_PATH_PRESENTATION;
    fileNames[MoarLayer_Service] = LIBRARY_PATH_SERVICE;
    //testing multiple instances
    fileNames[MoarLayer_Service+1] = LIBRARY_PATH_INTERFACE;

    //load
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = loadLibrary(fileNames[i], libraries+i);
        if (LIBRARY_LOAD_OK == res)
            printf("%s by %s loaded\n", libraries[i].Info.LibraryName, libraries[i].Info.Author);
        else
            printf("%s load failed\n",fileNames[i]);
    }
    //prepare sockets
    //start layers here
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = createThread(&(libraries[i]),NULL);
        if(THREAD_CREATE_OK == res)
            printf("thread for %s created\n",libraries[i].Info.LibraryName);
        else
            printf("failed thread creation for %s\n",libraries[i].Info.LibraryName);
    }
    //wait
    pause();
    //stop
    for(int i=0; i<LAYERS_COUNT;i++) {

        int res = exitThread(&(libraries[i]));
        if(THREAD_EXIT_OK == res)
            printf("thread %s exited\n",libraries[i].Info.LibraryName);
        else
            printf("failed thread exit for %s\n",libraries[i].Info.LibraryName);
    }
    //unload
    for(int i=0; i<LAYERS_COUNT;i++) {
        int res = closeLibrary(libraries+i);
        //check for empty lib closing
        if (LIBRARY_CLOSE_OK == res)
            printf("library %s closed\n",libraries[i].Filename);
        else
            printf("close %s failed\n",libraries[i].Filename);
    }
    return 0;
}