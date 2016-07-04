//
// Created by svalov on 7/2/16.
//
#include <libraryLoader.h>
#include <stdio.h>
#include <moarLibInterface.h>

MoarLibrary_T libs[MoarLayer_LayersCount];
char *names[MoarLayer_LayersCount];

int main(int argc, char** argv)
{
    names[MoarLayer_Interface] = "../layers/interface/libmoar_interface.so";
    names[MoarLayer_Channel] = "../layers/channel/libmoar_channel.so";
    names[MoarLayer_Routing] = "../layers/routing/libmoar_routing.so";
    names[MoarLayer_Presentation] = "../layers/presentation/libmoar_presentation.so";
    names[MoarLayer_Service] = "../layers/service/libmoar_service.so";

    for(int i=0; i<MoarLayer_LayersCount;i++) {
        int res = loadLibrary(names[i], &(libs[i]));
        if (!res)
            printf("%s loaded\n", libs[i].Info.LibraryName);
        else
            printf("load failed\n");
    }

    for(int i=0; i<MoarLayer_LayersCount;i++) {
        int res = closeLibrary(&(libs[i]));
        if (!res)
            printf("library closed\n");
        else
            printf("close failed\n");
    }

    return 0;
}