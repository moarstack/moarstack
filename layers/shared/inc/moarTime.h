//
// Created by svalov on 8/9/16.
//

#ifndef MOARSTACK_MOARTIME_H
#define MOARSTACK_MOARTIME_H

#include <stdint.h>

#define INFINITY_TIME	INT64_MAX

#pragma pack(push, 1)

typedef int64_t moarTime_T;
typedef int64_t moarTimeInterval_T;

#pragma pack(pop)

moarTime_T timeGetCurrent();
moarTimeInterval_T timeGetDifference(moarTime_T first, moarTime_T second);
int timeCompare(moarTime_T first, moarTime_T second);
moarTime_T timeAddInterval(moarTime_T time, moarTimeInterval_T interval);

#endif //MOARSTACK_MOARTIME_H
