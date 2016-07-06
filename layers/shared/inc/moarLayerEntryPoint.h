//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_MOARLAYERENTRYPOINT_H
#define MOARSTACK_MOARLAYERENTRYPOINT_H

#define MOAR_LAYER_ENTRY_POINT MoarLayerEntryPoint
#define MOAR_LAYER_ENTRY_POINT_NAME "MoarLayerEntryPoint"

typedef struct{
    int UpSocketHandler;
    int DownSocketHandler;
} MoarLayerStartupParams_T;

typedef void* (*moarLayerEntryPoint_F)(void *);

#ifdef __cplusplus
extern "C" {
#endif

extern void* MOAR_LAYER_ENTRY_POINT(void* arg);

#ifdef __cplusplus
};
#endif

#endif //MOARSTACK_MOARLAYERENTRYPOINT_H
