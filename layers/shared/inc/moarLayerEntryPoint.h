//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_MOARLAYERENTRYPOINT_H
#define MOARSTACK_MOARLAYERENTRYPOINT_H

typedef struct{
    int UpSocketHandler;
    int DownSocketHandler;
} MoarLayerStartupParams_T;

typedef void* (*moarLayerEntryPoint_F)(void *);

#ifdef __cplusplus
extern "C" {
#endif

extern void* MoarLayerEntryPoint(void* arg);

#ifdef __cplusplus
};
#endif

#endif //MOARSTACK_MOARLAYERENTRYPOINT_H
