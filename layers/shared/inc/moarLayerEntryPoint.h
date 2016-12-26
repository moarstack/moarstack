//
// Created by svalov on 7/2/16.
//

#ifndef MOARSTACK_MOARLAYERENTRYPOINT_H
#define MOARSTACK_MOARLAYERENTRYPOINT_H

#include <hashTable.h>

#define MOAR_LAYER_ENTRY_POINT		MoarLayerEntryPoint
#define MOAR_LAYER_ENTRY_POINT_NAME	"MoarLayerEntryPoint"

#pragma pack(push, 1)

typedef struct{
    int UpSocketHandler;
    int DownSocketHandler;
	hashTable_T* LayerConfig;
} MoarLayerStartupParams_T;

typedef void* (*moarLayerEntryPoint_F)(void *);

#pragma pack(pop)

__BEGIN_DECLS

extern void* MOAR_LAYER_ENTRY_POINT(void* arg);

__END_DECLS

#endif //MOARSTACK_MOARLAYERENTRYPOINT_H
