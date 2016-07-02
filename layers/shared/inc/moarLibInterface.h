//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_LIBINTERFACE_H
#define MOARSTACK_LIBINTERFACE_H

#include "stdint.h"

typedef struct{
    char* Author;
    char* LibraryName;
    char* ShortDescription;
    uint16_t VersionMajor;
    uint16_t VersionMinor;
    uint16_t VersionBuild;
    uint16_t MinMoarLevel;
} MoarLibInfo_T;

#ifdef __cplusplus
extern "C" {
#endif

extern int8_t GetMoarLibraryInfo(MoarLibInfo_T* libInfo);

#ifdef __cplusplus
};
#endif

#endif //MOARSTACK_LIBINTERFACE_H
