//
// Created by svalov on 7/2/16.
//

#include <moarLibInterface.h>
#include <stddef.h>

char author[] = "[censored]";
char libName[] = "Interface Layer";
char libDescription[] = "";
uint16_t versionMajor = 0;
uint16_t versionMinor = 1;
uint16_t versionBuild = 1;
uint16_t targetMoarVersion = 1;

int MoarLibraryInfo(MoarLibInfo_T* libInfo){
    if(NULL == libInfo)
        return MOAR_LIBRARY_INFO_FAILED;
    libInfo->Author = author;
    libInfo->LibraryName = libName;
    libInfo->ShortDescription = libDescription;
    libInfo->TargetMoarApiVersion = targetMoarVersion;
    libInfo->VersionMajor = versionMajor;
    libInfo->VersionMinor = versionMinor;
    libInfo->VersionBuild = versionBuild;
    libInfo->LayerType = MoarLayer_Interface;
    return MOAR_LIBRARY_INFO_OK;
}
