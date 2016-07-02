//
// Created by svalov on 7/2/16.
//

#include <moarLibInterface.h>
#include "moarLibInterface.h"

char author[] = "[censored]";
char libName[] = "Interface Layer";
char libDescription[] = "";
uint16_t versionMajor = 0;
uint16_t versionMinor = 1;
uint16_t versionBuild = 1;
uint16_t minMoarVersion = 1;

int8_t GetMoarLibraryInfo(MoarLibInfo_T* libInfo){
    libInfo->Author = author;
    libInfo->LibraryName = libName;
    libInfo->ShortDescription = libDescription;
    libInfo->MinMoarLevel = minMoarVersion;
    libInfo->VersionMajor = versionMajor;
    libInfo->VersionMinor = versionMinor;
    libInfo->VersionBuild = versionBuild;
    libInfo->LayerType = MoarLayer_Interface;
    return 0;
}
