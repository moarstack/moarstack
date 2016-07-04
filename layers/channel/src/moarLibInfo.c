//
// Created by svalov on 7/2/16.
//

#include <moarLibInterface.h>

char author[] = "[censored]";
char libName[] = "Channel Layer";
char libDescription[] = "";
uint16_t versionMajor = 0;
uint16_t versionMinor = 1;
uint16_t versionBuild = 1;
uint16_t moarVersion = 1;

int MoarLibraryInfo(MoarLibInfo_T* libInfo){
    libInfo->Author = author;
    libInfo->LibraryName = libName;
    libInfo->ShortDescription = libDescription;
    libInfo->TargetMoarApiVersion = moarVersion;
    libInfo->VersionMajor = versionMajor;
    libInfo->VersionMinor = versionMinor;
    libInfo->VersionBuild = versionBuild;
    libInfo->LayerType = MoarLayer_Channel;
    return 0;
}
